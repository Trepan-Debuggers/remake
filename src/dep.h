/* Definitions of dependency data structures for GNU Make.
Copyright (C) 1988, 1989, 1991, 1992, 1993, 1996, 2004
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
the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
Boston, MA 02111-1307, USA.  */

#ifndef DEP_H
#define DEP_H

#include "hash.h"

/* Flag bits for the second argument to `read_makefile'.
   These flags are saved in the `changed' field of each
   `struct dep' in the chain returned by `read_all_makefiles'.  */

#define RM_NO_DEFAULT_GOAL	(1 << 0) /* Do not set default goal.  */
#define RM_INCLUDED		(1 << 1) /* Search makefile search path.  */
#define RM_DONTCARE		(1 << 2) /* No error if it doesn't exist.  */
#define RM_NO_TILDE		(1 << 3) /* Don't expand ~ in file name.  */
#define RM_NOFLAG		0

/* Structure representing one dependency of a file.
   Each struct file's `deps' points to a chain of these,
   chained through the `next'.

   Note that the first two words of this match a struct nameseq.  */

struct dep
  {
    struct dep *next;
    char *name;
    struct file *file;
    unsigned int changed : 8;
    unsigned int ignore_mtime : 1;
  };

typedef struct dep dep_t;

/* Structure used in chains of names, for parsing and globbing.  */

struct nameseq
  {
    struct nameseq *next;
    char *name;
    floc_t floc;
  };

typedef struct nameseq nameseq_t;

#ifndef NO_ARCHIVES
extern nameseq_t *ar_glob PARAMS ((char *arname, char *member_pattern, 
				   unsigned int size));
#endif

/*! Whether or not .SECONDARY with no prerequisites was given.  */
extern int all_secondary;

#ifndef	iAPX286
#define dep_name(d) ((d)->name == 0 ? (d)->file->name : (d)->name)
#else
/* Buggy compiler can't hack this.  */
extern char *dep_name ();
#endif

/*! Copy dependency chain making a new chain with the same contents
  as the old one and return that.  The return value is malloc'd. The
  caller must thus free it.
 */
extern struct dep *copy_dep_chain PARAMS ((dep_t *d));

extern struct dep *read_all_makefiles PARAMS ((char **makefiles));

/*! For each dependency of each file, make the `struct dep' point
   at the appropriate `struct file' (which may have to be created).

   Also mark the files depended on by .PRECIOUS, .PHONY, .SILENT,
   and various other special targets.  */
extern void snap_deps PARAMS ((hash_table_t *p_files));

/*! Remove duplicate dependencies in CHAIN.  */
extern void uniquize_deps PARAMS ((dep_t * chain));

#endif /*DEP_H*/
