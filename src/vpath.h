/* header for pattern-matching file search paths for GNU Make.
Copyright (C) 2004 Free Software Foundation, Inc.
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

#ifndef VPATH_H
#define VPATH_H

/*! Reverse the chain of selective VPATH lists so they
   will be searched in the order given in the makefiles
   and construct the list from the VPATH variable.  */

extern void build_vpath_lists PARAMS ((void));

/*! Construct the VPATH listing for the pattern and searchpath given.

   This function is called to generate selective VPATH lists and also for
   the general VPATH list (which is in fact just a selective VPATH that
   is applied to everything).  The returned pointer is either put in the
   linked list of all selective VPATH lists or in the GENERAL_VPATH
   variable.

   If SEARCHPATH is nil, remove all previous listings with the same
   pattern.  If PATTERN is nil, remove all VPATH listings.  Existing
   and readable directories that are not "." given in the searchpath
   separated by the path element separator (defined in make.h) are
   loaded into the directory hash table if they are not there already
   and put in the VPATH searchpath for the given pattern with trailing
   slashes stripped off if present (and if the directory is not the
   root, "/").  The length of the longest entry in the list is put in
   the structure as well.  The new entry will be at the head of the
   VPATHS chain.  */

extern void construct_vpath_list PARAMS ((char *pattern, char *dirpath));

/*! Search the GPATH list for a pathname string that matches the one
   passed in.  If it is found, return 1.  Otherwise we return 0.  */
extern int gpath_search PARAMS ((char *file, unsigned int len));

/*! Search the VPATH list whose pattern matches *FILE for a directory
   where the name pointed to by FILE exists.  If it is found, we set
   *FILE to the newly malloc'd name of the existing file, *MTIME_PTR
   (if MTIME_PTR is not NULL) to its modtime (or zero if no stat call
   was done), and return 1.  Otherwise we return 0.  */

extern int vpath_search PARAMS ((char **file, FILE_TIMESTAMP *mtime_ptr));

/*! Print the data base of VPATH search paths.  */

extern void print_vpath_data_base PARAMS ((void));

#endif /* VPATH_H */
