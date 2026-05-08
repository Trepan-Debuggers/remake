/* --watch event loop for remake: rebuild on file-system changes.
Copyright (C) 2026 Free Software Foundation, Inc.
This file is part of GNU Make / remake.

GNU Make is free software; you can redistribute it and/or modify it under the
terms of the GNU General Public License as published by the Free Software
Foundation; either version 3 of the License, or (at your option) any later
version.

GNU Make is distributed in the hope that it will be useful, but WITHOUT ANY
WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR
A PARTICULAR PURPOSE.  See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License along with
this program.  If not, see <https://www.gnu.org/licenses/>.  */

/* The outer event loop only progresses when the user (or a build tool)
   modifies a watched file.  The one inductive risk -- a rebuild that
   triggers itself -- is bounded by SELF_TRIGGER_HALT below, which warns
   then aborts.

   For non-makefile sources we do an in-process rebuild
   (update_goal_chain) for speed.  When a makefile changes we re-exec
   ourselves with the original argv, which is the only behavior
   consistent with normal make semantics (re-parse from scratch).  This
   is a simpler execvp than main.c's re_exec path -- we have no stdin
   temp file to preserve and no in-flight makefile rebuild.

   File-system-event detection is done by a backend selected at compile
   time -- watch_inotify.c on Linux/Cygwin, watch_poll.c elsewhere --
   behind the small interface in watchbackend.h.  */

#include "makeint.h"

#include "filedef.h"
#include "dep.h"
#include "debug.h"
#include "os.h"
#include "watchbackend.h"

#include <sys/stat.h>
#include <sys/time.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include <libgen.h>
#include <time.h>

#define SELF_TRIGGER_WARN          3
#define SELF_TRIGGER_HALT          10
/* Floor for the self-trigger window.  The actual window used is
   max(SELF_TRIGGER_WINDOW_MS_FLOOR, 2 * backend->min_latency_ms) so
   the polling backend's tick (typically 1 s) does not silently
   bypass the guard.  */
#define SELF_TRIGGER_WINDOW_MS_FLOOR 200

/* mainline globals we need: directory the user was in before -C, so
   we can restore it across the makefile-edit re-exec.  */
extern char *directory_before_chdir;

/* Monotonic time in milliseconds since some unspecified epoch, used
   only for differences.  Falls back to gettimeofday() (POSIX.1-2001)
   when clock_gettime() is unavailable; that fallback is wall-clock
   and so a step backwards (NTP, manual date change) will produce a
   negative diff -- callers must treat negative diffs as "long ago".  */
static intmax_t
monotonic_ms (void)
{
#if defined(HAVE_CLOCK_GETTIME) && defined(CLOCK_MONOTONIC)
  struct timespec ts;
  if (clock_gettime (CLOCK_MONOTONIC, &ts) == 0)
    return (intmax_t) ts.tv_sec * 1000 + ts.tv_nsec / 1000000;
#endif
#ifdef HAVE_GETTIMEOFDAY
  {
    struct timeval tv;
    if (gettimeofday (&tv, NULL) == 0)
      return (intmax_t) tv.tv_sec * 1000 + tv.tv_usec / 1000;
  }
#endif
  /* Last resort: 1-second resolution from time().  Self-trigger
     guard becomes coarse but the loop still works.  */
  return (intmax_t) time (NULL) * 1000;
}

/* For self-trigger detection.  */
static int self_trigger_count = 0;
static intmax_t last_build_finished_ms = 0;

/* DFS visited set: pointer-identity, linear-probed open addressing.
   Tiny hash table, sized to power of two >= 2 * |goal subgraph|.  */
struct visited_set
{
  struct file **slots;
  size_t cap;
  size_t n;
};

static void
visited_init (struct visited_set *v, size_t hint)
{
  size_t cap = 16;
  while (cap < hint * 2) cap <<= 1;
  v->slots = xcalloc (cap * sizeof (struct file *));
  v->cap = cap;
  v->n = 0;
}

static void
visited_free (struct visited_set *v)
{
  free (v->slots);
  v->slots = NULL;
  v->cap = v->n = 0;
}

