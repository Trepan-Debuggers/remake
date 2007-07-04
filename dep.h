/* Definitions of dependency data structures for GNU Make.
Copyright (C) 1988, 1989, 1990, 1991, 1992, 1993, 1994, 1995, 1996, 1997,
1998, 1999, 2000, 2001, 2002, 2003, 2004, 2005, 2006, 2007 Free Software
Foundation, Inc.
This file is part of GNU Make.

GNU Make is free software; you can redistribute it and/or modify it under the
terms of the GNU General Public License as published by the Free Software
Foundation; either version 3 of the License, or (at your option) any later
version.

GNU Make is distributed in the hope that it will be useful, but WITHOUT ANY
WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR
A PARTICULAR PURPOSE.  See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License along with
this program.  If not, see <http://www.gnu.org/licenses/>.  */

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
   chained through the `next'. `stem' is the stem for this
   dep line of static pattern rule or NULL.

   Note that the first two words of this match a struct nameseq.  */

struct dep
  {
    struct dep *next;
    const char *name;
    const char *stem;
    struct file *file;
    unsigned int changed : 8;
    unsigned int ignore_mtime : 1;
    unsigned int staticpattern : 1;
    unsigned int need_2nd_expansion : 1;
  };


/* Structure used in chains of names, for parsing and globbing.  */

struct nameseq
  {
    struct nameseq *next;
    const char *name;
  };


struct nameseq *multi_glob (struct nameseq *chain, unsigned int size);
#ifdef VMS
struct nameseq *parse_file_seq ();
#else
struct nameseq *parse_file_seq (char **stringp, int stopchar, unsigned int size, int strip);
#endif
char *tilde_expand (const char *name);

#ifndef NO_ARCHIVES
struct nameseq *ar_glob (const char *arname, const char *member_pattern, unsigned int size);
#endif

#define dep_name(d) ((d)->name == 0 ? (d)->file->name : (d)->name)

struct dep *alloc_dep (void);
void free_dep (struct dep *d);
struct dep *copy_dep_chain (const struct dep *d);
void free_dep_chain (struct dep *d);
void free_ns_chain (struct nameseq *n);
struct dep *read_all_makefiles (const char **makefiles);
int eval_buffer (char *buffer);
int update_goal_chain (struct dep *goals);
void uniquize_deps (struct dep *);
