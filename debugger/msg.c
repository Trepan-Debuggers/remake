/* 
Copyright (C) 2011
R. Bernstein <rocky@gnu.org>
This file is part of GNU Make (remake variant).

GNU Make is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2, or (at your option)
any later version.

GNU Make is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with GNU Make; see the file COPYING.  If not, write to
the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
Boston, MA 02111-1307, USA.  */

/* Debugger messages */
#include "msg.h"

#if HAVE_VPRINTF || HAVE_DOPRNT
# define HAVE_STDVARARGS 1
# if __STDC__
#  include <stdarg.h>
#  define VA_START(args, lastarg) va_start(args, lastarg)
# else
#  include <varargs.h>
#  define VA_START(args, lastarg) va_start(args)
# endif
# if HAVE_VPRINTF
#  define VA_PRINTF(fp, lastarg, args) vfprintf((fp), (lastarg), (args))
# else
#  define VA_PRINTF(fp, lastarg, args) _doprnt((lastarg), (args), (fp))
# endif
# define VA_END(args) va_end(args)
#else
/* # undef HAVE_STDVARARGS */
# define va_alist a1, a2, a3, a4, a5, a6, a7, a8
# define va_dcl char *a1, *a2, *a3, *a4, *a5, *a6, *a7, *a8;
# define VA_START(args, lastarg)
# define VA_PRINTF(fp, lastarg, args) fprintf((fp), (lastarg), va_alist)
# define VA_END(args)
#endif

/* Print an error message on stdout.  */
void
#if __STDC__ && HAVE_STDVARARGS
dbg_errmsg(const char *fmt, ...)
#else
dbg_errmsg (const char *fmt, va_alist)
#endif
{
#if HAVE_STDVARARGS
  va_list args;
#endif
  fprintf(stdout, "** ");
  VA_START (args, fmt);
  VA_PRINTF (stdout, fmt, args);
  VA_END (args);
  fprintf (stdout, "\n");
  fflush (stdout);
}

/* Print a message on stdout.  */
void
#if __STDC__ && HAVE_STDVARARGS
dbg_msg(const char *fmt, ...)
#else
dbg_msg (const char *fmt, va_alist)
#endif
{
#if HAVE_STDVARARGS
  va_list args;
#endif
  VA_START (args, fmt);
  VA_PRINTF (stdout, fmt, args);
  VA_END (args);
  fprintf (stdout, "\n");
  fflush (stdout);
}
