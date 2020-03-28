/* $Id: function.h,v 1.10 2007/01/04 12:03:20 rockyb Exp $
Copyright (C) 1988, 1989, 1991-1997, 1999, 2002, 2004, 2005
Free Software Foundation, Inc.
Copyright (C) 2005 R. Bernstein <rocky@gnu.org>
This file is part of GNU Make (remake variant).

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

/** \file function.h
 *
 *  \brief Builtin function expansion for GNU Make.
 */

#ifndef FUNCTION_H
#define FUNCTION_H

#include "make.h"

/*!
  Replace all carriage returns and linefeeds with spaces.
  Carriage return is replaced on UNIX as well. Is this desirable?
 */
extern void fold_newlines (char *buffer, unsigned int *length);

extern void hash_init_function_table (void);

extern void hash_free_function_table (void);

/*! Set begpp to point to the first non-whitespace character of the string,
  and endpp to point to the last non-whitespace character of the string.
  If the string is empty or contains nothing but whitespace, endpp will be
  begpp-1.
 */
extern char *strip_whitespace (const char **begpp, const char **endpp);

/*!
  $(eval *makefile-string*)

  Always resolves to the empty string.

  Treat the arguments as a segment of makefile, and parse them.
*/
extern char *func_eval (char *o, char **argv, const char *funcname UNUSED);


#endif /*FUNCTION_H*/
