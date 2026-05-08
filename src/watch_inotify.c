/* inotify-based --watch backend for remake.
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

#include "makeint.h"

#ifdef HAVE_SYS_INOTIFY_H

#include "debug.h"
#include "os.h"
#include "watchbackend.h"

#include <sys/inotify.h>
#include <unistd.h>
#include <poll.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>

/* Tunables.  Kept conservative; expose later as env vars if needed.  */
#define DEBOUNCE_MS            50
#define DEBOUNCE_MAX_EVENTS    10000
#define EVENT_BUF_BYTES        (64 * 1024)

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
};

struct watch_backend
{
  int notify_fd;
  struct watched_dir *src_dirs;
  int n_src_dirs, cap_src_dirs;
  struct watched_dir *mk_dirs;
  int n_mk_dirs, cap_mk_dirs;
};

static void
dir_add_basename (struct watched_dir *d, const char *base)
{
  int i;
  for (i = 0; i < d->n_basenames; i++)
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
find_or_add_dir (struct watch_backend *wb, struct watched_dir **arr,
                 int *n, int *cap, const char *path)
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
  memset (d, 0, sizeof (*d));
  d->path = xstrdup (path);
  d->wd = inotify_add_watch (wb->notify_fd, path, mask);
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

struct watch_backend *
wb_init (void)
{
  struct watch_backend *wb = xcalloc (sizeof (*wb));
  wb->notify_fd = inotify_init1 (IN_CLOEXEC | IN_NONBLOCK);
  if (wb->notify_fd < 0)
    {
      OS (fatal, NILF, _("--watch: inotify_init: %s"), strerror (errno));
      /* unreachable */
      free (wb);
      return NULL;
    }
  return wb;
}

int
wb_add (struct watch_backend *wb, const char *dir, const char *base,
        int is_makefile)
{
  struct watched_dir **arr  = is_makefile ? &wb->mk_dirs    : &wb->src_dirs;
  int *n                    = is_makefile ? &wb->n_mk_dirs  : &wb->n_src_dirs;
  int *cap                  = is_makefile ? &wb->cap_mk_dirs: &wb->cap_src_dirs;
  struct watched_dir *d = find_or_add_dir (wb, arr, n, cap, dir);
  if (!d) return -1;
  dir_add_basename (d, base);
  return 0;
}

/* Returns 1 if (wd, basename) hits the source set, 2 if makefile set,
   0 if neither.  Makefile takes precedence on a tie -- a re-exec is a
   superset of a rebuild.  */
static int
classify (struct watch_backend *wb, int wd, const char *base)
{
  int i, j;
  for (i = 0; i < wb->n_mk_dirs; i++)
    if (wb->mk_dirs[i].wd == wd)
      for (j = 0; j < wb->mk_dirs[i].n_basenames; j++)
        if (strcmp (wb->mk_dirs[i].basenames[j], base) == 0)
          return 2;
  for (i = 0; i < wb->n_src_dirs; i++)
    if (wb->src_dirs[i].wd == wd)
      for (j = 0; j < wb->src_dirs[i].n_basenames; j++)
        if (strcmp (wb->src_dirs[i].basenames[j], base) == 0)
          return 1;
  return 0;
}

int
wb_wait (struct watch_backend *wb)
{
  char buf[EVENT_BUF_BYTES] __attribute__((aligned(8)));
  struct pollfd pfd;
  int kind = 0;
  int events_seen = 0;
  int r;

  pfd.fd = wb->notify_fd;
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
      ssize_t len = read (wb->notify_fd, buf, sizeof (buf));
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
              int k = classify (wb, e->wd, e->name);
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

int
wb_min_latency_ms (const struct watch_backend *wb)
{
  (void) wb;
  /* Inotify reports events in milliseconds; the debounce is the only
     intentional delay we add.  */
  return DEBOUNCE_MS;
}

void
wb_close (struct watch_backend *wb)
{
  int i, j;
  if (!wb) return;
  if (wb->notify_fd >= 0) close (wb->notify_fd);
  for (i = 0; i < wb->n_src_dirs; i++)
    {
      free (wb->src_dirs[i].path);
      for (j = 0; j < wb->src_dirs[i].n_basenames; j++)
        free (wb->src_dirs[i].basenames[j]);
      free (wb->src_dirs[i].basenames);
    }
  free (wb->src_dirs);
  for (i = 0; i < wb->n_mk_dirs; i++)
    {
      free (wb->mk_dirs[i].path);
      for (j = 0; j < wb->mk_dirs[i].n_basenames; j++)
        free (wb->mk_dirs[i].basenames[j]);
      free (wb->mk_dirs[i].basenames);
    }
  free (wb->mk_dirs);
  free (wb);
}

#endif /* HAVE_SYS_INOTIFY_H */
