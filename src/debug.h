/* Debugging macros and interface.
Copyright (C) 1999, 2004 Free Software Foundation, Inc.
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

#define DB_NONE         (0x000)
#define DB_BASIC        (0x001)
#define DB_VERBOSE      (0x002)
#define DB_JOBS         (0x004)
#define DB_IMPLICIT     (0x008)
#define DB_MAKEFILES    (0x100)

#define DB_ALL          (0xfff)

#define DEBUGGER_ON_ERROR  0x1   /* Enter debugger on any error */
#define DEBUGGER_ON_FATAL  0x2   /* Enter debugger on a fatal error */
#define DEBUGGER_ON_SIG    0x4   /* Enter debugger on getting a signal */

extern int db_level;

/*! If 1, we give additional error reporting information. */
extern int extended_errors;

/*! If 1, we show variable definitions */
extern int show_variable_definitions;

/*! If 1, we are tracing execution */
extern int tracing;

/*! If nonzero, we are debugging after each "step" for that many times. 
  When we have a value 1, then we actually run the debugger read loop.
  Otherwise we decrement the step count.

*/
extern unsigned int debugger_stepping;

/*! If nonzero, enter the debugger if we hit a fatal error.
*/
extern unsigned int debugger_on_error;

/*! If nonzero, we have requested some sort of debugging.
*/
extern unsigned int debugger_enabled;

#define ISDB(_l)    ((_l)&db_level)

#define DBS(_l,_x)  do{ if(ISDB(_l)) {print_spaces (depth); \
                                      printf _x; fflush (stdout);} }while(0)

#define DBF(_l,_x)  do{ if(ISDB(_l))					\
      {									\
	print_spaces (depth);						\
	if (file->floc.filenm) {					\
	  print_floc_prefix(&file->floc);				\
	  printf("\t");							\
	}								\
	printf (_x, file->name);					\
	fflush (stdout);} }while(0)

#define DB(_l,_x)   do{ if(ISDB(_l)) {printf _x; fflush (stdout);} }while(0)

#endif /*DEBUG_H*/
