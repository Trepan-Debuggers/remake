/* --watch loop for remake: rebuild on file-system changes.

   Co-inductive by design: the outer event loop only progresses when the
   user (or a build tool) modifies a watched file.  The only inductive
   risk -- a rebuild that triggers itself -- is bounded by a counter
   below (SELF_TRIGGER_HALT) which warns then aborts.

   For non-makefile sources we do an in-process rebuild
   (update_goal_chain) for speed.  When a *makefile* changes we
   re-exec ourselves with the original argv, which is the only
   behavior consistent with normal make semantics (re-parse from
   scratch).  This mirrors what main.c does at the bootstrap re_exec:
   label; we deliberately use a simpler execvp here to keep the patch
   minimal -- see main.c around the re_exec label for the fuller
   ceremony (MAKE_RESTARTS bookkeeping, stdin temp file fixup) which
   we do not need in the watch case.  */

#include "makeint.h"

#ifdef HAVE_SYS_INOTIFY_H

#include "filedef.h"
#include "dep.h"
#include "debug.h"
#include "os.h"

#include <sys/inotify.h>
#include <sys/stat.h>
#include <unistd.h>
#include <poll.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <libgen.h>
#include <time.h>

/* Tunables.  Kept conservative; expose later as env vars if needed.  */
#define DEBOUNCE_MS            50
#define DEBOUNCE_MAX_EVENTS    10000
#define EVENT_BUF_BYTES        (64 * 1024)
#define SELF_TRIGGER_WARN      3
#define SELF_TRIGGER_HALT      10
#define SELF_TRIGGER_WINDOW_NS 200000000L  /* 200ms */

/* A directory we watch.  We watch the *directory* (not the file) so
   atomic-replace saves (vim, most editors) don't detach the watch.
   Each entry tracks the basenames in that directory we care about,
   so changes to unrelated siblings can be filtered out.  */
struct watched_dir
{
  int wd;                       /* inotify watch descriptor */
  char *path;                   /* directory path (owned) */
  char **basenames;             /* names of files we care about */
  int n_basenames;
  int cap_basenames;
  unsigned int is_makefile_dir : 1;
};

static int notify_fd = -1;
static struct watched_dir *src_dirs = NULL;
static int n_src_dirs = 0, cap_src_dirs = 0;
static struct watched_dir *mk_dirs = NULL;
static int n_mk_dirs = 0, cap_mk_dirs = 0;

/* For self-trigger detection.  */
static int self_trigger_count = 0;
static struct timespec last_build_finished;

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
      for (size_t k = 0; k < old_cap; k++)
        if (old[k]) visited_add (v, old[k]);
      free (old);
    }
  return 1;
}

/* Add basename to dir entry if not present.  */
static void
dir_add_basename (struct watched_dir *d, const char *base)
{
  for (int i = 0; i < d->n_basenames; i++)
    if (strcmp (d->basenames[i], base) == 0) return;
  if (d->n_basenames == d->cap_basenames)
    {
      d->cap_basenames = d->cap_basenames ? d->cap_basenames * 2 : 4;
      d->basenames = xrealloc (d->basenames,
                               d->cap_basenames * sizeof (char *));
    }
  d->basenames[d->n_basenames++] = xstrdup (base);
}

static struct watched_dir *
find_or_add_dir (struct watched_dir **arr, int *n, int *cap,
                 const char *path, int is_makefile_dir)
{
  /* CLOSE_WRITE coalesces in-progress writes; MOVED_TO catches editor
     atomic-replace; CREATE/DELETE catch newly-added/removed prereqs.
     IN_MODIFY and IN_ATTRIB are intentionally omitted -- they fire
     mid-write and on metadata-only changes, producing spurious
     wakeups.  */
  const uint32_t mask = IN_CLOSE_WRITE | IN_MOVED_TO | IN_CREATE | IN_DELETE;
  struct watched_dir *d;
  int i;

  for (i = 0; i < *n; i++)
    if (strcmp ((*arr)[i].path, path) == 0)
      return &(*arr)[i];

  if (*n == *cap)
    {
      *cap = *cap ? *cap * 2 : 8;
      *arr = xrealloc (*arr, *cap * sizeof (struct watched_dir));
    }
  d = &(*arr)[*n];
  memset (d, 0, sizeof *d);
  d->path = xstrdup (path);
  d->is_makefile_dir = is_makefile_dir ? 1 : 0;

  d->wd = inotify_add_watch (notify_fd, path, mask);
  if (d->wd < 0)
    {
      OSS (error, NILF, _("--watch: cannot watch directory '%s': %s"),
           path, strerror (errno));
      free (d->path);
      d->path = NULL;
      return NULL;
    }
  (*n)++;
  return d;
}

/* Split f->name into (dirpath, basename) using a writable copy.
   Returns 0 on success.  Caller frees *dir_out.  */
