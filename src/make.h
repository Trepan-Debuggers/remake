/* Miscellaneous global declarations and portability cruft for GNU Make.
Copyright (C) 1988, 1989, 1990, 1991, 1992, 1993, 1994, 1995, 1996, 1997,
1998, 1999, 2000, 2001, 2002, 2003, 2004, 2005, 2006, 2007, 2008, 2009,
2010 Free Software Foundation, Inc.
This file is part of GNU Make.

GNU Make is free software; you can redistribute it and/or modify it under the
terms of the GNU General Public License as published by the Free Software
Foundation; either version 3 of the License, or (at your option) any later
version.

GNU Make is distributed in the hope that it will be useful, but WITHOUT ANY
WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR
A PARTICULAR PURPOSE.  See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License along with
this program.  If not, see <http://www.gnu.org/licenses/>.  */

/* We use <config.h> instead of "config.h" so that a compilation
   using -I. -I$srcdir will use ./config.h rather than $srcdir/config.h
   (which it would do because make.h was found in $srcdir).  */
#ifndef MAKE_H
#define MAKE_H

#include <config.h>
#undef  HAVE_CONFIG_H
#define HAVE_CONFIG_H 1

/* Specify we want GNU source code.  This must be defined before any
   system headers are included.  */

#define _GNU_SOURCE 1

/* AIX requires this to be the first thing in the file.  */
#if HAVE_ALLOCA_H
# include <alloca.h>
#else
# ifdef _AIX
 #pragma alloca
# else
#  if !defined(__GNUC__) && !defined(WINDOWS32)
#   ifndef alloca /* predefined by HP cc +Olibcalls */
char *alloca ();
#   endif
#  endif
# endif
#endif


#ifdef  CRAY
/* This must happen before #include <signal.h> so
   that the declaration therein is changed.  */
# define signal bsdsignal
#endif

/* If we're compiling for the dmalloc debugger, turn off string inlining.  */
#if defined(HAVE_DMALLOC_H) && defined(__GNUC__)
# define __NO_STRING_INLINES
#endif

#include <sys/types.h>
#include <sys/stat.h>
#include <signal.h>
#include <stdio.h>
#include <ctype.h>
#ifdef HAVE_SYS_TIMEB_H
/* SCO 3.2 "devsys 4.2" has a prototype for `ftime' in <time.h> that bombs
   unless <sys/timeb.h> has been included first.  Does every system have a
   <sys/timeb.h>?  If any does not, configure should check for it.  */
# include <sys/timeb.h>
#endif

#if TIME_WITH_SYS_TIME
# include <sys/time.h>
# include <time.h>
#else
# if HAVE_SYS_TIME_H
#  include <sys/time.h>
# else
#  include <time.h>
# endif
#endif

#include <errno.h>

#ifndef errno
extern int errno;
#endif

#ifndef isblank
# define isblank(c)     ((c) == ' ' || (c) == '\t')
#endif

#ifdef  HAVE_UNISTD_H
# include <unistd.h>
/* Ultrix's unistd.h always defines _POSIX_VERSION, but you only get
   POSIX.1 behavior with `cc -YPOSIX', which predefines POSIX itself!  */
# if defined (_POSIX_VERSION) && !defined (ultrix) && !defined (VMS)
#  define POSIX 1
# endif
#endif

/* Some systems define _POSIX_VERSION but are not really POSIX.1.  */
#if (defined (butterfly) || defined (__arm) || (defined (__mips) && defined (_SYSTYPE_SVR3)) || (defined (sequent) && defined (i386)))
# undef POSIX
#endif

#if !defined (POSIX) && defined (_AIX) && defined (_POSIX_SOURCE)
# define POSIX 1
#endif

#ifndef RETSIGTYPE
# define RETSIGTYPE     void
#endif

#ifndef sigmask
# define sigmask(sig)   (1 << ((sig) - 1))
#endif

#ifndef HAVE_SA_RESTART
# define SA_RESTART 0
#endif

#ifdef  HAVE_LIMITS_H
# include <limits.h>
#endif
#ifdef  HAVE_SYS_PARAM_H
# include <sys/param.h>
#endif

#ifndef PATH_MAX
# ifndef POSIX
#  define PATH_MAX      MAXPATHLEN
# endif
#endif
#ifndef MAXPATHLEN
# define MAXPATHLEN 1024
#endif

#ifdef  PATH_MAX
# define GET_PATH_MAX   PATH_MAX
//# define PATH_VAR(var)  char var[PATH_MAX]
#else
# define NEED_GET_PATH_MAX 1
# define GET_PATH_MAX   (get_path_max ())
// # define PATH_VAR(var)  char *var = alloca (GET_PATH_MAX)
unsigned int get_path_max (void);
#endif