static int
visited_add (struct visited_set *v, struct file *f)
{
  size_t mask = v->cap - 1;
  size_t i = ((uintptr_t) f >> 4) & mask;
  size_t k;
  while (v->slots[i])
    {
      if (v->slots[i] == f) return 0;       /* already present */
      i = (i + 1) & mask;
    }
  v->slots[i] = f;
  v->n++;
  /* Grow if load factor > 0.5.  */
  if (v->n * 2 > v->cap)
    {
      struct file **old = v->slots;
      size_t old_cap = v->cap;
      v->cap *= 2;
      v->slots = xcalloc (v->cap * sizeof (struct file *));
      v->n = 0;
      for (k = 0; k < old_cap; k++)
        if (old[k]) visited_add (v, old[k]);
      free (old);
    }
  return 1;
}

/* Split NAME into (dirpath, basename), writing freshly-allocated copies
   to *DIR_OUT and *BASE_OUT.  Caller frees both.  POSIX dirname()/
   basename() may modify their input, so we use scratch copies.  */
static void
split_path (const char *name, char **dir_out, char **base_out)
{
  char *scratch_dir = xstrdup (name);
  char *scratch_base = xstrdup (name);
  *dir_out = xstrdup (dirname (scratch_dir));
  *base_out = xstrdup (basename (scratch_base));
  free (scratch_dir);
  free (scratch_base);
}

/* Add the file f to the source watch set if it is a leaf source --
   not phony, has a real name on disk, and is not itself a build
   product (no recipe attached).  Watching build outputs would make
   each successful build trigger the next one.  */
static void
watch_one_source (struct watch_backend *wb, struct file *f)
{
  char *dir, *base;
  if (!f || !f->name) return;
  if (f->phony) return;
  if (f->cmds) return;
  if (f->name[0] == '\0') return;

  split_path (f->name, &dir, &base);
  wb_add (wb, dir, base, 0 /* source */);
  free (dir);
  free (base);
}

/* DFS the goal subgraph collecting watch entries.  */
static void
collect_watched (struct watch_backend *wb, struct file *f,
                 struct visited_set *seen)
{
  struct dep *d;
  if (!f) return;
  while (f->renamed) f = f->renamed;
  if (!visited_add (seen, f)) return;

  watch_one_source (wb, f);

  for (d = f->deps; d; d = d->next)
    if (d->file) collect_watched (wb, d->file, seen);
  for (d = f->also_make; d; d = d->next)
    if (d->file) collect_watched (wb, d->file, seen);
}

static void
build_source_watch_set (struct watch_backend *wb, struct goaldep *goals)
{
  struct visited_set seen;
  struct goaldep *g;
  visited_init (&seen, 64);
  for (g = goals; g; g = g->next)
    if (g->file) collect_watched (wb, g->file, &seen);
  visited_free (&seen);
}

/* read_files is the chain main() parsed; watch each entry's
   directory.  This set is computed once per process: any change here
   triggers re-exec, and the new process recomputes from scratch.  */
static void
build_makefile_watch_set (struct watch_backend *wb,
                          struct goaldep *read_files)
{
  struct goaldep *g;
  for (g = read_files; g; g = g->next)
    {
      char *dir, *base;
      if (!g->file || !g->file->name) continue;
      split_path (g->file->name, &dir, &base);
      wb_add (wb, dir, base, 1 /* makefile */);
      free (dir);
      free (base);
    }
}

/* Reset per-build state on the goal subgraph so update_goal_chain
   re-evaluates everything.  Walking via DFS keeps us scoped: we never
   reset files outside the goals' transitive prereqs.  */
static void
reset_one (struct file *f, struct visited_set *seen)
{
  struct dep *d;
  if (!f) return;
  while (f->renamed) f = f->renamed;
  if (!visited_add (seen, f)) return;

  /* update_goal_chain bumps a global 'considered' counter on entry and
     compares f->considered to it, so leaving f->considered alone is
     enough to force re-evaluation.  */
  f->last_mtime = UNKNOWN_MTIME;
  f->mtime_before_update = UNKNOWN_MTIME;
  f->updated = 0;
  f->command_state = cs_not_started;
  f->update_status = us_none;

  for (d = f->deps; d; d = d->next)
    {
      d->changed = 0;
      if (d->file) reset_one (d->file, seen);
    }
  for (d = f->also_make; d; d = d->next)
    if (d->file) reset_one (d->file, seen);
}

static void
reset_goal_subgraph (struct goaldep *goals)
{
  struct visited_set seen;
  struct goaldep *g;
  visited_init (&seen, 64);
  for (g = goals; g; g = g->next)
    if (g->file) reset_one (g->file, &seen);
  visited_free (&seen);
}

