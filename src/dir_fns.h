/* $Id: dir_fns.h,v 1.4 2005/12/17 19:44:09 rockyb Exp $

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

/** \file dir_fns.h
 *
 *  \brief Definition of directory routines for GNU Make.
 */


#ifndef __DIR_FNS_H__
#define __DIR_FNS_H__

#include <glob.h>

/*! Return 1 if the name PSZ_FILENAME in directory PSZ_DIRNAME
   is entered in the dir hash table.
   FILENAME must contain no slashes.  */
extern int dir_file_exists_p (char * psz_dirname, char *psz_filename);

/*! Return 1 if the name PSZ_FILENAME in directory PSZ_DIRNAME is
   entered in the dir hash table.  PSZ_FILENAME must contain no
   slashes.  */
extern int file_exists_p (char *);

/*! Mark PSZ_FILENAME as `impossible' for `file_impossible_p'.
  This means an attempt has been made to search for FILENAME
  as an intermediate file, and it has failed.  */
extern void file_impossible (char *psz_filename);

/*! Return nonzero if PSZ_FILENAME has been marked impossible.  */
extern int file_impossible_p (char *psz_filename);

/* Return the already allocated name in the
   directory hash table that matches DIR.  */
extern char *dir_name (char *psz_dir);

/*! Initialize directory hash tables */
extern void hash_init_directories (void);

/*! Free directory hash tables */
extern void hash_free_directories (void);

extern void dir_setup_glob(glob_t *p_gl);

#endif /*__DIR_FNS_H__*/
