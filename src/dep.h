/* $Id: dep.h,v 1.10 2006/11/20 10:29:13 rockyb Exp $
Definitions of dependency data structures for GNU Make.
Copyright (C) 1988, 1989, 1990, 1991, 1992, 1993, 1994, 1995, 1996, 1997,
1998, 1999, 2000, 2001, 2002, 2003, 2004, 2005, 2006 Free Software
Foundation, Inc.
This file is part of GNU Make.

GNU Make is free software; you can redistribute it and/or modify it under the
terms of the GNU General Public License as published by the Free Software
Foundation; either version 2, or (at your option) any later version.

GNU Make is distributed in the hope that it will be useful, but WITHOUT ANY
WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR
A PARTICULAR PURPOSE.  See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License along with
GNU Make; see the file COPYING.  If not, write to the Free Software
Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301 USA.  */

#ifndef DEP_H
#define DEP_H

/** \file dep.h
 *
 *  \brief Definitions of dependency data structures for GNU Make.
 */

#include "types.h"
#include "hash.h"

/* Flag bits for the second argument to `read_makefile'.
   These flags are saved in the `changed' field of each
   `struct dep' in the chain returned by `read_all_makefiles'.  */

#define RM_NO_DEFAULT_GOAL	(1 << 0) /* Do not set default goal.  */
#define RM_INCLUDED		(1 << 1) /* Search makefile search path.  */
#define RM_DONTCARE		(1 << 2) /* No error if it doesn't exist.  */
#define RM_NO_TILDE		(1 << 3) /* Don't expand ~ in file name.  */
#define RM_NOFLAG		0

/** \brief Structure representing one dependency of a file.  Each
   struct file's `deps' points to a chain of these, chained through
   the `next'. `stem' is the stem for this dep line of static pattern
   rule or NULL.

   Note that the first two words of this match a struct nameseq.  */

struct dep {
  dep_t *next;
  char *name;
  char *stem;
  file_t *file;
  unsigned int changed : 8;
  bool ignore_mtime;
  unsigned int staticpattern : 1;
  unsigned int need_2nd_expansion : 1;
};

/** \brief Structure used in chains of names, for parsing and globbing.  */

struct nameseq {
  nameseq_t *next;
  char *name;
  floc_t floc;
};

#ifndef NO_ARCHIVES
extern nameseq_t *ar_glob (const char *arname, char *member_pattern, 
			   unsigned int size);
#endif

/*! Whether or not .SECONDARY with no prerequisites was given.  */
extern int all_secondary;

#define dep_name(d) ((d)->name == 0 ? (d)->file->name : (d)->name)

/*! Allocate a new `struct dep' with all fields initialized to 0.   */
extern dep_t *alloc_dep (void);

/*! Free `struct dep' along with `name' and `stem'.   */
extern void dep_free (dep_t *d);

/*! Copy dependency chain making a new chain with the same contents
  as the old one and return that.  The return value is malloc'd. The
  caller must thus free it.
 */
dep_t *copy_dep_chain (dep_t *p_dep);

/*! Free memory associated with a dependency chain.  */
extern void dep_chain_free (dep_t *p_dep);

/*! Free all p_namseq memory.  */
extern void nameseq_free(nameseq_t *p_nameseq);

/*! Convert name sequence p_nameseq, into to a dependency chain
  as the old one and return that.  The return value is malloc'd. The
  caller must thus free it. p_nameseq is free'd.
 */
extern dep_t * nameseq_to_dep_chain (nameseq_t *p_nameseq);

dep_t *read_all_makefiles (char **makefiles);

/*! Set the intermediate flag.  */
extern void set_intermediate (const void *item);

/*! For each dependency of each file, make the `struct dep' point
   at the appropriate `struct file' (which may have to be created).

   Also mark the files depended on by .PRECIOUS, .PHONY, .SILENT,
   and various other special targets.  */
extern void snap_deps (hash_table_t *p_files);

/*! Remove duplicate dependencies in CHAIN.  */
extern void uniquize_deps (dep_t * chain);

/* Have we snapped deps yet?  */
extern bool snapped_deps;

#endif /*DEP_H*/
