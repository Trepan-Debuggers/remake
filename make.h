/* Copyright (C) 1988, 1989, 1990, 1991, 1992, 1993
	Free Software Foundation, Inc.
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
the Free Software Foundation, 675 Mass Ave, Cambridge, MA 02139, USA.  */

/* AIX requires this to be the first thing in the file.  */
#if defined (_AIX) && !defined (__GNUC__)
 #pragma alloca
#endif

/* We use <config.h> instead of "config.h" so that a compilation
   using -I. -I$srcdir will use ./config.h rather than $srcdir/config.h
   (which it would do because make.h was found in $srcdir).  */
#include <config.h>
#undef	HAVE_CONFIG_H
#define HAVE_CONFIG_H

#ifdef	CRAY
/* This must happen before #include <signal.h> so
   that the declaration therein is changed.  */
#define	signal	bsdsignal
#endif

#define _GNU_SOURCE
#include <sys/types.h>
#include <sys/stat.h>
#include <signal.h>
#include <stdio.h>
#include <ctype.h>
#ifdef HAVE_SYS_TIMEB_H
/* SCO 3.2 "devsys 4.2" has a prototype for `ftime' in <time.h> that bombs
   unless <sys/timeb.h> has been included first.  Does every system have a
   <sys/timeb.h>?  If any does not, configure should check for it.  */
#include <sys/timeb.h>
#endif
#include <time.h>
#include <errno.h>

#ifndef	errno
extern int errno;
#endif

#ifndef	isblank
#define	isblank(c)	((c) == ' ' || (c) == '\t')
#endif

#ifdef	HAVE_UNISTD_H
#include <unistd.h>
#ifdef	_POSIX_VERSION
#define	POSIX
#endif
#endif

/* Some systems define _POSIX_VERSION but are not really POSIX.1.  */
#if (defined (butterfly) || \
     (defined (__mips) && defined (_SYSTYPE_SVR3)) || \
     (defined (sequent) && defined (i386)))
#undef POSIX
#endif

#if !defined (POSIX) && defined (_AIX) && defined (_POSIX_SOURCE)
#define POSIX
#endif

#if !defined (HAVE_SYS_SIGLIST) && defined (HAVE__SYS_SIGLIST)
#define	sys_siglist	_sys_siglist
#define	HAVE_SYS_SIGLIST	/* Now we have it.  */

/* It was declared in <signal.h>, with who knows what type.
   Don't declare it again and risk conflicting.  */
#define	SYS_SIGLIST_DECLARED
#endif

#ifdef HAVE_SYS_SIGLIST
#ifndef SYS_SIGLIST_DECLARED
extern char *sys_siglist[];
#endif
#else
#include "signame.h"
#endif

/* Some systems do not define NSIG in <signal.h>.  */
#ifndef	NSIG
#ifdef	_NSIG
#define	NSIG	_NSIG
#else
#define	NSIG	32
#endif
#endif

#ifndef	RETSIGTYPE
#define	RETSIGTYPE	void
#endif

#ifndef	sigmask
#define	sigmask(sig)	(1 << ((sig) - 1))
#endif

#ifdef	HAVE_LIMITS_H
#include <limits.h>
#endif
#ifdef	HAVE_SYS_PARAM_H
#include <sys/param.h>
#endif

#ifndef	PATH_MAX
#ifndef	POSIX
#define	PATH_MAX	MAXPATHLEN
#endif	/* Not POSIX.  */
#endif	/* No PATH_MAX.  */
#ifndef MAXPATHLEN
#define MAXPATHLEN 1024
#endif	/* No MAXPATHLEN.  */

#ifdef	PATH_MAX
#define	GET_PATH_MAX	PATH_MAX
#define	PATH_VAR(var)	char var[PATH_MAX]
#else
#define	NEED_GET_PATH_MAX
extern unsigned int get_path_max ();
#define	GET_PATH_MAX	(get_path_max ())
#define	PATH_VAR(var)	char *var = (char *) alloca (GET_PATH_MAX)
#endif

#ifdef	STAT_MACROS_BROKEN
#ifdef	S_ISREG
#undef	S_ISREG
#endif
#ifdef	S_ISDIR
#undef	S_ISDIR
#endif
#endif	/* STAT_MACROS_BROKEN.  */

#ifndef	S_ISREG
#define	S_ISREG(mode)	(((mode) & S_IFMT) == S_IFREG)
#endif
#ifndef	S_ISDIR
#define	S_ISDIR(mode)	(((mode) & S_IFMT) == S_IFDIR)
#endif


#if	(defined (STDC_HEADERS) || defined (__GNU_LIBRARY__))
#include <stdlib.h>
#include <string.h>
#define	ANSI_STRING
#else	/* No standard headers.  */

#ifdef HAVE_STRING_H
#include <string.h>
#define	ANSI_STRING
#else
#include <strings.h>
#endif
#ifdef	HAVE_MEMORY_H
#include <memory.h>
#endif

