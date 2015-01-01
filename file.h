/* $Id: file.h,v 1.11 2006/03/30 05:01:49 rockyb Exp $
Copyright (C) 1988, 1989, 1990, 1991, 1992, 1993, 1994, 1997,
2002, 2004, 2005 Free Software Foundation, Inc.
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

/** \file file.h
 *
 *  \brief Definition of target file data structures for GNU Make.
 */

#ifndef FILE_H
#define FILE_H

#include "make.h"
#include "types.h"
#include "hash.h"

extern struct hash_table files;

/*! Free memory associated with p_file. */
void    free_file  (file_t *p_file);

/*!
  Remove all nonprecious intermediate files.

  @param sig if is nonzero, this was caused by a fatal signal,
  meaning that a different message will be printed, and
  the message will go to stderr rather than stdout.
*/
void    remove_intermediates (int sig);

void    init_hash_files (void);

char   *build_target_list (char *old_list);

/*! Thing of the below as a bit mask rather than an enumeration and
    use print_target_mask;
    The enumeration is created be helpful in debuggers where wants just to
    refer to the PRINT_TARGET_ names and get something.
*/
typedef enum
{
  PRINT_TARGET_NONORDER  = 0x001,
  PRINT_TARGET_ORDER     = 0x002,
  PRINT_TARGET_ATTRS     = 0x004,
  PRINT_TARGET_TIME      = 0x008,
  PRINT_TARGET_STATE     = 0x010,
  PRINT_TARGET_VARS      = 0x020,
  PRINT_TARGET_VARS_HASH = 0x040,
  PRINT_TARGET_CMDS      = 0x080,
  PRINT_TARGET_PREV      = 0x100,
  PRINT_TARGET_CMDS_EXP  = 0x200,
  PRINT_TARGET_DEPEND    = (PRINT_TARGET_ORDER|PRINT_TARGET_NONORDER),
  PRINT_TARGET_ALL       = 0x0FF,
} print_target_mask_t;

/* The below variable is to make sure the enumerations are accessible
   in a debugger. */
extern print_target_mask_t debugger_enum_mask;

/*! Print the data base of files.  */
extern void  print_target (const void *item);

/*! Print some or all properties of the data base of files.  */
extern void  print_target_props (file_t *p_target, print_target_mask_t i_mask);

/*! Expand and parse each dependency line. */
extern void expand_deps (file_t *f);
#endif /*FILE_H*/
