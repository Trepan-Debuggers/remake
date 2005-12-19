/* Output or logging functions for GNU Make.  

Copyright (C) 2004, 2005 Free Software Foundation, Inc.  This file is part
of GNU Make.

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

#include "make.h"
#include "commands.h"
#include "dbg_cmd.h"
#include "debug.h"
#include "dep.h"
#include "expand.h"
#include "print.h"
#include "read.h"

/* Think of the below not as an enumeration but as #defines done in a
   way that we'll be able to use the value in a gdb. */
enum debug_print_enums_e debug_print_enums1;

#if HAVE_VPRINTF || HAVE_DOPRNT
# define HAVE_STDVARARGS 1
# if __STDC__
#  include <stdarg.h>
#  define VA_START(args, lastarg) va_start(args, lastarg)
# else
#  include <varargs.h>
#  define VA_START(args, lastarg) va_start(args)
# endif
# if HAVE_VPRINTF
#  define VA_PRINTF(fp, lastarg, args) vfprintf((fp), (lastarg), (args))
# else
#  define VA_PRINTF(fp, lastarg, args) _doprnt((lastarg), (args), (fp))
# endif
# define VA_END(args) va_end(args)
#else
/* # undef HAVE_STDVARARGS */
# define va_alist a1, a2, a3, a4, a5, a6, a7, a8
# define va_dcl char *a1, *a2, *a3, *a4, *a5, *a6, *a7, *a8;
# define VA_START(args, lastarg)
# define VA_PRINTF(fp, lastarg, args) fprintf((fp), (lastarg), va_alist)
# define VA_END(args)
#endif


/* Print a message on stdout.  */

void
#if __STDC__ && HAVE_STDVARARGS
message (int prefix, const char *fmt, ...)
#else
message (int prefix, const char *fmt, va_alist)
#endif
{
#if HAVE_STDVARARGS
  va_list args;
#endif

  log_working_directory (1);

  if (fmt != 0)
    {
      if (prefix)
	{
	  if (makelevel == 0)
	    printf ("%s: ", program);
	  else
	    printf ("%s[%u]: ", program, makelevel);
	}
      VA_START (args, fmt);
      VA_PRINTF (stdout, fmt, args);
      VA_END (args);
      putchar ('\n');
    }

  fflush (stdout);
}

/* Print an error message.  */

void
#if __STDC__ && HAVE_STDVARARGS
error (const floc_t *flocp, const char *fmt, ...)
#else
error (flocp, fmt, va_alist)
     const floc_t *flocp;
     const char *fmt;
     va_dcl
#endif
{
#if HAVE_STDVARARGS
  va_list args;
#endif

  log_working_directory (1);

  if (flocp && flocp->filenm)
    fprintf (stderr, "%s:%lu: ", flocp->filenm, flocp->lineno);
  else if (makelevel == 0)
    fprintf (stderr, "%s: ", program);
  else
    fprintf (stderr, "%s[%u]: ", program, makelevel);

  VA_START(args, fmt);
  VA_PRINTF (stderr, fmt, args);
  VA_END (args);

  putc ('\n', stderr);
  fflush (stderr);
  if (debugger_on_error & DEBUGGER_ON_ERROR) 
    enter_debugger(NULL, NULL, -1);
}

void
#if __STDC__ && HAVE_STDVARARGS
err (target_stack_node_t *p_call, const char *fmt, ...)
#else
err (p_call, fmt, va_alist)
     target_stack_node_t *p_call;
     const char *fmt;
     va_dcl