#ifndef CHAR_BIT
# define CHAR_BIT 8
#endif

/* The minimum and maximum values for the integer type T.
   Use ~ (t) 0, not -1, for portability to 1's complement hosts.  */
#define INTEGER_TYPE_MINIMUM(t) \
  (! INTEGER_TYPE_SIGNED (t) ? (t) 0 : ~ (t) 0 << (sizeof (t) * CHAR_BIT - 1))
#define INTEGER_TYPE_MAXIMUM(t) (~ (t) 0 - INTEGER_TYPE_MINIMUM (t))

#ifndef CHAR_MAX
# define CHAR_MAX INTEGER_TYPE_MAXIMUM (char)
#endif

#ifdef STAT_MACROS_BROKEN
# ifdef S_ISREG
#  undef S_ISREG
# endif
# ifdef S_ISDIR
#  undef S_ISDIR
# endif
#endif  /* STAT_MACROS_BROKEN.  */

#ifndef S_ISREG
# define S_ISREG(mode)  (((mode) & S_IFMT) == S_IFREG)
#endif
#ifndef S_ISDIR
# define S_ISDIR(mode)  (((mode) & S_IFMT) == S_IFDIR)
#endif

#ifndef __attribute__
/* This feature is available in gcc versions 2.5 and later.  */
# if __GNUC__ < 2 || (__GNUC__ == 2 && __GNUC_MINOR__ < 5) || __STRICT_ANSI__
#  define __attribute__(x)
# endif
/* The __-protected variants of `format' and `printf' attributes
   are accepted by gcc versions 2.6.4 (effectively 2.7) and later.  */
# if __GNUC__ < 2 || (__GNUC__ == 2 && __GNUC_MINOR__ < 7)
#  define __format__ format
#  define __printf__ printf
# endif
#endif
// #define UNUSED  __attribute__ ((unused))

#if defined (STDC_HEADERS) || defined (__GNU_LIBRARY__)
# include <stdlib.h>
# include <string.h>
# define ANSI_STRING 1
#else   /* No standard headers.  */
# ifdef HAVE_STRING_H
#  include <string.h>
#  define ANSI_STRING 1
# else
#  include <strings.h>
# endif
# ifdef HAVE_MEMORY_H
#  include <memory.h>
# endif
# ifdef HAVE_STDLIB_H
#  include <stdlib.h>
# else
void *malloc (int);
void *realloc (void *, int);
void free (void *);

void abort (void) __attribute__ ((noreturn));
void exit (int) __attribute__ ((noreturn));
# endif /* HAVE_STDLIB_H.  */

#endif /* Standard headers.  */

/* These should be in stdlib.h.  Make sure we have them.  */
#ifndef EXIT_SUCCESS
# define EXIT_SUCCESS 0
#endif
#ifndef EXIT_FAILURE
# define EXIT_FAILURE 1
#endif

#ifndef  ANSI_STRING

/* SCO Xenix has a buggy macro definition in <string.h>.  */
#undef  strerror
#if !defined(__DECC)
char *strerror (int errnum);
#endif

#endif  /* !ANSI_STRING.  */
#undef  ANSI_STRING

#if HAVE_INTTYPES_H
# include <inttypes.h>
#endif
#define FILE_TIMESTAMP uintmax_t

#if !defined(HAVE_STRSIGNAL)
char *strsignal (int signum);
#endif

/* Test if two strings are equal. Is this worthwhile?  Should be profiled.  */
#define streq(a, b) \
   ((a) == (b) || \
    (*(a) == *(b) && (*(a) == '\0' || !strcmp ((a) + 1, (b) + 1))))

/* Test if two strings are equal, but match case-insensitively on systems
   which have case-insensitive filesystems.  Should only be used for
   filenames!  */
#ifdef HAVE_CASE_INSENSITIVE_FS
# define patheq(a, b) \
    ((a) == (b) \
     || (tolower((unsigned char)*(a)) == tolower((unsigned char)*(b)) \
         && (*(a) == '\0' || !strcasecmp ((a) + 1, (b) + 1))))
#else
# define patheq(a, b) streq(a, b)
#endif

#define strneq(a, b, l) (strncmp ((a), (b), (l)) == 0)

#if defined(__GNUC__) || defined(ENUM_BITFIELDS)
# define ENUM_BITFIELD(bits)    :bits
#else
# define ENUM_BITFIELD(bits)
#endif

