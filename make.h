/* Miscellaneous global declarations and portability cruft for GNU Make.
Copyright (C) 1988,89,90,91,92,93,94,95,96,97 Free Software Foundation, Inc.
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


/* Use prototypes if available.  */
#if defined (__cplusplus) || (defined (__STDC__) && __STDC__)
#undef  PARAMS
#define PARAMS(protos)  protos
#else /* Not C++ or ANSI C.  */
#undef  PARAMS
#define PARAMS(protos)  ()
#endif /* C++ or ANSI C.  */


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
/* Ultrix's unistd.h always defines _POSIX_VERSION, but you only get
   POSIX.1 behavior with `cc -YPOSIX', which predefines POSIX itself!  */
#if defined (_POSIX_VERSION) && !defined (ultrix) && !defined (VMS)
#define	POSIX
#endif
#endif

/* Some systems define _POSIX_VERSION but are not really POSIX.1.  */
#if (defined (butterfly) || defined (__arm) || (defined (__mips) && defined (_SYSTYPE_SVR3)) || (defined (sequent) && defined (i386)))
#undef POSIX
#endif

#if !defined (POSIX) && defined (_AIX) && defined (_POSIX_SOURCE)
#define POSIX
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
extern unsigned int get_path_max PARAMS ((void));
#define	GET_PATH_MAX	(get_path_max ())
#define	PATH_VAR(var)	char *var = (char *) alloca (GET_PATH_MAX)
#endif

#ifndef CHAR_BIT
#define CHAR_BIT 8
#endif

/* Nonzero if the integer type T is signed.  */
#define INTEGER_TYPE_SIGNED(t) ((t) -1 < 0)

/* The minimum and maximum values for the integer type T.
   Use ~ (t) 0, not -1, for portability to 1's complement hosts.  */
#define INTEGER_TYPE_MINIMUM(t) \
  (! INTEGER_TYPE_SIGNED (t) ? (t) 0 : ~ (t) 0 << (sizeof (t) * CHAR_BIT - 1))
#define INTEGER_TYPE_MAXIMUM(t) (~ (t) 0 - INTEGER_TYPE_MINIMUM (t))

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

#ifdef VMS
#include <stdio.h>
#include <types.h>
#include <unixlib.h>
#include <unixio.h>
#include <errno.h>
#include <perror.h>
#endif

#if	(defined (STDC_HEADERS) || defined (__GNU_LIBRARY__) || defined(VMS))
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

extern char *malloc PARAMS ((int));
extern char *realloc PARAMS ((char *, int));
extern void free PARAMS ((char *));

extern void abort PARAMS ((void));
extern void exit PARAMS ((int));

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
#if defined(HAVE_MEMMOVE) && !defined(bcopy)
#define bcopy(s, d, n)	memmove ((d), (s), (n))
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

/* SCO Xenix has a buggy macro definition in <string.h>.  */
#undef	strerror

#if !defined(ANSI_STRING) && !defined(__DECC)
extern char *strerror PARAMS ((int errnum));
#endif


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
#ifdef _AMIGA
#define strieq(a, b) \
  ((a) == (b) || \
   (tolower(*(a)) == tolower(*(b)) && (*(a) == '\0' || !strcmpi ((a) + 1, (b) + 1))))
#else
#define strieq(a, b) \
  ((a) == (b) || \
   (*(a) == *(b) && (*(a) == '\0' || !strcmp ((a) + 1, (b) + 1))))
#endif
#else
/* Buggy compiler can't handle this.  */
#define streq(a, b) (strcmp ((a), (b)) == 0)
#define strieq(a, b) (strcmp ((a), (b)) == 0)
#endif

/* Add to VAR the hashing value of C, one character in a name.  */
#define	HASH(var, c) \
  ((var += (c)), (var = ((var) << 7) + ((var) >> 20)))
#ifdef _AMIGA /* Fold filenames on #amiga */
#define HASHI(var, c) \
  ((var += tolower((c))), (var = ((var) << 7) + ((var) >> 20)))
#else
#define HASHI(var, c) HASH(var,c)
#endif

#if defined(__GNUC__) || defined(ENUM_BITFIELDS)
#define	ENUM_BITFIELD(bits)	:bits
#else
#define	ENUM_BITFIELD(bits)
#endif

