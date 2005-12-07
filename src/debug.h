/* $Id: debug.h,v 1.9 2005/12/07 03:30:54 rockyb Exp $
Debugging macros and interface.
Copyright (C) 1999, 2004, 2005 Free Software Foundation, Inc.
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

#ifndef DEBUG_H
#define DEBUG_H

#include "types.h"

/**
   Imagine the below enums values as #define'd values rather than
   distinct values of an enum.
*/
typedef enum {
  DB_NONE           = 0x000,
  DB_BASIC          = 0x001, /** targets which need to be made and status;
				 also set when tracing or debugging */
  DB_VERBOSE        = 0x002,
  DB_JOBS           = 0x004,
  DB_IMPLICIT       = 0x008,
  DB_MAKEFILES      = 0x100,
  DB_READMAKEFILES  = 0x200, /** Reading makefiles */
  DB_CALLTRACE      = 0x400, /** GNU Make function call and returns */
  DB_ALL            = 0xfff
} debug_level_mask_t;

typedef enum {
  DEBUGGER_ON_ERROR  = 0x1,   /**< Enter debugger on any error */
  DEBUGGER_ON_FATAL  = 0x2,   /**< Enter debugger on a fatal error */
  DEBUGGER_ON_SIG    = 0x4    /**< Enter debugger on getting a signal */
} debug_enter_debugger_t;
  

/** These variables are trickery to force the above enum symbol values to
    be recorded in debug symbol tables. It is used to allow one refer
    to above enumeration values in a debugger and debugger
    expressions */
extern debug_level_mask_t debug_dummy_level_mask;
extern debug_enter_debugger_t debug_dummy_enter_debugger_mask;

/** bitmask of debug_level_mask values. */
extern int db_level;

/** The structure used to hold the list of strings given
    in command switches of a type that takes string arguments.  */

typedef struct stringlist
{
  char **list;	/* Nil-terminated list of strings.  */
  unsigned int idx;	/* Index into above.  */
  unsigned int max;	/* Number of pointers allocated.  */
} stringlist_t;

extern int debug_flag;

/*! If 1, we give additional error reporting information. */
extern int extended_errors;

/*! If 1, we show variable definitions */
extern int show_variable_definitions;

/*! If non-null, we are tracing execution */
extern int tracing;

/*! If true, enter the debugger before reading any makefiles. */
extern bool b_debugger_preread;

/*! If nonzero, we are debugging after each "step" for that many times. 
  When we have a value 1, then we actually run the debugger read loop.
  Otherwise we decrement the step count.

*/
extern unsigned int i_debugger_stepping;

/*! If nonzero, we are debugging after each "next" for that many times. 
  When we have a value 1, then we actually run the debugger read loop.
  Otherwise we decrement the step count.

*/
extern unsigned int i_debugger_nexting;

/*! If nonzero, enter the debugger if we hit a fatal error.
*/
extern unsigned int debugger_on_error;

/*! If nonzero, we have requested some sort of debugging.
*/
extern unsigned int debugger_enabled;

extern stringlist_t *db_flags;

#define ISDB(_l)    ((_l)&db_level)

/*! Debugged print */
#define DBPRINT(_x)           \
   printf _x; fflush (stdout)

/*! Debugged print indented a number of spaces given by "_depth" */
#define DBPRINTS(_x, _depth) \
   print_spaces (_depth);    \
   DBPRINT(_x)

/*! Debugged print if debug mask is set indented a number of spaces 
    implied by global variable "depth"
*/
#define DBS(_l,_x)           \
  do {                       \
    if(ISDB(_l)) {           \
       DBPRINTS(_x, depth);  \
    }                        \
  } while(0)

/*! Debugged print if debug mask is set indented a number of spaces 
    given by "_depth"
*/
#define DBSD(_l,_x,_depth)   \
  do {                       \
    if(ISDB(_l)) {           \
      DBPRINTS(_x, _depth);   \
    }                        \
  } while(0)

#define DBF(_l,_x)  do{ if(ISDB(_l))					\
      {									\
	print_spaces (depth);						\
	if (file->floc.filenm) {					\
	  print_floc_prefix(&file->floc);				\
	  printf("\t");							\
	}								\
	printf (_x, file->name);					\
	fflush (stdout);} }while(0)

#define DB(_l,_x)   do{ if(ISDB(_l)) {DBPRINT(_x);} }while(0)

/** Toggle -d on receipt of SIGUSR1.  */
#ifdef SIGUSR1
RETSIGTYPE debug_signal_handler (int sig);
#endif

extern void decode_debug_flags (void);

#endif /*DEBUG_H*/
