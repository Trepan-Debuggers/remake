/* $Id: expand.h,v 1.7 2005/12/23 04:20:38 rockyb Exp $
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

/** \file expand.h
 *
 *  \brief Header for variable expansion functions for GNU Make.
 */

#ifndef EXPAND_H
#define EXPAND_H

#include "variable.h"
#include "file.h"

#define	allocated_variable_expand(line) \
  allocated_variable_expand_for_file (line, (file_t *) 0)

/*! Like variable_expand_for_file, but the returned string is malloc'd.
   This function is called a lot.  It wants to be efficient.  */

extern char *allocated_variable_expand_for_file(char *psz_line, 
						file_t *p_file);


/*! Expand an argument for an expansion function.  The text starting
   at STR and ending at END is variable-expanded into a
   null-terminated string that is returned as the value.  This is done
   without clobbering `variable_buffer' or the current
   variable-expansion that is in progress.  */

extern char *expand_argument(const char *str, const char *end);

/*! Install a new variable_buffer context, returning the current one for
   safe-keeping.  */

extern void install_variable_buffer (char **pp_buf, unsigned int *pi_len);

/*! Restore a previously-saved variable_buffer setting (free the current one).
 */

#define recursively_expand(v)   recursively_expand_for_file (v, NULL)

/*! Recursively expand V.  The returned string is malloc'd.  */
extern char *recursively_expand_for_file(variable_t *v, file_t *file);

/*! Subroutine of variable_expand and friends:
   The text to add is LENGTH chars starting at STRING to the variable_buffer.
   The text is added to the buffer at PTR, and the updated pointer into
   the buffer is returned as the value.  Thus, the value returned by
   each call to variable_buffer_output should be the first argument to
   the following call.  */

extern void restore_variable_buffer(char *p_buf, unsigned int len);

extern char *variable_buffer_output(char *ptr, char *psz_string, 
				    unsigned int length);

/*! Scan PSZ_LINE for variable references and expansion-function calls.
   Build in `variable_buffer' the result of expanding the references and calls.
   Return the address of the resulting string, which is null-terminated
   and is valid only until the next time this function is called.  */

extern char *variable_expand(char *psz_line);

/** Expand PSZ_LINE. Expansion uses P_FILE_SET if it is not NULL. */
extern char *variable_expand_set (char *psz_line, 
				  variable_set_list_t *p_file_set);

/*! Scan STRING for variable references and expansion-function calls.  Only
   LENGTH bytes of STRING are actually scanned.  If LENGTH is -1, scan until
   a null byte is found.

   Write the results to PSZ_LINE, which must point into `variable_buffer'.  If
   LINE is NULL, start at the beginning of the buffer.
   Return a pointer to PSZ_LINE, or to the beginning of the buffer if 
   PSZ_LINE is NULL.  */

char *variable_expand_string(char *line, char *string, long int length);

#endif /*EXPAND_H*/
