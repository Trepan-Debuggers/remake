/* $Id: arscan.h,v 1.5 2005/12/25 20:53:01 rockyb Exp $
Copyright (C) 2004, 2005 Free Software Foundation, Inc.

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2, or (at your option)
any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307,
USA.  */

/** \file arscan.h
 *
 *  \brief header for scanning an archive file.
 */

#ifndef	ARSCAN_H
#define	ARSCAN_H

/*! 
   Open the given archive, find its members one by one,
   and for each one call function with the following arguments:

   -  member name,
   -  member name might be truncated flag,
   -  member header position in file,
   -  member data position in file,
   -  member data size,
   -  member date,
   -  member uid,
   -  member gid,
   -  member protection mode,
   -  argument supplied as the last parameter.

   @param archive archive file descriptor for reading the data.
   This descriptor is poised to read the data of the member
   when function is called.  It does not matter how much
   data function reads.

   @param function function to call for each member. If function
   returns nonzero, we immediately return what FUNCTION returned.

   @param arg the argument given to function on each iteration.

   @return -1 if archive does not exist, -2 if archive has invalid
   format, and 0 if have scanned successfully.  */

extern long int ar_scan (char *archive, 
			 long int (*function) (), long int arg);

/*! 
  See if an archive member is equal to given name.

  @param name name to test 

  @param mem name to test against. 

  @param truncated if nonzero, MEM may be truncated to sizeof (struct
  ar_hdr.ar_name) - 1.
  
  @return nonzero iff NAME matches MEM.
*/
extern int ar_name_equal (char *name, char *mem, bool truncated);

/*! 
   Set date of member in archive to current time.
  
   @param psz_arname name of archive

   @param psz_memname name of member in archive

   @return 
   -  0 if successful, 
   - -1 if file p_arname does not exist, 
   - -2 if not a valid archive, 
   - -3 if other random system call error (including file read-only), 
   -  1 if valid but member psz_memname does not exist.
*/
extern int ar_member_touch (char *psz_arname, char *psz_memname);

#endif /*ARSCAN_H*/
