/* File-system watch backend interface for remake's --watch.
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

/* The frontend in watch.c walks the goal subgraph and feeds each watched
   (directory, basename) pair to a backend through this interface.

   Exactly one backend is selected at compile time:

     HAVE_SYS_INOTIFY_H  -> watch_inotify.c   (Linux, Cygwin; can be
                            forced off via configure --without-inotify)
     else                -> watch_poll.c      (universal fallback,
                            stat()-based)

   Future per-platform backends (kqueue on the BSDs / macOS,
   ReadDirectoryChangesW on Windows) plug in here through the same
   interface, selected ahead of polling when their AC_CHECK_HEADERS
   probes succeed.

   wb_wait waits for a change.  On the inotify backend it blocks until
   the kernel reports an event; on the polling backend it sleeps up to
   one tick and then returns whatever it found (possibly nothing).  It
   returns:
       2 = a watched makefile changed (caller should re-exec),
       1 = a watched source changed (caller should rebuild),
       0 = wake-up but no relevant change (caller should call again),
      -1 = signal received or fatal error (caller should die).

   wb_min_latency_ms returns a backend-specific upper bound on how
   long after a build finishes a "self-triggered" event might still
   look like a fresh user edit.  For inotify this is small (events
   stream within milliseconds); for polling it is one tick.  The
   frontend uses it to size the self-trigger window.  */

#ifndef WATCHBACKEND_H
#define WATCHBACKEND_H 1

struct watch_backend;

struct watch_backend *wb_init (void);
int  wb_add (struct watch_backend *wb, const char *dir, const char *base,
             int is_makefile);
int  wb_wait (struct watch_backend *wb);
int  wb_min_latency_ms (const struct watch_backend *wb);
void wb_close (struct watch_backend *wb);

#endif /* WATCHBACKEND_H */