#if defined(__MSDOS__) || defined(WINDOWS32)
#define PATH_SEPARATOR_CHAR ';'
#else
#if defined(VMS)
#define PATH_SEPARATOR_CHAR ','
#else
#define PATH_SEPARATOR_CHAR ':'
#endif
#endif

#ifdef WINDOWS32
#include <fcntl.h>
#include <malloc.h>
#define pipe(p) _pipe(p, 512, O_BINARY)
#define kill(pid,sig) w32_kill(pid,sig)

extern void sync_Path_environment(void);
extern int kill(int pid, int sig);
extern int safe_stat(char *file, struct stat *sb);
extern char *end_of_token_w32();
#endif

extern void die ();
extern void message ();
extern void fatal ();
extern void error ();
extern void log_working_directory ();
extern void makefile_error ();
extern void makefile_fatal ();
extern void pfatal_with_name ();
extern void perror_with_name ();
extern char *savestring ();
extern char *concat ();
extern char *xmalloc ();
extern char *xrealloc ();
extern char *find_next_token ();
extern char *next_token ();
extern char *end_of_token ();
extern void collapse_continuations ();
extern void remove_comments ();
extern char *sindex ();
extern char *lindex ();
extern int alpha_compare ();
extern void print_spaces ();
extern struct dep *copy_dep_chain ();
extern char *find_char_unquote ();
extern char *find_percent ();

#ifndef	NO_ARCHIVES
extern int ar_name ();
extern void ar_parse_name ();
extern int ar_touch ();
extern time_t ar_member_date ();
#endif

extern void dir_load ();
extern int dir_file_exists_p ();
extern int file_exists_p ();
extern int file_impossible_p ();
extern void file_impossible ();
extern char *dir_name ();

extern void define_default_variables ();
extern void set_default_suffixes ();
extern void install_default_suffix_rules ();
extern void install_default_implicit_rules ();
extern void count_implicit_rule_limits ();
extern void convert_to_pattern ();
extern void create_pattern_rule ();

extern void build_vpath_lists ();
extern void construct_vpath_list ();
extern int vpath_search ();
extern int gpath_search ();

extern void construct_include_path ();
extern void uniquize_deps ();

extern int update_goal_chain ();
extern void notice_finished_file ();

extern void user_access ();
extern void make_access ();
extern void child_access ();

#ifdef	HAVE_VFORK_H
#include <vfork.h>
#endif

/* We omit these declarations on non-POSIX systems which define _POSIX_VERSION,
   because such systems often declare them in header files anyway.  */

#if !defined (__GNU_LIBRARY__) && !defined (POSIX) && !defined (_POSIX_VERSION) && !defined(WINDOWS32)

extern long int atol ();
#ifndef VMS
extern long int lseek ();
#endif

#endif	/* Not GNU C library or POSIX.  */

#ifdef	HAVE_GETCWD
extern char *getcwd ();
#ifdef VMS
extern char *getwd PARAMS ((char *));
#endif
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
extern int posix_pedantic;
extern int clock_skew_detected;

extern unsigned int job_slots;
#ifndef NO_FLOAT
extern double max_load_average;
#else
extern int max_load_average;
#endif

extern char *program;
extern char *starting_directory;
extern unsigned int makelevel;
extern char *version_string, *remote_description;

extern unsigned int commands_started;

extern int handling_fatal_signal;


#define DEBUGPR(msg) \
  do if (debug_flag) { print_spaces (depth); printf (msg, file->name); \
		       fflush (stdout); } while (0)

#ifdef VMS
# ifndef EXIT_FAILURE
#  define EXIT_FAILURE 3
# endif
# ifndef EXIT_SUCCESS
#  define EXIT_SUCCESS 1
# endif
# ifndef EXIT_TROUBLE
#  define EXIT_TROUBLE 2
# endif
#else
# ifndef EXIT_FAILURE
#  define EXIT_FAILURE 2
# endif
# ifndef EXIT_SUCCESS
#  define EXIT_SUCCESS 0
# endif
# ifndef EXIT_TROUBLE
#  define EXIT_TROUBLE 1
# endif
#endif

