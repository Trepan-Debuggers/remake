/* Convert between signal names and numbers.
   Copyright (C) 1990, 1992, 1993, 1995, 1996 Free Software Foundation, Inc.
   This file is part of the GNU C Library.

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public License as
   published by the Free Software Foundation; either version 2 of the
   License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public
   License along with the GNU C Library; see the file COPYING.LIB.  If not,
   write to the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.  */

#include "make.h"

#include <stdio.h>
#include <sys/types.h>		/* Some systems need this for <signal.h>.  */
#include <signal.h>

#ifdef HAVE_STRING_H
#include <string.h>
#endif

/* Some systems declare `sys_siglist in <unistd.h>; if
   configure defined SYS_SIGLIST_DECLARED, it may expect
   to find the declaration there.  */
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif


/* Some systems do not define NSIG in <signal.h>.  */
#ifndef	NSIG
#ifdef	_NSIG
#define	NSIG	_NSIG
#else
#define	NSIG	32
#endif
#endif

#if !__STDC__
#define const
#endif

#include "signame.h"

#ifndef HAVE_SYS_SIGLIST
/* There is too much variation in Sys V signal numbers and names, so
   we must initialize them at runtime.  */

static const char *undoc;

const char *sys_siglist[NSIG];

#else	/* HAVE_SYS_SIGLIST.  */

#ifndef SYS_SIGLIST_DECLARED
extern char *sys_siglist[];
#endif	/* Not SYS_SIGLIST_DECLARED.  */

#endif	/* Not HAVE_SYS_SIGLIST.  */

/* Table of abbreviations for signals.  Note:  A given number can
   appear more than once with different abbreviations.  */
#define SIG_TABLE_SIZE  (NSIG*2)

typedef struct
  {
    int number;
    const char *abbrev;
  } num_abbrev;
static num_abbrev sig_table[SIG_TABLE_SIZE];
/* Number of elements of sig_table used.  */
static int sig_table_nelts = 0;

/* Enter signal number NUMBER into the tables with ABBREV and NAME.  */

static void
init_sig (number, abbrev, name)
     int number;
     const char *abbrev;
     const char *name;
{
#ifndef HAVE_SYS_SIGLIST
  /* If this value is ever greater than NSIG it seems like it'd be a bug in
     the system headers, but... better safe than sorry.  We know, for
     example, that this isn't always true on VMS.  */

  if (number >= 0 && number < NSIG)
    sys_siglist[number] = name;
#endif
  if (sig_table_nelts < SIG_TABLE_SIZE)
    {
      sig_table[sig_table_nelts].number = number;
      sig_table[sig_table_nelts++].abbrev = abbrev;
    }
}

