/* $Id: read.h,v 1.7 2007/01/04 12:03:20 rockyb Exp $ 
Copyright (C) 2004, 2005 Free Software Foundation, Inc.
Copyright (C) 2008 R. Bernstein <rocky@gnu.org>

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

/** \file read.h
 *
 *  \brief Header for Reading and parsing of makefiles for GNU Make.
 */

#ifndef READ_H
#define READ_H

#include "dep.h"

extern stringlist_t *include_directories;

/*! The chain of makefiles read by read_makefile.  */
extern dep_t *read_makefiles;

int eval_buffer (char *buffer);


/*! Search STRING for an unquoted STOPCHAR or blank (if BLANK is nonzero).
   Backslashes quote STOPCHAR, blanks if BLANK is nonzero, and backslash.
   Quoting backslashes are removed from STRING by compacting it into
   itself.  Returns a pointer to the first unquoted STOPCHAR if there is
   one, or nil if there are none.  */

char *find_char_unquote (char *string, int stop1, int stop2, 
			 int blank);

/*! Search PATTERN for an unquoted %.  */
char *find_percent (char *pattern);

/*! Construct the list of include directories
   from the arguments and the default list.  
*/
extern void construct_include_path (char **arg_dirs);

/*! Expand ~ or ~USER at the beginning of NAME.
   Return a newly malloc'd string or 0.  
*/
char *tilde_expand (char *name);

/*! Given a chain of struct nameseq's describing a sequence of filenames,
   in reverse of the intended order, return a new chain describing the
   result of globbing the filenames.  The new chain is in forward order.
   The links of the old chain are freed or used in the new chain.
   Likewise for the names in the old chain.

   SIZE is how big to construct chain elements.
   This is useful if we want them actually to be other structures
   that have room for additional info.
*/
nameseq_t * multi_glob (struct nameseq *chain, unsigned int size);

#endif /*READ_H*/