#endif
{
#if HAVE_STDVARARGS
  va_list args;
#endif
  floc_t *p_floc   = NULL;
  file_t *p_target = NULL;
   
  log_working_directory (1);

  if (p_call && p_call->p_target) {
    p_target = p_call->p_target;
    p_floc   = &(p_target->floc);
  }
  
  if (p_floc && p_floc->filenm)
    fprintf (stderr, "%s:%lu: ", p_floc->filenm, p_floc->lineno);
  else if (makelevel == 0)
    fprintf (stderr, "%s: ", program);
  else
    fprintf (stderr, "%s[%u]: ", program, makelevel);

  VA_START(args, fmt);
  VA_PRINTF (stderr, fmt, args);
  VA_END (args);

  putc ('\n', stderr);
  if (!no_extended_errors) {
    if (p_call) 
      print_target_stack(p_call, -1, MAX_STACK_SHOW);
    else if (p_stack_floc_top)
      print_floc_stack(-1, MAX_STACK_SHOW);
  }
  fflush (stderr);
  if (debugger_on_error & DEBUGGER_ON_ERROR) 
    enter_debugger(p_call, p_target, -1);
}

/* Print an error message and exit.  */

void
#if __STDC__ && HAVE_STDVARARGS
fatal (const floc_t *flocp, const char *fmt, ...)
#else
fatal (flocp, fmt, va_alist)
     const floc_t *flocp;
     const char *fmt;
     va_dcl
#endif
{
#if HAVE_STDVARARGS
  va_list args;
#endif

  log_working_directory (1);

  if (flocp && flocp->filenm)
    fprintf (stderr, "%s:%lu: *** ", flocp->filenm, flocp->lineno);
  else if (makelevel == 0)
    fprintf (stderr, "%s: *** ", program);
  else
    fprintf (stderr, "%s[%u]: *** ", program, makelevel);

  VA_START(args, fmt);
  VA_PRINTF (stderr, fmt, args);
  VA_END (args);

  fputs (_(".  Stop.\n"), stderr);

  /* If in_debugger == 1 and we don't die or enter a the debugger again.
     if in_debugger == 77 then we just propagate that wish.
     
   */
  switch (in_debugger) {
  case 0:
    if ( (debugger_on_error & DEBUGGER_ON_FATAL) || debugger_enabled )
      enter_debugger(NULL, NULL, 2);
    die (2);
    break;
  case DEBUGGER_QUIT_RC:
    die(DEBUGGER_QUIT_RC);
  default:
  case 1: ;
    break;
    
  }
}

/* Print an error message and exit.  */

void
#if __STDC__ && HAVE_STDVARARGS
fatal_err(target_stack_node_t *p_call, const char *fmt, ...)
#else
fatal_err (flocp, fmt, va_alist)
     target_stack_node_t *p_call;
     const char *fmt;
     va_dcl
#endif
{
#if HAVE_STDVARARGS
  va_list args;
#endif
  floc_t *p_floc   = NULL;
  file_t *p_target = NULL;

  log_working_directory (1);

  if (p_call && p_call->p_target) {
    p_target = p_call->p_target;
    p_floc   = &(p_target->floc);
  }
  
  if (p_floc && p_floc->filenm)
    fprintf (stderr, "%s:%lu: *** ", p_floc->filenm, p_floc->lineno);
  else if (makelevel == 0)
    fprintf (stderr, "%s: *** ", program);
  else
    fprintf (stderr, "%s[%u]: *** ", program, makelevel);

  VA_START(args, fmt);
  VA_PRINTF (stderr, fmt, args);
  VA_END (args);

  fputs (_(".  Stop.\n"), stderr);
  if (!no_extended_errors) {
    if (p_call) 
      print_target_stack(p_call, -1, MAX_STACK_SHOW);
    else if (p_stack_floc_top)
      print_floc_stack(-1, MAX_STACK_SHOW);
  }
  if ( (debugger_on_error & DEBUGGER_ON_FATAL) || debugger_enabled )
    enter_debugger(p_call, p_target, 2);
  die (2);
}

#ifndef HAVE_STRERROR

#undef	strerror

char *
strerror (int errnum)
{
  extern int errno, sys_nerr;
#ifndef __DECC
  extern char *sys_errlist[];
#endif
  static char buf[] = "Unknown error 12345678901234567890";

  if (errno < sys_nerr)
    return sys_errlist[errnum];

  sprintf (buf, _("Unknown error %d"), errnum);
  return buf;
}
#endif

