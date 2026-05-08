/* stat()-based polling --watch backend for remake.
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

/* This backend is selected when no native fs-event API is available.
   It is the universal fallback: every platform that has stat(2) can
   use --watch through this backend.  Linux / Cygwin builds prefer
   watch_inotify.c instead; pass --without-inotify to configure to
   route through here for testing.

   We compare the previous and current stat() of every watched path on
   each tick.  A path is considered "changed" if mtime, size, or inode
   differs (the last covers atomic-replace editor saves).  Granularity
   is set by env MAKE_WATCH_POLL_INTERVAL (milliseconds; default
   1000).  */

#include "makeint.h"

#ifndef HAVE_SYS_INOTIFY_H

#include "debug.h"
#include "os.h"
#include "watchbackend.h"

#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>

#ifdef HAVE_NANOSLEEP
# include <time.h>
#else
/* Fallback: select(2) with a finite timeout is the most portable
   "sleep with sub-second precision" call.  POSIX guarantees it on
   every platform GNU Make builds on.  */
# include <sys/select.h>
#endif

#define DEFAULT_POLL_MS  1000
#define MIN_POLL_MS      50
#define MAX_POLL_MS      60000

struct watched_entry
{
  char *path;                   /* full path, owned */
  struct stat last;             /* last seen stat; valid iff have_stat */
  int have_stat;                /* 0 if path did not exist last tick */
  int is_makefile;
};

struct watch_backend
{
  struct watched_entry *entries;
  int n, cap;
  int poll_ms;
};

static int
parse_poll_ms (void)
{
  const char *s = getenv ("MAKE_WATCH_POLL_INTERVAL");
  char *end;
  long v;

  if (!s || !*s) return DEFAULT_POLL_MS;
  v = strtol (s, &end, 10);
  if (*end != '\0' || v < MIN_POLL_MS || v > MAX_POLL_MS)
    {
      OSN (error, NILF,
           _("--watch: ignoring bad MAKE_WATCH_POLL_INTERVAL='%s' (using %d ms)"),
           s, DEFAULT_POLL_MS);
      return DEFAULT_POLL_MS;
    }
  return (int) v;
}

struct watch_backend *
wb_init (void)
{
  struct watch_backend *wb = xcalloc (sizeof (*wb));
  wb->poll_ms = parse_poll_ms ();
  return wb;
}

int
wb_add (struct watch_backend *wb, const char *dir, const char *base,
        int is_makefile)
{
  size_t dlen = strlen (dir);
  size_t blen = strlen (base);
  /* +2: one for the separator, one for NUL.  */
  char *full = xmalloc (dlen + blen + 2);
  int sep = (dlen > 0 && dir[dlen - 1] != '/');
  struct watched_entry *e;
  int i;

  if (dlen == 1 && dir[0] == '.')
    {
      /* Strip leading "./" so two callers passing "./foo" and "foo"
         end up identical and we don't double-watch.  */
      memcpy (full, base, blen + 1);
    }
  else
    {
      memcpy (full, dir, dlen);
      if (sep) full[dlen++] = '/';
      memcpy (full + dlen, base, blen + 1);
    }

  /* Dedup against existing entries.  Linear scan is fine: the goal
     subgraph is rebuilt periodically and is typically small.  */
  for (i = 0; i < wb->n; i++)
    if (wb->entries[i].is_makefile == is_makefile
        && strcmp (wb->entries[i].path, full) == 0)
      {
        free (full);
        return 0;
      }

  if (wb->n == wb->cap)
    {
      wb->cap = wb->cap ? wb->cap * 2 : 64;
      wb->entries = xrealloc (wb->entries,
                              wb->cap * sizeof (struct watched_entry));
    }
  e = &wb->entries[wb->n++];
  e->path = full;
  e->is_makefile = is_makefile;
  e->have_stat = 0;
  if (stat (full, &e->last) == 0)
    e->have_stat = 1;
  return 0;
}

/* Returns 1 if e changed since last call (and updates e->last), 0 if
   not.  A vanished path counts as change.  A reappeared path counts as
   change.  */
static int
entry_changed (struct watched_entry *e)
{
  struct stat st;
  int rc = stat (e->path, &st);
  if (rc != 0)
    {
      if (e->have_stat)
        {
          e->have_stat = 0;
          return 1;                          /* existed -> gone */
        }
      return 0;                              /* still missing */
    }
  if (!e->have_stat)
    {
      e->have_stat = 1;
      e->last = st;
      return 1;                              /* gone -> exists */
    }
  if (st.st_mtime != e->last.st_mtime
      || st.st_size  != e->last.st_size
      || st.st_ino   != e->last.st_ino)
    {
      e->last = st;
      return 1;
    }
  return 0;
}

/* Sleep up to MS milliseconds.  Returns 0 if the sleep finished, or
   -1 if interrupted by a signal -- in which case the caller should
   propagate the interruption upward (so Ctrl-C does not feel
   sluggish under the poll backend's tick).  */
static int
sleep_ms (int ms)
{
#ifdef HAVE_NANOSLEEP
  struct timespec req;
  req.tv_sec  = ms / 1000;
  req.tv_nsec = (ms % 1000) * 1000000L;
  if (nanosleep (&req, NULL) < 0)
    return -1;                          /* signal or other error */
  return 0;
#else
  struct timeval tv;
  tv.tv_sec  = ms / 1000;
  tv.tv_usec = (ms % 1000) * 1000;
  if (select (0, NULL, NULL, NULL, &tv) < 0)
    return -1;
  return 0;
#endif
}

int
wb_wait (struct watch_backend *wb)
{
  int kind = 0;
  int i;

  /* Sleep, then sweep.  We sleep first so the caller's just-finished
     build has a beat to settle.  Propagate EINTR upward as -1 so
     Ctrl-C is responsive even mid-tick.  */
  if (sleep_ms (wb->poll_ms) < 0)
    return -1;

  for (i = 0; i < wb->n; i++)
    {
      if (entry_changed (&wb->entries[i]))
        {
          int k = wb->entries[i].is_makefile ? 2 : 1;
          if (k > kind) kind = k;
          if (kind == 2) break;              /* makefile wins */
        }
    }
  return kind;                               /* 0 means no change this tick */
}

int
wb_min_latency_ms (const struct watch_backend *wb)
{
  return wb->poll_ms;
}

void
wb_close (struct watch_backend *wb)
{
  int i;
  if (!wb) return;
  for (i = 0; i < wb->n; i++)
    free (wb->entries[i].path);
  free (wb->entries);
  free (wb);
}

#endif /* !HAVE_SYS_INOTIFY_H */
