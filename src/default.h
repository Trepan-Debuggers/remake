/* Header for Data base of default implicit rules for GNU Make.
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

#ifndef DEFAULT_H
#define DEFAULT_H

/*!  
  Create definitions for all of the builtin default variables,
  e.g. CC, LD, MAKE. Nothing is done fault if no_builtin_flag is set.
 */
void define_default_variables (void);

/*! Set up the default .SUFFIXES list.  */
void set_default_suffixes (void);

/*! Enter the default suffix rules as file rules.  This used to be
   done in install_default_implicit_rules, but that loses because we
   want the suffix rules installed before reading makefiles, and thee
   pattern rules installed after.  */
void install_default_suffix_rules (void);

/*! Install the default pattern rules.  */
void install_default_implicit_rules (void);

void free_default_suffix_rules (void);

#endif /*DEFAULT_H*/
