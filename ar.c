/* Copyright (C) 1988, 1989, 1990, 1991, 1992, 1993
	Free Software Foundation, Inc.
This file is part of GNU Make.

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
the Free Software Foundation, 675 Mass Ave, Cambridge, MA 02139, USA.  */

#include "make.h"
#include "file.h"

#ifndef	NO_ARCHIVES

/* Defined in arscan.c.  */
extern long int ar_scan ();
extern int ar_member_touch ();
extern int ar_name_equal ();


/* Return nonzero if NAME is an archive-member reference, zero if not.
   An archive-member reference is a name like `lib(member)'.
   If a name like `lib((entry))' is used, a fatal error is signaled at
   the attempt to use this unsupported feature.  */

int
ar_name (name)
     char *name;
{
  char *p = index (name, '('), *end = name + strlen (name) - 1;
  
  if (p == 0 || p == name || *end != ')')
    return 0;

  if (p[1] == '(' && end[-1] == ')')
    fatal ("attempt to use unsupported feature: `%s'", name);

  return 1;
}


/* Parse the archive-member reference NAME into the archive and member names.
   Put the malloc'd archive name in *ARNAME_P if ARNAME_P is non-nil;
   put the malloc'd member name in *MEMNAME_P if MEMNAME_P is non-nil.  */

void
ar_parse_name (name, arname_p, memname_p)
     char *name, **arname_p, **memname_p;
{
  char *p = index (name, '('), *end = name + strlen (name) - 1;

  if (arname_p != 0)
    *arname_p = savestring (name, p - name);

  if (memname_p != 0)
    *memname_p = savestring (p + 1, end - (p + 1));
}  

static long int ar_member_date_1 ();

/* Return the modtime of NAME.  */

time_t
ar_member_date (name)
     char *name;
{
  char *arname;
  int arname_used = 0;
  char *memname;
  long int val;

  ar_parse_name (name, &arname, &memname);

  /* Make sure we know the modtime of the archive itself because
     we are likely to be called just before commands to remake a
     member are run, and they will change the archive itself.  */
  {
    struct file *arfile;
    arfile = lookup_file (arname);
    if (arfile == 0)
      {
	arfile = enter_file (arname);
	arname_used = 1;
      }

    (void) f_mtime (arfile, 0);
  }

  val = ar_scan (arname, ar_member_date_1, (long int) memname);

  if (!arname_used)
    free (arname);
  free (memname);

  return (val <= 0 ? (time_t) -1 : (time_t) val);
}

/* This function is called by `ar_scan' to find which member to look at.  */

/* ARGSUSED */
static long int
ar_member_date_1 (desc, mem, truncated,
		  hdrpos, datapos, size, date, uid, gid, mode, name)
     int desc;
     char *mem;
     int truncated;
     long int hdrpos, datapos, size, date;
     int uid, gid, mode;
     char *name;
{
  return ar_name_equal (name, mem, truncated) ? date : 0;
}

/* Set the archive-member NAME's modtime to now.  */

int
ar_touch (name)
     char *name;
{
  char *arname, *memname;
  int arname_used = 0;
  register int val;

  ar_parse_name (name, &arname, &memname);

  /* Make sure we know the modtime of the archive itself before we
     touch the member, since this will change the archive itself.  */
  {
    struct file *arfile;
    arfile = lookup_file (arname);
    if (arfile == 0)
      {
	arfile = enter_file (arname);
	arname_used = 1;
      }

    (void) f_mtime (arfile, 0);
  }

  val = 1;
  switch (ar_member_touch (arname, memname))
    {
    case -1:
      error ("touch: Archive `%s' does not exist", arname);
      break;
    case -2:
      error ("touch: `%s' is not a valid archive", arname);
      break;
    case -3:
      perror_with_name ("touch: ", arname);
      break;
    case 1:
      error ("touch: Member `%s' does not exist in `%s'", memname, arname);
      break;
    case 0:
      val = 0;
      break;
    default:
      error ("touch: Bad return code from ar_member_touch on `%s'", name);
    }

  if (!arname_used)
    free (arname);
  free (memname);

  return val;
}

#endif
