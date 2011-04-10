/* Output or logging functions for GNU Make.  

Copyright (C) 2005, 2007, 2008 R. Bernstein <rocky@gnu.org>
This file is part of GNU Make (remake variant).
Copyright (C) 2004, 2005, 2007, 2008, Free Software Foundation, Inc.  

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
#include "debugger/cmd.h"
#include "debug.h"
#include "dep.h"
#include "read.h"
#include "print.h"

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
#if HAVE_ANSI_COMPILER && USE_VARIADIC && HAVE_STDARG_H
error (const floc_t *flocp, const char *fmt, ...)
#else
error (flocp, fmt, va_alist)
     const floc_t *flocp;
     const char *fmt;
     va_dcl
#endif
{
#if USE_VARIADIC
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
    enter_debugger(NULL, NULL, -1, DEBUG_ERROR_HIT);
}

void
#if __STDC__ && HAVE_STDVARARGS
err_with_stack (target_stack_node_t *p_call, const char *fmt, ...)
#else
err_with_stack (p_call, fmt, va_alist)
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
    enter_debugger(p_call, p_target, -1, DEBUG_ERROR_HIT);
}

/* Print an error message and exit.  */

void
#if HAVE_ANSI_COMPILER && USE_VARIADIC && HAVE_STDARG_H
fatal (const floc_t *flocp, const char *fmt, ...)
#else
fatal (flocp, fmt, va_alist)
     const floc_t *flocp;
     const char *fmt;
     va_dcl
#endif
{
#if USE_VARIADIC
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
      enter_debugger(NULL, NULL, 2, DEBUG_ERROR_HIT);
    die (2);
    break;
  case DEBUGGER_QUIT_RC:
    die(DEBUGGER_QUIT_RC);
  default:
  case 1: 
    longjmp(debugger_loop, 0);
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
    enter_debugger(p_call, p_target, 2, DEBUG_ERROR_HIT);
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
    enter_debugger(NULL, NULL, -1, DEBUG_ERROR_HIT);
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

/*! Display a variable and its value. */
void 
print_variable (variable_t *p_v)
{
  if (p_v) {
    const char *psz_origin = origin2str(p_v->origin);
    if (NULL != p_v->fileinfo.filenm) {
      printf(_("%s:%lu (origin: %s) %s = %s\n"), 
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
      printf(_("%s:%lu (origin: %s) %s := %s\n"), 
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

/*! Show target information: location and name. */
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
    debug_enter_reason_t reason = DEBUG_STEP_HIT;
    if (i_debugger_stepping)
      reason = DEBUG_STEP_HIT;
    else if (p_child->file->tracing & BRK_BEFORE_PREREQ) 
      reason = DEBUG_BRKPT_BEFORE_PREREQ;
    else if (p_child->file->tracing & BRK_BEFORE_PREREQ) 
      reason = DEBUG_BRKPT_AFTER_PREREQ;
      
    rc=enter_debugger(p, p_child->file, 0, reason);
  }

  return rc;
}

/*! Display the target stack. i_pos is the position we are currently.
  i_max is the maximum number of entries to show.
 */
extern void 
print_target_stack (target_stack_node_t *p, int i_pos, int i_max)
{
  int i=0;
  printf("\n");
  for ( ; p && i < i_max ; 
	i++, p = p->p_parent  ) {
    floc_t floc;
    file_t *p_target = p->p_target;
    const char *psz_target_name = 
      (p_target && p_target->name) ? p_target->name : "(null)";

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
      printf ("#%u  %s at ", i, psz_target_name);
      print_floc_prefix(&floc);
    } else {
      if (i_pos != -1) {
	printf("%s", (i == i_pos) ? "=>" : "  ");
      }
      if (p_target->phony)
	printf ("#%u  %s (.PHONY target)", i, psz_target_name);
      else 
	printf ("#%u  %s at ??", i, psz_target_name);

    }
    printf ("\n");
  }
}

/*! Display the Makefile read stack. i_pos is the position we are currently.
  i_max is the maximum number of entries to show. */
extern void 
print_floc_stack (int i_pos, int i_max)
{
  int i=0;
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

/*! Print the file information.  */
void print_file (file_t *p_file) 
{
  char buf[FILE_TIMESTAMP_PRINT_LEN_BOUND + 1];
  printf("File %s:\n", p_file->name);
  file_timestamp_sprintf (buf, p_file->last_mtime);
  printf("\tLast modified: %s\n",  buf);
  if (p_file->mtime_before_update != p_file->last_mtime) {
    file_timestamp_sprintf (buf, p_file->mtime_before_update);
    printf("\tBefore update: %s\n",  buf);
  }
  printf("\tNumber of lines: %u\n",  p_file->nlines);
}

/*! Print the list makefiles read by read_makefiles().  */
bool print_read_makefiles(const char *psz_filename)
{
  dep_t *p_dep;
  if (!read_makefiles) return false;
  if (NULL == psz_filename) {
    for (p_dep = read_makefiles; p_dep; p_dep = p_dep->next) {
      if (p_dep->file) {
        print_file(p_dep->file);
      }
    }
    return true;
  } else {
    for (p_dep = read_makefiles; p_dep; p_dep = p_dep->next) {
      if (p_dep->file && 0 == strcmp(p_dep->file->name, psz_filename)) {
        print_file(p_dep->file);
        return true;
      }
    }
  }
  return false;
}

/*! Print the command line used to invoke Make. */
void print_cmdline (void) 
{
  unsigned int i;
  printf(_("Command-line arguments:"));
  printf("\n\t\"");
  if (global_argv[1]) {
    printf("%s", argv0);
    for (i = 1; global_argv[i]; i++) {
      printf(" %s", global_argv[i]);
    }
    printf("\"");
  } else {
    printf(_(" none"));
  }
  printf("\n");
}


/* 
 * Local variables:
 * eval: (c-set-style "gnu")
 * indent-tabs-mode: nil
 * End: 
 */
