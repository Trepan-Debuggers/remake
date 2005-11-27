/* $Id: ar_fns.h,v 1.2 2005/11/27 20:38:01 rockyb Exp $
Copyright (C) 2005 Free Software Foundation, Inc.
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
the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
Boston, MA 02111-1307, USA.  */

#ifndef AR_FNS_H
#define AR_FNS_H

#include <time.h>

/*! Return nonzero if PSZ_NAME is an archive-member reference, zero if not.
   An archive-member reference is a name like `lib(member)'.  If a
   name like `lib((entry))' is used, a fatal error is signaled at the
   attempt to use this unsupported feature.
*/
extern int ar_name(char *psz_name);

/*! Parse the archive-member reference NAME into the archive and
   member names.  Put the malloc'd archive name in *ARNAME_P if
   ARNAME_P is non-nil; put the malloc'd member name in *MEMNAME_P if
   MEMNAME_P is non-nil.
*/
extern void ar_parse_name (char *name, char **arname_p, char **memname_p);

/*! Set the archive-member NAME's modtime to now.  */
extern int ar_touch (char *psz_name);

/*! Return the modtime of PSZ_NAME.  */
extern time_t ar_member_date (char *);

#endif /*AR_FNS_H*/
