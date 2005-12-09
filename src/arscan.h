/* $Id: arscan.h,v 1.3 2005/12/09 12:11:09 rockyb Exp $
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

/*! Takes three arguments ARCHIVE, FUNCTION and ARG.

   Open the archive named ARCHIVE, find its members one by one,
   and for each one call FUNCTION with the following arguments:
     archive file descriptor for reading the data,
     member name,
     member name might be truncated flag,
     member header position in file,
     member data position in file,
     member data size,
     member date,
     member uid,
     member gid,
     member protection mode,
     ARG.

   The descriptor is poised to read the data of the member
   when FUNCTION is called.  It does not matter how much
   data FUNCTION reads.

   If FUNCTION returns nonzero, we immediately return
   what FUNCTION returned.

   Returns -1 if archive does not exist,
   Returns -2 if archive has invalid format.
   Returns 0 if have scanned successfully.  */
extern long int ar_scan (char *archive, 
			 long int (*function) (), long int arg);

/*! Return nonzero iff NAME matches MEM.
   If TRUNCATED is nonzero, MEM may be truncated to
   sizeof (struct ar_hdr.ar_name) - 1.  */
extern int ar_name_equal (char *name, char *mem, int truncated);

/*! Set date of member MEMNAME in archive ARNAME to current time.
   Returns 0 if successful,
   -1 if file ARNAME does not exist,
   -2 if not a valid archive,
   -3 if other random system call error (including file read-only),
   1 if valid but member MEMNAME does not exist.
*/
extern int ar_member_touch (char *arname, char *memname);

#endif /*ARSCAN_H*/