void
watch_loop (struct goaldep *goals, struct goaldep *read_files,
            int argc, char **argv)
{
  char **saved_argv;
  struct watch_backend *wb;
  enum update_status st;
  intmax_t self_trigger_window_ms;
  int min_lat;
  int i;

  /* Save argv for re-exec.  argv may live on the stack of main(); we
     copy pointers, but the strings themselves are stable.  */
  saved_argv = xmalloc ((argc + 1) * sizeof (char *));
  for (i = 0; i < argc; i++) saved_argv[i] = argv[i];
  saved_argv[argc] = NULL;

  wb = wb_init ();
  if (!wb)
    {
      free (saved_argv);
      return;                                /* wb_init already fataled */
    }

  /* Self-trigger window must be at least one backend tick (otherwise
     under polling the just-finished build's mtime updates land
     outside the window and the guard never fires).  */
  min_lat = wb_min_latency_ms (wb);
  self_trigger_window_ms = SELF_TRIGGER_WINDOW_MS_FLOOR;
  if ((intmax_t) min_lat * 2 > self_trigger_window_ms)
    self_trigger_window_ms = (intmax_t) min_lat * 2;

  /* Initial build, in-process, exactly as the non-watch path would.
     -p (print_data_base_flag) and other end-of-run actions are
     honored by die() on the rebuild path; they are *not* honored on
     the makefile-edit re-exec path (execvp replaces the process
     image), which mirrors the non-watch behavior of restart.  */
  st = update_goal_chain (goals);
  if (st == us_failed)
    O (error, NILF,
       _("--watch: initial build failed; will retry on next change."));

  build_source_watch_set (wb, goals);
  build_makefile_watch_set (wb, read_files);
  last_build_finished_ms = monotonic_ms ();

  O (message, 0, _("--watch: waiting for changes (Ctrl-C to stop)..."));

  for (;;)
    {
      int kind;
      intmax_t now_ms, since_ms;

      kind = wb_wait (wb);
      if (kind < 0)
        {
          /* Signal: let normal signal handling shut us down cleanly.  */
          die (MAKE_FAILURE);
        }
      if (kind == 0)
        continue;                            /* events were not for us */

      now_ms = monotonic_ms ();

      if (kind == 2)
        {
          /* Makefile change: re-exec for full re-parse.  Restore the
             working directory the user invoked us from so that any
             relative paths in argv (e.g. -C subdir, -f path) and
             argv[0] itself resolve identically in the new process.  */
          O (message, 0,
             _("--watch: makefile changed, re-executing remake."));
          fflush (stdout);
          fflush (stderr);
          wb_close (wb);
          if (directory_before_chdir != 0
              && chdir (directory_before_chdir) < 0)
            OS (fatal, NILF, _("--watch: chdir before re-exec: %s"),
                strerror (errno));
          execvp (saved_argv[0], saved_argv);
          OS (fatal, NILF, _("--watch: execvp failed: %s"), strerror (errno));
        }

      /* Inductive-unboundedness guard: if this event arrived within
         the self-trigger window of the last build finishing, assume
         the rebuild touched its own watched outputs.  Negative diffs
         (clock skew under the gettimeofday fallback) are treated as
         "long ago" so we do not falsely accuse.  */
      since_ms = now_ms - last_build_finished_ms;
      if (since_ms >= 0 && since_ms < self_trigger_window_ms)
        {
          self_trigger_count++;
          if (self_trigger_count == SELF_TRIGGER_WARN)
            O (error, NILF,
               _("--watch: warning: rebuild appears to trigger itself; "
                 "check for recipes that touch their own watched outputs."));
          if (self_trigger_count >= SELF_TRIGGER_HALT)
            {
              O (error, NILF,
                 _("--watch: rebuild self-trigger limit reached, exiting."));
              die (MAKE_FAILURE);
            }
        }
      else
        self_trigger_count = 0;

      O (message, 0, _("--watch: change detected, rebuilding."));
      reset_goal_subgraph (goals);
      st = update_goal_chain (goals);
      if (st == us_failed)
        O (error, NILF, _("--watch: build failed; will retry on next change."));
      build_source_watch_set (wb, goals);  /* pick up new generated prereqs */
      last_build_finished_ms = monotonic_ms ();
    }
}
