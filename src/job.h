/* $Id: job.h,v 1.7 2005/12/02 12:12:09 rockyb Exp $
Definitions for managing subprocesses in GNU Make.
Copyright (C) 1992, 1993, 1996, 1999, 2005 Free Software Foundation, Inc.
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

#ifndef JOB_H
#define JOB_H

#include "trace.h"

#ifdef HAVE_FCNTL_H
# include <fcntl.h>
#else
# include <sys/file.h>
#endif

/** How to set close-on-exec for a file descriptor.  */

#if !defined F_SETFD
# define CLOSE_ON_EXEC(_d)
#else
# ifndef FD_CLOEXEC
#  define FD_CLOEXEC 1
# endif
# define CLOSE_ON_EXEC(_d) (void) fcntl ((_d), F_SETFD, FD_CLOEXEC)
#endif

/** Structure describing a running or dead child process.  */
typedef struct child
  {
    struct child *next;		/**< Link in the chain.  */

    floc_t fileinfo;	        /**< Where commands were defined. Note:
				   this could be a pattern target.
				 */

    struct file *file;		/**< File being remade.  */
                                /**< Note this can be different from the
				   file of fileinfo. The above might be
				   a pattern target, while this is a target
				   which matches the pattern target and thus
				   uses it. For example we may have
				   main.o here while fileinfo might match
				   .c.o . 
				 */

    char **environment;		/**< Environment for commands.  */

    char **command_lines;	/**< Array of variable-expanded cmd lines.  */
    unsigned int *line_no;	/**< line # offsets of chopped commands. */
    unsigned int command_line;	/**< Index into above.  */
    char *command_ptr;		/**< Ptr into command_lines[command_line].  */

    pid_t pid;			/**< Child process's ID number.  */
    char *sh_batch_file;        /**< Script file for shell commands */
    unsigned int remote:1;	/**< Nonzero if executing remotely.  */

    unsigned int noerror:1;	/**< Nonzero if commands contained a `-'.  */

    unsigned int good_stdin:1;	/**< Nonzero if this child has a good stdin. */
    unsigned int deleted:1;	/**< Nonzero if targets have been deleted.  */
    unsigned int tracing:1;	/**< Nonzero child should be traced.  */
  } child_t;

extern struct child *children;

/*!
 Create a `struct child' for FILE and start its commands running.
*/
extern void new_job (file_t *file, target_stack_node_t *p_call_stack);

extern void reap_children (int block, int err, 
			   target_stack_node_t *p_call_stack);
extern void start_waiting_jobs (target_stack_node_t *p_call_stack);

/*! Figure out the argument list necessary to run LINE as a command.
   Try to avoid using a shell.  This routine handles only ' quoting,
   and " quoting when no backslash, $ or ` characters are seen in the
   quotes.  Starting quotes may be escaped with a backslash.  If any
   of the characters in sh_chars[] is seen, or any of the builtin
   commands listed in sh_cmds[] is the first word of a line, the shell
   is used.

   If RESTP is not NULL, *RESTP is set to point to the first newline in LINE.
   If *RESTP is NULL, newlines will be ignored.

   FILE is the target whose commands these are.  It is used for
   variable expansion for $(SHELL) and $(IFS).  */
extern char **construct_command_argv (char *line, char **restp, file_t *file, 
				      char** batch_file);

/*! Start a child process. This function returns the new pid.  */
extern void child_execute_job (int stdin_fd, int stdout_fd, char **argv, char **envp);

/*! Replace the current process with one running the command in ARGV,
   with environment ENVP.  This function does not return.  */
extern void exec_command (char **argv, char **envp);

extern unsigned int job_slots_used;

/*! block getting any maskable signals.  */
extern void block_sigs (void);

#ifdef POSIX
/*! Remove blocks on signals.  */
extern void unblock_sigs (void);
#else
#ifdef	HAVE_SIGSETMASK
extern int fatal_signal_mask;
#define	unblock_sigs()	sigsetmask (0)
#else
#define	unblock_sigs()
#endif
#endif

#endif /* JOB_H */