/*! Print an error message from errno.  */
void
perror_with_name (const char *str, const char *name)
{
  error (NILF, _("%s%s: %s"), str, name, strerror (errno));
  if (debugger_on_error & DEBUGGER_ON_ERROR) 
    enter_debugger(NULL, NULL, -1);
}

/*! Print an error message from errno and exit.  */

void
pfatal_with_name (const char *psz_name)
{
  fatal (NILF, _("%s: %s"), psz_name, strerror (errno));
}

/*! Under -d, write a message describing the current IDs.  */

void
log_access (char *flavor)
{
  if (! ISDB (DB_JOBS))
    return;

  /* All the other debugging messages go to stdout,
     but we write this one to stderr because it might be
     run in a child fork whose stdout is piped.  */

  fprintf (stderr, _("%s: user %lu (real %lu), group %lu (real %lu)\n"),
	   flavor, (unsigned long) geteuid (), (unsigned long) getuid (),
           (unsigned long) getegid (), (unsigned long) getgid ());
  fflush (stderr);
}


/*! Write a message indicating that we've just entered or
   left (according to ENTERING) the current directory.  */
void
log_working_directory (int entering)
{
  static int entered = 0;

  /* Print nothing without the flag.  Don't print the entering message
     again if we already have.  Don't print the leaving message if we
     haven't printed the entering message.  */
  if (! print_directory_flag || entering == entered)
    return;

  entered = entering;

  if (print_data_base_flag)
    fputs ("# ", stdout);

  /* Use entire sentences to give the translators a fighting chance.  */

  if (makelevel == 0)
    if (starting_directory == 0)
      if (entering)
        printf (_("%s: Entering an unknown directory\n"), program);
      else
        printf (_("%s: Leaving an unknown directory\n"), program);
    else
      if (entering)
        printf (_("%s: Entering directory `%s'\n"),
                program, starting_directory);
      else
        printf (_("%s: Leaving directory `%s'\n"),
                program, starting_directory);
  else
    if (starting_directory == 0)
      if (entering)
        printf (_("%s[%u]: Entering an unknown directory\n"),
                program, makelevel);
      else
        printf (_("%s[%u]: Leaving an unknown directory\n"),
                program, makelevel);
    else
      if (entering)
        printf (_("%s[%u]: Entering directory `%s'\n"),
                program, makelevel, starting_directory);
      else
        printf (_("%s[%u]: Leaving directory `%s'\n"),
                program, makelevel, starting_directory);
}

/*! Display a variable and its value. */
void 
print_variable (variable_t *p_v)
{
  if (p_v) {
    const char *psz_origin = origin2str(p_v->origin);
    if (NULL != p_v->fileinfo.filenm) {
      printf("%s:%lu (origin: %s) %s = %s\n", 
	     p_v->fileinfo.filenm, p_v->fileinfo.lineno,
	     psz_origin,
	     p_v->name, p_v->value);
    } else {
      printf("(origin %s) %s = %s\n", psz_origin, p_v->name, p_v->value);
    }
  }
}

/*! Display a variable and its value with all substitutions included. */
void 
print_variable_expand (variable_t *p_v)
{
  if (p_v) {
    const char *psz_origin = origin2str(p_v->origin);
    if (NULL != p_v->fileinfo.filenm) {
      printf("%s:%lu (origin: %s) %s := %s\n", 
	     p_v->fileinfo.filenm, p_v->fileinfo.lineno,
	     psz_origin,
	     p_v->name, variable_expand(p_v->value));
    } else {
      printf("(origin %s) %s := %s\n", psz_origin, 
	     p_v->name, variable_expand(p_v->value));
    }
  }
}

/*! Show a command before executing it. */
extern void 
print_target_prefix (const char *p_name) 
{
  printf(" %s", p_name);
  if (makelevel != 0) {
    printf ("[%u]", makelevel);
  }
}

/*! Show a command before executing it. */
extern void 
print_file_target_prefix (const file_t *p_target) 
{
  print_floc_prefix(&(p_target->floc));
  print_target_prefix(p_target->name);
}

