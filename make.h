/* Copyright (C) 1988, 1989, 1990, 1991, 1992 Free Software Foundation, Inc.
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

#define _GNU_SOURCE
#include <sys/types.h>
#include <sys/stat.h>
#include <signal.h>
#include <stdio.h>
#include <ctype.h>
#include <time.h>

#ifdef	HAVE_UNISTD_H
#include <unistd.h>
#ifdef	_POSIX_VERSION
#define	POSIX
#endif
#endif

#ifndef	isblank
#define	isblank(c)	((c) == ' ' || (c) == '\t')
#endif

#if	!defined(NSIG) && defined(_NSIG)
#define	NSIG	_NSIG
#endif

#ifndef	RETSIGTYPE
#define	RETSIGTYPE	void
#endif

#ifdef	CRAY
#define	signal	bsdsignal
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
#ifdef	POSIX
#define	GET_PATH_MAX	pathconf ("/", _PC_PATH_MAX)
#else	/* Not POSIX.  */
#ifndef MAXPATHLEN
#define MAXPATHLEN 1024
#endif	/* No MAXPATHLEN.  */
#define	PATH_MAX	MAXPATHLEN
#endif	/* POSIX.  */
#endif	/* No PATH_MAX.  */

#ifdef	PATH_MAX
#define	PATH_VAR(var)	char var[PATH_MAX]
#define	GET_PATH_MAX	PATH_MAX
#else
#define	PATH_VAR(var)	char *var = (char *) alloca (GET_PATH_MAX)
#endif

#ifdef	uts
#ifdef	S_ISREG
#undef	S_ISREG
#endif
#ifdef	S_ISDIR
#undef	S_ISDIR
#endif
#endif	/* uts.  */

#ifndef	S_ISREG
#define	S_ISREG(mode)	(((mode) & S_IFMT) == S_IFREG)
#endif
#ifndef	S_ISDIR
#define	S_ISDIR(mode)	(((mode) & S_IFMT) == S_IFDIR)
#endif


#if	(defined (STDC_HEADERS) || defined (__GNU_LIBRARY__) \
	 || defined (POSIX))
#include <stdlib.h>
#include <string.h>
#define	ANSI_STRING
#else	/* No standard headers.  */

#ifdef	USG

#include <string.h>
#ifdef	NEED_MEMORY_H
#include <memory.h>
#endif
#define	ANSI_STRING

#else	/* Not USG.  */

#ifdef	NeXT

#include <string.h>

#else	/* Not NeXT.  */

#include <strings.h>

#ifndef	bcmp
extern int bcmp ();
#endif
#ifndef	bzero
extern void bzero ();
#endif
#ifndef	bcopy
extern void bcopy ();
#endif

#endif	/* NeXT. */

#endif	/* USG.  */

extern char *malloc (), *realloc ();
extern void free ();

#endif	/* Standard headers.  */

#ifdef	ANSI_STRING
#define	index(s, c)	strchr((s), (c))
#define	rindex(s, c)	strrchr((s), (c))

#define bcmp(s1, s2, n)	memcmp ((s1), (s2), (n))
#define bzero(s, n)	memset ((s), 0, (n))
#define bcopy(s, d, n)	memcpy ((d), (s), (n))
#endif
#undef	ANSI_STRING


#ifdef	__GNUC__
#undef	alloca
#define	alloca(n)	__builtin_alloca (n)
#else	/* Not GCC.  */
#if	defined (sparc) || defined (HAVE_ALLOCA_H)
#include <alloca.h>
#else	/* Not sparc or HAVE_ALLOCA_H.  */
extern char *alloca ();
#endif	/* sparc or HAVE_ALLOCA_H.  */
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
extern void set_default_suffixes (), install_default_implicit_rules ();
extern void convert_to_pattern (), count_implicit_rule_limits ();
extern void create_pattern_rule ();

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

#if	defined(__GNU_LIBRARY__) || defined(POSIX)

#include <unistd.h>

#else

#ifndef	USG
extern int sigsetmask ();
extern int sigblock ();
#endif
extern int kill ();
extern void abort (), exit ();
extern int unlink (), stat (), fstat ();
extern void qsort ();
extern int atoi ();
extern long int atol ();
extern int pipe (), close (), read (), write (), open ();
extern long int lseek ();
#endif	/* GNU C library or POSIX.  */

#ifndef	GETCWD_MISSING
extern char *getcwd ();
#else
extern char *getwd ();
#define	getcwd (buf, len)	getwd (buf)
#endif

#if !defined(__GNU_LIBRARY__) && (!defined(vfork) || !defined(POSIX))
#ifdef	POSIX
extern pid_t vfork ();
#else
extern int vfork ();
#endif
#endif

extern char **environ;

extern char *reading_filename;
extern unsigned int *reading_lineno_ptr;

extern int just_print_flag, silent_flag, ignore_errors_flag, keep_going_flag;
extern int debug_flag, print_data_base_flag, question_flag, touch_flag;
extern int env_overrides, no_builtin_rules_flag, print_version_flag;
extern int print_directory_flag;

extern unsigned int job_slots;
extern double max_load_average;

extern char *program;

extern unsigned int makelevel;


#define DEBUGPR(msg) \
  do if (debug_flag) { print_spaces (depth); printf (msg, file->name); \
		       fflush (stdout); } while (0)

#ifdef	__GNUC__
#define	max(a, b) \
  ({ register int __a = (a), __b = (b); __a > __b ? __a : __b; })
#else
#define max(a, b) ((a) > (b) ? (a) : (b))
#endif
