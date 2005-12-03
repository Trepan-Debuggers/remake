/* $Id: print.h,v 1.6 2005/12/03 01:27:45 rockyb Exp $
Header for output or logging functions for GNU Make.
Copyright (C) 2004, 2005 Free Software Foundation, Inc.
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

#ifndef PRINT_H
#define PRINT_H

#include "variable.h"
#include "job.h"
#include "trace.h"

/*! Fancy processing for variadic functions in both ANSI and pre-ANSI
   compilers.  */
#if defined __STDC__ && __STDC__
extern void message (int prefix, const char *fmt, ...)
                     __attribute__ ((__format__ (__printf__, 2, 3)));
extern void error (const floc_t *flocp, const char *fmt, ...)
                   __attribute__ ((__format__ (__printf__, 2, 3)));
extern void fatal (const floc_t *flocp, const char *fmt, ...)
                   __attribute__ ((__format__ (__printf__, 2, 3)));
#else
extern void message ();
extern void error ();
extern void fatal ();
#endif

/*! Versions of error and fatal with the ability to show call-stack. */
#if defined __STDC__ && __STDC__
extern void err (target_stack_node_t *p_call, const char *fmt, ...)
                   __attribute__ ((__format__ (__printf__, 2, 3)));
extern void fatal_err (target_stack_node_t *p_call, const char *fmt, ...)
                   __attribute__ ((noreturn, __format__ (__printf__, 2, 3)));
#else
extern void err();
extern void fatal_err();
#endif

/* Think of the below not as an enumeration but as #defines done in a
   way that we'll be able to use the value in a gdb. */
extern enum debug_print_enums_e {
  MAX_STACK_SHOW = 1000,
} debug_print_enums1;

/*! Under -d, write a message describing the current IDs.  */
extern void log_access (char *flavor);

/*! Write a message indicating that we've just entered or
  left (according to ENTERING) the current directory.  */
extern void log_working_directory (int);

/*! Print an error message from errno.  */
extern void perror_with_name (const char *, const char *);

/*! Print an error message from errno and exit.  */
extern void pfatal_with_name (const char *psz_name);

/*! Display a variable and its value. */
extern void print_variable (variable_t *p_v);

/*! Display a variable and its value with all substitutions included. */
extern void print_variable_expand (variable_t *p_v);

/*! Display common file target prefix message output file target. */
extern void print_file_target_prefix (const file_t *p_target);

/*! Display common target prefix message. */
extern void print_target_prefix (const char *p_name);

/*! Display common prefix message output file target. */
extern void print_floc_prefix (const floc_t *p_floc);

/*! Display common prefix message output file target. */
extern debug_return_t print_child_cmd (child_t *p_child, 
				       target_stack_node_t *p);

/*! Display the target stack i_pos is the position we are currently.
  i_max is the maximum number of entries to show.
 */
extern void print_target_stack (target_stack_node_t *p, int i_pos, int i_max);

/*! Display the Makefile read stack. i_pos is the position we are currently.
  i_max is the maximum number of entries to show. */
extern void print_floc_stack (int i_pos, int i_max);

#endif /*PRINT_H*/
