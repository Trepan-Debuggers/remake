/* $Id: misc.h,v 1.1 2005/12/11 12:15:51 rockyb Exp $
Copyright (C) 2005 rocky@panix.com
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

/** \file misc.h 
 *
 *  \brief Miscellaneous generic support functions for GNU Make.
 */

#ifndef MISC_H
#define MISC_H
#include "make.h"

/*! Close standard output, exiting with status 'exit_failure' on
   failure.  If a program writes *anything* to stdout, that program
   should close stdout and make sure that it succeeds before exiting.
   Otherwise, suppose that you go to the extreme of checking the
   return status of every function that does an explicit write to
   stdout.  The last printf can succeed in writing to the internal
   stream buffer, and yet the fclose(stdout) could still fail (due
   e.g., to a disk full error) when it tries to write out that
   buffered data.  Thus, you would be left with an incomplete output
   file and the offending program would exit successfully.  Even
   calling fflush is not always sufficient, since some file systems
   (NFS and CODA) buffer written/flushed data until an actual close
   call.

   Besides, it's wasteful to check the return value from every call
   that writes to stdout -- just let the internal stream state record
   the failure.  That's what the ferror test is checking below.

   It's important to detect such failures and exit nonzero because many
   tools (most notably `make' and other build-management systems) depend
   on being able to detect failure in other tools via their exit status.  */

extern void close_stdout (void);

/*! Give the process appropriate permissions for a child process.
  This is like user_access, but you can't get back to make_access.  */

extern void child_access (void);

/*! Return the address of the first whitespace or null in the string
    S.  */
extern char *end_of_token (const char *s);

#ifdef WINDOWS32
/*! Same as end_of_token, but take into account a stop character
 */
extern char * end_of_token_w32 (char *s, char stopchar);
#endif

/*! Find the next token in PTR; return the address of it, and store
    the length of the token into *LENGTHPTR if LENGTHPTR is not
    nil.  */
extern char * find_next_token (char **ptr, unsigned int *lengthptr);

#ifdef NEED_GET_PATH_MAX
extern unsigned int get_path_max (void);
#endif

/*! Limited INDEX:
  Search through the string STRING, which ends at LIMIT, for the character C.
  Returns a pointer to the first occurrence, or nil if none is found.
  Like INDEX except that the string searched ends where specified
  instead of at the first null.  */
extern char *lindex (const char *s, const char *limit, int c);

/*! Give the process appropriate permissions for access to make data
    (i.e., the load average).  */
extern void make_access (void);

/*! Return the address of the first nonwhitespace or null in the
    string S.  */
extern char * next_token (const char *s);

extern char *savestring (const char *str, unsigned int length);

/*! Give the process appropriate permissions for access to user data
    (i.e., to stat files, or to spawn a child process).  */
extern void user_access (void);

#endif /*MISC_H*/
