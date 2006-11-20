/* $Id: ar_fns.h,v 1.6 2006/11/20 10:29:13 rockyb Exp $
Copyright (C) 2005 rocky@panix.com
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

/** \file ar_fns.h
 *
 *  \brief Interface to `ar' archives for GNU Make.
 */

#ifndef AR_FNS_H
#define AR_FNS_H

/** Specify we want GNU source code.  This must be defined before any
   system headers are included.  */
#define _GNU_SOURCE 1

#include "types.h"
#include <time.h>

/*! 
   See if psz_name is an archive-member reference or not.

   An archive-member reference is a name like `lib(member)'.  If a
   name like `lib((entry))' is used, a fatal error is signaled at the
   attempt to use this unsupported feature.

   @param psz_name archive-member reference to look up.
   @return true if psz_name is an archive-member reference, false if not.
*/
extern bool ar_name(const char *psz_name);

/*! 
   Parse the archive-member reference into the archive and
   member names.  

   @param psz_name archive-member name to look up.
   @param ppsz_arname place where the malloc'd archive name if it is non-nil
   @param ppsz_memname place to put malloc'd member name if it is non-nil
*/
extern void ar_parse_name (const char *psz_name, char **ppsz_arname, 
			   char **ppsz_memname);

/*! Set the archive-member modtime to now.  
  @param psz_name member modtime name to set
  @return 0 if things went okay 1; if not.
*/
extern int ar_touch (const char *psz_name);

/*! 
  Get modtime date of an archive member.
  @param psz_name archive-member name to retrieve modtime of
  @return the modtime for psz_name. 
*/
extern time_t ar_member_date (const char *psz_name);

#endif /*AR_FNS_H*/
