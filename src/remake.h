/* $Id: remake.h,v 1.6 2005/12/25 20:53:01 rockyb Exp $
Copyright (C) 2004, 2005
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

/** \file remake.h
 *
 *  \brief Header for basic dependency engine for GNU Make.
 */

/*! Remake all the goals in the `struct dep' chain GOALS.  Return -1
   if nothing was done, 0 if all goals were updated successfully, or 1
   if a goal failed.  If MAKEFILES is nonzero, these goals are
   makefiles, so -t, -q, and -n should be disabled for them unless
   they were also command-line targets, and we should only make one
   goal at a time and return as soon as one goal whose `changed'
   member is nonzero is successfully made.  */

#ifndef REMAKE_H
#define REMAKE_H

#include "dep.h"
#include "file.h"

/*! 
  Get the mtime of a file.  If the file is not found, and search is
  nonzero, VPATH searching and replacement is done.  If that fails, a
  library (-lLIBNAME) is tried and the library's actual name
  (/lib/libLIBNAME.a, etc.) is substituted in


  @param p_file pointer to file to get mtime of. To avoid excess stat calls,
  the time in the file is cached.

  @param search whether to do vpath searching.

  @return the mtime of a file.  
*/

extern FILE_TIMESTAMP f_mtime (file_t *p_file, bool search);

/*! Set FILE's `updated' flag and re-check its mtime and the mtime's
  of all files listed in its `also_make' member.  Under -t, this
  function also touches FILE.
  
  On return, FILE->update_status will no longer be -1 if it was.
*/
extern void notice_finished_file (file_t *file);

/*! Remake all the goals in the `struct dep' chain GOALS.  Return -1
   if nothing was done, 0 if all goals were updated successfully, or 1
   if a goal failed.  If MAKEFILES is nonzero, these goals are
   makefiles, so -t, -q, and -n should be disabled for them unless
   they were also command-line targets, and we should only make one
   goal at a time and return as soon as one goal whose `changed'
   member is nonzero is successfully made.
*/
extern int update_goal_chain (dep_t *goals, int makefiles);

#endif /*REMAKE_H*/
