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

#if FILE_TIMESTAMP_HI_RES
# define FILE_TIMESTAMP_STAT_MODTIME(fname, st) \
    file_timestamp_cons (fname, (st).st_mtime, (st).st_mtim.ST_MTIM_NSEC)
#else
# define FILE_TIMESTAMP_STAT_MODTIME(fname, st) \
    file_timestamp_cons (fname, (st).st_mtime, 0)
#endif

/** If FILE_TIMESTAMP is 64 bits (or more), use nanosecond resolution.
   (Multiply by 2**30 instead of by 10**9 to save time at the cost of
   slightly decreasing the number of available timestamps.)  With
   64-bit FILE_TIMESTAMP, this stops working on 2514-05-30 01:53:04
   UTC, but by then uintmax_t should be larger than 64 bits.  */
#define FILE_TIMESTAMPS_PER_S (FILE_TIMESTAMP_HI_RES ? 1000000000 : 1)
#define FILE_TIMESTAMP_LO_BITS (FILE_TIMESTAMP_HI_RES ? 30 : 0)

#define FILE_TIMESTAMP_S(ts) (((ts) - ORDINARY_MTIME_MIN) \
			      >> FILE_TIMESTAMP_LO_BITS)
#define FILE_TIMESTAMP_NS(ts) ((int) (((ts) - ORDINARY_MTIME_MIN) \
				      & ((1 << FILE_TIMESTAMP_LO_BITS) - 1)))

/** Upper bound on length of string "YYYY-MM-DD HH:MM:SS.NNNNNNNNN"
   representing a file timestamp.  The upper bound is not necessarily 19,
   since the year might be less than -999 or greater than 9999.

   Subtract one for the sign bit if in case file timestamps can be negative;
   subtract FLOOR_LOG2_SECONDS_PER_YEAR to yield an upper bound on how many
   file timestamp bits might affect the year;
   302 / 1000 is log10 (2) rounded up;
   add one for integer division truncation;
   add one more for a minus sign if file timestamps can be negative;
   add 4 to allow for any 4-digit epoch year (e.g. 1970);
   add 25 to allow for "-MM-DD HH:MM:SS.NNNNNNNNN".  */
#define FLOOR_LOG2_SECONDS_PER_YEAR 24
#define FILE_TIMESTAMP_PRINT_LEN_BOUND \
  (((sizeof (FILE_TIMESTAMP) * CHAR_BIT - 1 - FLOOR_LOG2_SECONDS_PER_YEAR) \
    * 302 / 1000) \
   + 1 + 1 + 4 + 25)

/** Convert an external file timestamp to internal form.  */
extern FILE_TIMESTAMP file_timestamp_cons (char const *fname, time_t s, 
					   int ns);

/** Return the current time as a file timestamp, setting *RESOLUTION to
   its resolution.  */
extern FILE_TIMESTAMP file_timestamp_now (int *resolution);

/** 
    Place into the buffer P a printable representation of the file
    timestamp TS.
    
    @param p output buffer for printable timestamp
    @param ts timestamp to convert.
 */
extern void file_timestamp_sprintf (char *p, FILE_TIMESTAMP ts);

/*! Expand and parse each dependency line. */
extern void expand_deps (file_t *f);
#endif /*FILE_H*/