extern char *malloc (), *realloc ();
extern void free ();

extern void qsort ();
extern void abort (), exit ();

#endif	/* Standard headers.  */

#ifdef	ANSI_STRING

#ifndef	index
#define	index(s, c)	strchr((s), (c))
#endif
#ifndef	rindex
#define	rindex(s, c)	strrchr((s), (c))
#endif

#ifndef	bcmp
#define bcmp(s1, s2, n)	memcmp ((s1), (s2), (n))
#endif
#ifndef	bzero
#define bzero(s, n)	memset ((s), 0, (n))
#endif
#ifndef	bcopy
#define bcopy(s, d, n)	memcpy ((d), (s), (n))
#endif

#else	/* Not ANSI_STRING.  */

#ifndef	bcmp
extern int bcmp ();
#endif
#ifndef	bzero
extern void bzero ();
#endif
#ifndef	bcopy
extern void bcopy ();
#endif

#endif	/* ANSI_STRING.  */
#undef	ANSI_STRING


#ifdef	__GNUC__
#undef	alloca
#define	alloca(n)	__builtin_alloca (n)
#else	/* Not GCC.  */
#ifdef	HAVE_ALLOCA_H
#include <alloca.h>
#else	/* Not HAVE_ALLOCA_H.  */
#ifndef	_AIX
extern char *alloca ();
#endif	/* Not AIX.  */
#endif	/* HAVE_ALLOCA_H.  */
#endif	/* GCC.  */

#ifndef	iAPX286
#define streq(a, b) \
  ((a) == (b) || \
   (*(a) == *(b) && (*(a) == '\0' || !strcmp ((a) + 1, (b) + 1))))
#else
/* Buggy compiler can't handle this.  */
#define streq(a, b) (strcmp ((a), (b)) == 0)
#endif

/* Add to VAR the hashing value of C, one character in a name.  */
#define	HASH(var, c) \
  ((var += (c)), (var = ((var) << 7) + ((var) >> 20)))

#if defined(__GNUC__) || defined(ENUM_BITFIELDS)
#define	ENUM_BITFIELD(bits)	:bits
#else
#define	ENUM_BITFIELD(bits)
#endif

extern void die ();
extern void message (), fatal (), error ();
extern void makefile_error (), makefile_fatal ();
extern void pfatal_with_name (), perror_with_name ();
extern char *savestring (), *concat ();
extern char *xmalloc (), *xrealloc ();
extern char *find_next_token (), *next_token (), *end_of_token ();
extern void collapse_continuations (), remove_comments ();
extern char *sindex (), *lindex ();
extern int alpha_compare ();
extern void print_spaces ();
extern struct dep *copy_dep_chain ();
extern char *find_percent ();

#ifndef	NO_ARCHIVES
extern int ar_name ();
extern void ar_parse_name ();
extern int ar_touch ();
extern time_t ar_member_date ();
#endif

extern void dir_load ();
extern int dir_file_exists_p (), file_exists_p (), file_impossible_p ();
extern void file_impossible ();
extern char *dir_name ();

extern void define_default_variables ();
extern void set_default_suffixes (), install_default_suffix_rules ();
extern void install_default_implicit_rules (), count_implicit_rule_limits ();
extern void convert_to_pattern (), create_pattern_rule ();

extern void build_vpath_lists (), construct_vpath_list ();
extern int vpath_search ();

extern void construct_include_path ();
extern void uniquize_deps ();

extern int update_goal_chain ();
extern void notice_finished_file ();

extern void user_access (), make_access (), child_access ();


#ifdef	HAVE_VFORK_H
#include <vfork.h>
#endif

#if !defined (__GNU_LIBRARY__) && !defined (POSIX)

#ifdef	HAVE_SIGSETMASK
extern int sigsetmask ();
extern int sigblock ();
#endif
extern int kill ();
extern int atoi ();
extern long int atol ();
extern int unlink (), stat (), fstat ();
extern int pipe (), close (), read (), write (), open ();
extern long int lseek ();

#endif	/* Not GNU C library or POSIX.  */

#ifdef	HAVE_GETCWD
extern char *getcwd ();
#else
extern char *getwd ();
#define	getcwd(buf, len)	getwd (buf)
#endif

extern char **environ;

extern char *reading_filename;
extern unsigned int *reading_lineno_ptr;

extern int just_print_flag, silent_flag, ignore_errors_flag, keep_going_flag;
extern int debug_flag, print_data_base_flag, question_flag, touch_flag;
extern int env_overrides, no_builtin_rules_flag, print_version_flag;
extern int print_directory_flag, warn_undefined_variables_flag;

extern unsigned int job_slots;
extern double max_load_average;

extern char *program;
extern char *starting_directory;
extern unsigned int makelevel;
extern char *version_string, *remote_description;

extern unsigned int commands_started;

extern int handling_fatal_signal;


#define DEBUGPR(msg) \
  do if (debug_flag) { print_spaces (depth); printf (msg, file->name); \
		       fflush (stdout); } while (0)