static int
split_path (const char *name, char **dir_out, char **base_out,
            char **scratch_out)
{
  char *scratch_dir = xstrdup (name);
  char *scratch_base = xstrdup (name);
  *dir_out = xstrdup (dirname (scratch_dir));
  *base_out = xstrdup (basename (scratch_base));
  free (scratch_dir);
  free (scratch_base);
  *scratch_out = NULL;
  return 0;
}

/* Add the file f to the source watch set if it is a leaf source --
   not phony, has a real name on disk, and is not itself a build
   product (no recipe attached).  Watching build outputs would make
   each successful build trigger the next one.  */
static void
watch_one_source (struct file *f)
{
  char *dir, *base, *scratch;
  struct watched_dir *d;

  if (!f || !f->name) return;
  if (f->phony) return;
  if (f->cmds) return;
  if (f->name[0] == '\0') return;

  if (split_path (f->name, &dir, &base, &scratch) != 0) return;

  d = find_or_add_dir (&src_dirs, &n_src_dirs, &cap_src_dirs, dir, 0);
  if (d) dir_add_basename (d, base);

  free (dir);
  free (base);
}

/* DFS the goal subgraph collecting watch entries.  */
static void
collect_watched (struct file *f, struct visited_set *seen)
{
  if (!f) return;
  while (f->renamed) f = f->renamed;
  if (!visited_add (seen, f)) return;

  watch_one_source (f);

  for (struct dep *d = f->deps; d; d = d->next)
    if (d->file) collect_watched (d->file, seen);
  for (struct dep *d = f->also_make; d; d = d->next)
    if (d->file) collect_watched (d->file, seen);
}

static void
build_source_watch_set (struct goaldep *goals)
{
  struct visited_set seen;
  visited_init (&seen, 64);
  for (struct goaldep *g = goals; g; g = g->next)
    if (g->file) collect_watched (g->file, &seen);
  visited_free (&seen);
}

/* read_makefiles is the chain remake parsed; watch each entry's
   directory.  This set is computed once per process: any change here
   triggers re-exec, and the new process recomputes from scratch.  */
static void
build_makefile_watch_set (void)
{
  struct goaldep *g;
  char *dir, *base, *scratch;
  struct watched_dir *d;

  for (g = read_makefiles; g; g = g->next)
    {
      if (!g->file || !g->file->name) continue;
      if (split_path (g->file->name, &dir, &base, &scratch) != 0) continue;
      d = find_or_add_dir (&mk_dirs, &n_mk_dirs, &cap_mk_dirs, dir, 1);
      if (d) dir_add_basename (d, base);
      free (dir); free (base);
    }
}

/* Reset per-build state on the goal subgraph so update_goal_chain
   re-evaluates everything.  Walking via DFS keeps us scoped: we never
   reset files outside the goals' transitive prereqs.  */
static void
reset_one (struct file *f, struct visited_set *seen)
{
  if (!f) return;
  while (f->renamed) f = f->renamed;
  if (!visited_add (seen, f)) return;

  f->last_mtime = UNKNOWN_MTIME;
  f->mtime_before_update = UNKNOWN_MTIME;
  f->updated = 0;
  f->considered = 0;
  f->command_state = cs_not_started;
  f->update_status = us_none;

  for (struct dep *d = f->deps; d; d = d->next)
    {
      d->changed = 0;
      if (d->file) reset_one (d->file, seen);
    }
  for (struct dep *d = f->also_make; d; d = d->next)
    if (d->file) reset_one (d->file, seen);
}

static void
reset_goal_subgraph (struct goaldep *goals)
{
  struct visited_set seen;
  visited_init (&seen, 64);
  for (struct goaldep *g = goals; g; g = g->next)
    if (g->file) reset_one (g->file, &seen);
  visited_free (&seen);
}

/* Returns 1 if (wd, basename) hits the source set, 2 if makefile set,
   0 if neither.  Makefile takes precedence on a tie -- a re-exec is a
   superset of a rebuild.  */
static int
classify (int wd, const char *base)
{
  for (int i = 0; i < n_mk_dirs; i++)
    if (mk_dirs[i].wd == wd)
      for (int j = 0; j < mk_dirs[i].n_basenames; j++)
        if (strcmp (mk_dirs[i].basenames[j], base) == 0)
          return 2;
  for (int i = 0; i < n_src_dirs; i++)
    if (src_dirs[i].wd == wd)
      for (int j = 0; j < src_dirs[i].n_basenames; j++)
        if (strcmp (src_dirs[i].basenames[j], base) == 0)
          return 1;
  return 0;
}

/* Drain inotify events with a debounce.  Returns 1 if a source change
   (or queue overflow) was seen, 2 if a makefile change was seen
   (highest wins), 0 if no watched basenames matched (caller should
   keep waiting), -1 on fatal read error.  Bounded by both DEBOUNCE_MS
   and DEBOUNCE_MAX_EVENTS.  */