/* Handle gettext and locales.  */

#if HAVE_LOCALE_H
# include <locale.h>
#else
# define setlocale(category, locale)
#endif

#include <gettext.h>

#define _(msgid)            gettext (msgid)
#define N_(msgid)           gettext_noop (msgid)
#define S_(msg1,msg2,num)   ngettext (msg1,msg2,num)

/* Handle other OSs.  */
#ifndef PATH_SEPARATOR_CHAR
# if defined(HAVE_DOS_PATHS)
#  define PATH_SEPARATOR_CHAR ';'
# elif defined(VMS)
#  define PATH_SEPARATOR_CHAR ','
# else
#  define PATH_SEPARATOR_CHAR ':'
# endif
#endif

/* This is needed for getcwd() and chdir(), on some W32 systems.  */
#if defined(HAVE_DIRECT_H)
# include <direct.h>
#endif

#ifdef WINDOWS32
# include <fcntl.h>
# include <malloc.h>
# define pipe(_p)        _pipe((_p), 512, O_BINARY)
# define kill(_pid,_sig) w32_kill((_pid),(_sig))

void sync_Path_environment (void);
int w32_kill (pid_t pid, int sig);
char *end_of_token_w32 (const char *s, char stopchar);
int find_and_set_default_shell (const char *token);

/* indicates whether or not we have Bourne shell */
extern int no_default_sh_exe;

/* is default_shell unixy? */
extern int unixy_shell;
#endif  /* WINDOWS32 */

#if defined(HAVE_SYS_RESOURCE_H) && defined(HAVE_GETRLIMIT) && defined(HAVE_SETRLIMIT)
# define SET_STACK_SIZE
#endif
#ifdef SET_STACK_SIZE
# include <sys/resource.h>
extern struct rlimit stack_limit;
#endif

/* We have to have stdarg.h or varargs.h AND v*printf or doprnt to use
   variadic versions of these functions.  */

#if HAVE_STDARG_H || HAVE_VARARGS_H
# if HAVE_VPRINTF || HAVE_DOPRNT
#  define USE_VARIADIC 1
# endif
#endif

void die (int);
void pfatal_with_name (const char *);
void perror_with_name (const char *, const char *);
void *xmalloc (size_t);
void *xcalloc (size_t);
void *xrealloc (void *, size_t);
char *xstrdup (const char *);
char *xstrndup (const char *, size_t);
//char *find_next_token (const char **, unsigned int *);
char *next_token (const char *);
char *end_of_token (const char *);
void collapse_continuations (char *);
char *lindex (const char *, const char *, int);
int alpha_compare (const void *, const void *);
void print_spaces (unsigned int);
char *find_percent (char *);
const char *find_percent_cached (const char **);
FILE *open_tmpfile (char **, const char *);

#ifndef NO_ARCHIVES
int ar_name (const char *);
void ar_parse_name (const char *, char **, char **);
int ar_touch (const char *);
time_t ar_member_date (const char *);


#endif

int dir_file_exists_p (const char *, const char *);
int file_exists_p (const char *);
int file_impossible_p (const char *);
void file_impossible (const char *);
const char *dir_name (const char *);
void hash_init_directories (void);

void define_default_variables (void);
void set_default_suffixes (void);
void install_default_suffix_rules (void);
void install_default_implicit_rules (void);

void build_vpath_lists (void);
void construct_vpath_list (char *pattern, char *dirpath);
const char *vpath_search (const char *file, FILE_TIMESTAMP *mtime_ptr,
                          unsigned int* vpath_index, unsigned int* path_index);
int gpath_search (const char *file, size_t len);

/*! Construct the list of include directories
   from the arguments and the default list.
*/
extern void construct_include_path (const char **arg_dirs);

void make_access (void);

char *strip_whitespace (const char **begpp, const char **endpp);

/* String caching  */
void strcache_init (void);
void strcache_print_stats (const char *prefix);
int strcache_iscached (const char *str);
const char *strcache_add (const char *str);

#ifdef  HAVE_VFORK_H
# include <vfork.h>
#endif

/* We omit these declarations on non-POSIX systems which define _POSIX_VERSION,
   because such systems often declare them in header files anyway.  */

#if !defined (__GNU_LIBRARY__) && !defined (POSIX) && !defined (_POSIX_VERSION) && !defined(WINDOWS32)

long int atol ();
# ifndef VMS
long int lseek ();
# endif

#endif  /* Not GNU C library or POSIX.  */

#ifdef  HAVE_GETCWD
# if !defined(VMS) && !defined(__DECC)
char *getcwd ();
# endif
#else
char *getwd ();
# define getcwd(buf, len)       getwd (buf)
#endif