/*! Show a command before executing it. */
extern void 
print_floc_prefix (const floc_t *p_floc) 
{
  if (!p_floc) return;
  if (p_floc->filenm) {
    if (!basename_filenames && strlen(p_floc->filenm) 
	&& p_floc->filenm[0] != '/') 
      printf("%s/", starting_directory);
    printf("%s:%lu", p_floc->filenm, p_floc->lineno);
  } else {
    if (!basename_filenames)
      printf("%s/", starting_directory);
    printf("??:%lu", p_floc->lineno);
  }
}

/*! Show a command before executing it. */
extern debug_return_t
print_child_cmd (child_t *p_child, target_stack_node_t *p)
{
  debug_return_t rc = continue_execution;

  if (!p_child) return continue_execution;

  if (i_debugger_stepping || p_child->file->tracing) {
    rc=enter_debugger(p, p_child->file, 0);
  }

  return rc;
}

/*! Display the target stack. i_pos is the position we are currently.
  i_max is the maximum number of entries to show.
 */
extern void 
print_target_stack (target_stack_node_t *p, int i_pos, int i_max)
{
  unsigned int i=0;
  printf("\n");
  for ( ; p && i < i_max ; 
	i++, p = p->p_parent  ) {
    floc_t floc;
    file_t *p_target = p->p_target;

    /* If we don't have a line recorded for the target,
       but we do have one for the commands it runs,
       use that.
    */
    if (p_target->floc.filenm) {
      memcpy(&floc, &(p_target->floc), sizeof(floc_t));
    } else if (p_target->cmds) {
      memcpy(&floc, &(p_target->cmds->fileinfo.filenm), sizeof(floc_t));
      /* HACK: is it okay to assume that the target is on the line
	 before the first command? Or should we list the line
	 that the command starts on - so we know we've faked the location?
      */
      floc.lineno--;
    } else {
	floc.filenm = NULL;
    }
    
    if (floc.filenm) {
      if (i_pos != -1) {
	printf("%s", (i == i_pos) ? "=>" : "  ");
      }
      printf ("#%u  %s at ", i, p_target->name);
      print_floc_prefix(&floc);
    } else {
      if (i_pos != -1) {
	printf("%s", (i == i_pos) ? "=>" : "  ");
      }
      if (p_target->phony)
	printf ("#%u  %s (.PHONY target)", i, p_target->name);
      else 
	printf ("#%u  %s at ??", i, p_target->name);

    }
    printf ("\n");
  }
}

/*! Display the Makefile read stack. i_pos is the position we are currently.
  i_max is the maximum number of entries to show. */
extern void 
print_floc_stack (int i_pos, int i_max)
{
  unsigned int i=0;
  floc_stack_node_t *p;
  printf("\n");
  for ( p=p_stack_floc_top; p && i < i_max ; 
	i++, p = p->p_parent ) {
    if (i_pos != -1) {
      printf("%s", (i == i_pos) ? "=>" : "  ");
    }
    printf ("#%u  ", i);
    if (p->p_floc->filenm) {
      print_floc_prefix(p->p_floc);
    }
    printf ("\n");
  }
}

/*! Print the list makefiles read by read_makefiles().  */
void print_read_makefiles (void) 
{
  dep_t *p_dep;
  if (!read_makefiles) return;
  for (p_dep = read_makefiles; p_dep; p_dep = p_dep->next) {
    if (p_dep->file) {
      if (p_dep != read_makefiles)
	printf(", ");
      printf("%s", p_dep->file->name);
    }
  }
  printf("\n");
}

/*! Print the command line used to invode this program */
void print_cmdline (void) 
{
  unsigned int i;
  printf("Command-line arguments:");
  if (global_argv[1]) {
    printf("\n\t\"%s", global_argv[1]);
    for (i = 2; global_argv[i]; i++) {
      printf(" %s", global_argv[i]);
    }
    printf("\"");
  } else {
    printf(" none");
  }
  printf("\n");
}