static int
wait_and_drain (void)
{
  char buf[EVENT_BUF_BYTES] __attribute__((aligned(8)));
  struct pollfd pfd;
  int kind = 0;
  int events_seen = 0;
  int r;

  pfd.fd = notify_fd;
  pfd.events = POLLIN;
  pfd.revents = 0;

  /* Block until first event arrives.  */
  for (;;)
    {
      r = poll (&pfd, 1, -1);
      if (r < 0)
        {
          if (errno == EINTR) return -1;     /* signal: caller decides */
          OS (error, NILF, _("--watch: poll: %s"), strerror (errno));
          return -1;
        }
      if (pfd.revents & POLLIN) break;
    }

  /* Then drain with short timeout to coalesce bursts.  */
  for (;;)
    {
      ssize_t len = read (notify_fd, buf, sizeof buf);
      char *p;
      if (len < 0)
        {
          if (errno == EAGAIN
#if EAGAIN != EWOULDBLOCK
              || errno == EWOULDBLOCK
#endif
              ) break;
          if (errno == EINTR) continue;
          OS (error, NILF, _("--watch: read: %s"), strerror (errno));
          return -1;
        }
      for (p = buf; p < buf + len; )
        {
          struct inotify_event *e = (struct inotify_event *) p;
          if (e->mask & IN_Q_OVERFLOW)
            return 1;                        /* lost events: force source rebuild */
          if (e->len > 0)
            {
              int k = classify (e->wd, e->name);
              if (k > kind) kind = k;
            }
          p += sizeof (struct inotify_event) + e->len;
          if (++events_seen >= DEBOUNCE_MAX_EVENTS)
            return kind;                     /* hard cap */
        }
      if (kind == 2) return 2;               /* makefile wins, no need to drain more */

      pfd.revents = 0;
      r = poll (&pfd, 1, DEBOUNCE_MS);
      if (r <= 0) break;
    }

  /* If we got events but none matched watched basenames (e.g. build
     outputs in the same directory), report kind=0.  The caller's outer
     loop will go back to poll() and block until the next event, so
     this does not spin.  */
  return kind;
}

static long
ts_diff_ns (const struct timespec *a, const struct timespec *b)
{
  return (a->tv_sec - b->tv_sec) * 1000000000L + (a->tv_nsec - b->tv_nsec);
}

void
watch_loop (struct goaldep *goals, int argc, char **argv)
{
  /* Save argv for re-exec.  argv may live on the stack of main(); we
     copy pointers, but the strings themselves are stable.  */
  char **saved_argv = xmalloc ((argc + 1) * sizeof (char *));
  enum update_status st;
  int i;

  for (i = 0; i < argc; i++) saved_argv[i] = argv[i];
  saved_argv[argc] = NULL;

  notify_fd = inotify_init1 (IN_CLOEXEC | IN_NONBLOCK);
  if (notify_fd < 0)
    {
      OS (fatal, NILF, _("--watch: inotify_init: %s"), strerror (errno));
      return;
    }

  /* Initial build, in-process, exactly as the non-watch path would.  */
  st = update_goal_chain (goals);
  if (st == us_failed)
    O (error, NILF,
       _("--watch: initial build failed; will retry on next change."));

  build_source_watch_set (goals);
  build_makefile_watch_set ();
  clock_gettime (CLOCK_MONOTONIC, &last_build_finished);

  /* Note: -p (print_data_base_flag) is honored on exit via die(),
     same as the non-watch path.  */

  O (message, 0, _("--watch: waiting for changes (Ctrl-C to stop)..."));

  for (;;)
    {
      int kind;
      struct timespec now;

      kind = wait_and_drain ();
      if (kind < 0)
        {
          /* Signal: let normal signal handling shut us down cleanly.  */
          die (MAKE_FAILURE);
        }
      if (kind == 0)
        continue;                            /* events were not for us */

      clock_gettime (CLOCK_MONOTONIC, &now);

      if (kind == 2)
        {
          /* Makefile change: re-exec for full re-parse.  Simpler than
             the bootstrap re_exec path in main.c -- we have no stdin
             temp file to preserve and no in-flight makefile rebuild.  */
          O (message, 0,
             _("--watch: makefile changed, re-executing remake."));
          fflush (stdout);
          fflush (stderr);
          close (notify_fd);
          execvp (saved_argv[0], saved_argv);
          OS (fatal, NILF, _("--watch: execvp failed: %s"), strerror (errno));
        }

      /* Inductive-unboundedness guard: if the rebuild that just
         finished is what triggered this wake (event came within
         SELF_TRIGGER_WINDOW_NS of the build completing), assume the
         rebuild touched its own watched outputs.  */
      if (ts_diff_ns (&now, &last_build_finished) < SELF_TRIGGER_WINDOW_NS)
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
      build_source_watch_set (goals);  /* pick up new generated prereqs */
      clock_gettime (CLOCK_MONOTONIC, &last_build_finished);
    }
}

#endif /* HAVE_SYS_INOTIFY_H */