#if !HAVE_STRCASECMP
# if HAVE_STRICMP
#  define strcasecmp stricmp
# elif HAVE_STRCMPI
#  define strcasecmp strcmpi
# else
/* Create our own, in misc.c */
int strcasecmp (const char *s1, const char *s2);
# endif
#endif

#if !HAVE_STRNCASECMP
# if HAVE_STRNICMP
#  define strncasecmp strnicmp
# elif HAVE_STRNCMPI
#  define strncasecmp strncmpi
# else
/* Create our own, in misc.c */
int strncasecmp (const char *s1, const char *s2, int n);
# endif
#endif

extern char **environ;

extern int just_print_flag, silent_flag, ignore_errors_flag, keep_going_flag;
extern int print_data_base_flag, question_flag, touch_flag, always_make_flag;
extern int env_overrides, no_builtin_rules_flag, no_builtin_variables_flag;
extern int print_version_flag, print_directory, check_symlink_flag;
extern int warn_undefined_variables_flag, posix_pedantic, not_parallel;
extern int second_expansion, clock_skew_detected, rebuilding_makefiles;
extern int one_shell;

/* can we run commands via 'sh -c xxx' or must we use batch files? */
extern int batch_mode_shell;

/* Resetting the command script introduction prefix character.  */
#define RECIPEPREFIX_NAME          ".RECIPEPREFIX"
#define RECIPEPREFIX_DEFAULT       '\t'
extern char cmd_prefix;

extern unsigned int job_slots;
#ifndef NO_FLOAT
extern double max_load_average;
#else
extern int max_load_average;
#endif

/*! Value of argv[0] which seems to get modified. Can we merge this with
    program below? */
extern char *argv0;

/*! Our initial arguments -- used for debugger restart execvp.  */
extern const char * const *global_argv;

extern char *starting_directory;

/*! Our current directory before processing any -C options.  */
extern char *directory_before_chdir;

/*! Value of the MAKELEVEL variable at startup (or 0).  */
extern unsigned int makelevel;

/*! Value of the MAKEPARENT variable at startup (or 0). */
extern pid_t makeparent_pid;

/*! Value of the MAKEPARENT_TARGET variable at startup (or NULL). */
extern char *makeparent_target;

extern char *version_string, *remote_description, *make_host;

extern unsigned int commands_started;

extern volatile sig_atomic_t handling_fatal_signal;


#ifndef MIN
#define MIN(_a,_b) ((_a)<(_b)?(_a):(_b))
#endif
#ifndef MAX
#define MAX(_a,_b) ((_a)>(_b)?(_a):(_b))
#endif

/* Set up heap debugging library dmalloc.  */

#ifdef HAVE_DMALLOC_H
#include <dmalloc.h>
#endif

#ifndef initialize_main
# ifdef __EMX__
#  define initialize_main(pargc, pargv) \
                          { _wildcard(pargc, pargv); _response(pargc, pargv); }
# else
#  define initialize_main(pargc, pargv)
# endif
#endif

#ifdef __EMX__
# if !defined chdir
#  define chdir _chdir2
# endif
# if !defined getcwd
#  define getcwd _getcwd2
# endif

/* NO_CHDIR2 causes make not to use _chdir2() and _getcwd2() instead of
   chdir() and getcwd(). This avoids some error messages for the
   make testsuite but restricts the drive letter support. */
# ifdef NO_CHDIR2
#  warning NO_CHDIR2: usage of drive letters restricted
#  undef chdir
#  undef getcwd
# endif
#endif

#ifndef initialize_main
# define initialize_main(pargc, pargv)
#endif


/* Some systems (like Solaris, PTX, etc.) do not support the SA_RESTART flag
   properly according to POSIX.  So, we try to wrap common system calls with
   checks for EINTR.  Note that there are still plenty of system calls that
   can fail with EINTR but this, reportedly, gets the vast majority of
   failure cases.  If you still experience failures you'll need to either get
   a system where SA_RESTART works, or you need to avoid -j.  */

#define EINTRLOOP(_v,_c)   while (((_v)=_c)==-1 && errno==EINTR)

/* While system calls that return integers are pretty consistent about
   returning -1 on failure and setting errno in that case, functions that
   return pointers are not always so well behaved.  Sometimes they return
   NULL for expected behavior: one good example is readdir() which returns
   NULL at the end of the directory--and _doesn't_ reset errno.  So, we have
   to do it ourselves here.  */

#endif /*MAKE_H*/