void
signame_init ()
{
#ifndef HAVE_SYS_SIGLIST
  int i;
  char *u = _("unknown signal");

  undoc = xstrdup(u);

  /* Initialize signal names.  */
  for (i = 0; i < NSIG; i++)
    sys_siglist[i] = undoc;
#endif /* !HAVE_SYS_SIGLIST */

  /* Initialize signal names.  */
#if defined (SIGHUP)
  init_sig (SIGHUP, "HUP", _("Hangup"));
#endif
#if defined (SIGINT)
  init_sig (SIGINT, "INT", _("Interrupt"));
#endif
#if defined (SIGQUIT)
  init_sig (SIGQUIT, "QUIT", _("Quit"));
#endif
#if defined (SIGILL)
  init_sig (SIGILL, "ILL", _("Illegal Instruction"));
#endif
#if defined (SIGTRAP)
  init_sig (SIGTRAP, "TRAP", _("Trace/breakpoint trap"));
#endif
  /* If SIGIOT == SIGABRT, we want to print it as SIGABRT because
     SIGABRT is in ANSI and POSIX.1 and SIGIOT isn't.  */
#if defined (SIGABRT)
  init_sig (SIGABRT, "ABRT", _("Aborted"));
#endif
#if defined (SIGIOT)
  init_sig (SIGIOT, "IOT", _("IOT trap"));
#endif
#if defined (SIGEMT)
  init_sig (SIGEMT, "EMT", _("EMT trap"));
#endif
#if defined (SIGFPE)
  init_sig (SIGFPE, "FPE", _("Floating point exception"));
#endif
#if defined (SIGKILL)
  init_sig (SIGKILL, "KILL", _("Killed"));
#endif
#if defined (SIGBUS)
  init_sig (SIGBUS, "BUS", _("Bus error"));
#endif
#if defined (SIGSEGV)
  init_sig (SIGSEGV, "SEGV", _("Segmentation fault"));
#endif
#if defined (SIGSYS)
  init_sig (SIGSYS, "SYS", _("Bad system call"));
#endif
#if defined (SIGPIPE)
  init_sig (SIGPIPE, "PIPE", _("Broken pipe"));
#endif
#if defined (SIGALRM)
  init_sig (SIGALRM, "ALRM", _("Alarm clock"));
#endif
#if defined (SIGTERM)
  init_sig (SIGTERM, "TERM", _("Terminated"));
#endif
#if defined (SIGUSR1)
  init_sig (SIGUSR1, "USR1", _("User defined signal 1"));
#endif
#if defined (SIGUSR2)
  init_sig (SIGUSR2, "USR2", _("User defined signal 2"));
#endif
  /* If SIGCLD == SIGCHLD, we want to print it as SIGCHLD because that
     is what is in POSIX.1.  */
#if defined (SIGCHLD)
  init_sig (SIGCHLD, "CHLD", _("Child exited"));
#endif
#if defined (SIGCLD)
  init_sig (SIGCLD, "CLD", _("Child exited"));
#endif
#if defined (SIGPWR)
  init_sig (SIGPWR, "PWR", _("Power failure"));
#endif
#if defined (SIGTSTP)
  init_sig (SIGTSTP, "TSTP", _("Stopped"));
#endif
#if defined (SIGTTIN)
  init_sig (SIGTTIN, "TTIN", _("Stopped (tty input)"));
#endif
#if defined (SIGTTOU)
  init_sig (SIGTTOU, "TTOU", _("Stopped (tty output)"));
#endif
#if defined (SIGSTOP)
  init_sig (SIGSTOP, "STOP", _("Stopped (signal)"));
#endif
#if defined (SIGXCPU)
  init_sig (SIGXCPU, "XCPU", _("CPU time limit exceeded"));
#endif
#if defined (SIGXFSZ)
  init_sig (SIGXFSZ, "XFSZ", _("File size limit exceeded"));
#endif
#if defined (SIGVTALRM)
  init_sig (SIGVTALRM, "VTALRM", _("Virtual timer expired"));
#endif
#if defined (SIGPROF)
  init_sig (SIGPROF, "PROF", _("Profiling timer expired"));
#endif
#if defined (SIGWINCH)
  /* "Window size changed" might be more accurate, but even if that
     is all that it means now, perhaps in the future it will be
     extended to cover other kinds of window changes.  */
  init_sig (SIGWINCH, "WINCH", _("Window changed"));
#endif
#if defined (SIGCONT)
  init_sig (SIGCONT, "CONT", _("Continued"));
#endif
#if defined (SIGURG)
  init_sig (SIGURG, "URG", _("Urgent I/O condition"));
#endif
#if defined (SIGIO)
  /* "I/O pending" has also been suggested.  A disadvantage is
     that signal only happens when the process has
     asked for it, not everytime I/O is pending.  Another disadvantage
     is the confusion from giving it a different name than under Unix.  */
  init_sig (SIGIO, "IO", _("I/O possible"));
#endif
#if defined (SIGWIND)
  init_sig (SIGWIND, "WIND", _("SIGWIND"));
#endif
#if defined (SIGPHONE)
  init_sig (SIGPHONE, "PHONE", _("SIGPHONE"));
#endif
#if defined (SIGPOLL)
  init_sig (SIGPOLL, "POLL", _("I/O possible"));
#endif
#if defined (SIGLOST)
  init_sig (SIGLOST, "LOST", _("Resource lost"));
#endif
#if defined (SIGDANGER)
  init_sig (SIGDANGER, "DANGER", _("Danger signal"));
#endif
#if defined (SIGINFO)
  init_sig (SIGINFO, "INFO", _("Information request"));
#endif
#if defined (SIGNOFP)
  init_sig (SIGNOFP, "NOFP", _("Floating point co-processor not available"));
#endif
}

/* Return the abbreviation for signal NUMBER.  */

char *
sig_abbrev (number)
     int number;
{
  int i;

  if (sig_table_nelts == 0)
    signame_init ();

  for (i = 0; i < sig_table_nelts; i++)
    if (sig_table[i].number == number)
      return (char *)sig_table[i].abbrev;
  return NULL;
}

/* Return the signal number for an ABBREV, or -1 if there is no
   signal by that name.  */

int
sig_number (abbrev)
     const char *abbrev;
{
  int i;

  if (sig_table_nelts == 0)
    signame_init ();

  /* Skip over "SIG" if present.  */
  if (abbrev[0] == 'S' && abbrev[1] == 'I' && abbrev[2] == 'G')
    abbrev += 3;

  for (i = 0; i < sig_table_nelts; i++)
    if (abbrev[0] == sig_table[i].abbrev[0]
	&& strcmp (abbrev, sig_table[i].abbrev) == 0)
      return sig_table[i].number;
  return -1;
}

#ifndef HAVE_PSIGNAL
/* Print to standard error the name of SIGNAL, preceded by MESSAGE and
   a colon, and followed by a newline.  */

void
psignal (signal, message)
     int signal;
     const char *message;
{
  if (signal <= 0 || signal >= NSIG)
    fprintf (stderr, "%s: unknown signal", message);
  else
    fprintf (stderr, "%s: %s\n", message, sys_siglist[signal]);
}
#endif

#ifndef HAVE_STRSIGNAL
/* Return the string associated with the signal number.  */

char *
strsignal (signal)
     int signal;
{
  static char buf[] = "Signal 12345678901234567890";

  if (signal > 0 || signal < NSIG)
    return (char *) sys_siglist[signal];

  sprintf (buf, "Signal %d", signal);
  return buf;
}
#endif
