/* Job execution and handling for GNU Make.
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

#include "make.h"
#include "job.h"
#include "filedef.h"
#include "commands.h"
#include "variable.h"
#include <assert.h>

/* Default shell to use.  */
#ifdef WINDOWS32
char *default_shell = "sh.exe";
int no_default_sh_exe = 1;
#else  /* WINDOWS32 */
#ifdef _AMIGA
char default_shell[] = "";
extern int MyExecute (char **);
#else
#ifdef __MSDOS__
/* The default shell is a pointer so we can change it if Makefile
   says so.  It is without an explicit path so we get a chance
   to search the $PATH for it (since MSDOS doesn't have standard
   directories we could trust).  */
char *default_shell = "command.com";
#else  /* __MSDOS__ */
char default_shell[] = "/bin/sh";
#endif /* __MSDOS__ */
#endif /* _AMIGA */
#endif /* WINDOWS32 */

#ifdef __MSDOS__
#include <process.h>
static int execute_by_shell;
static int dos_pid = 123;
int dos_status;
int dos_command_running;
#endif /* __MSDOS__ */

#ifdef _AMIGA
#include <proto/dos.h>
static int amiga_pid = 123;
static int amiga_status;
static char amiga_bname[32];
static int amiga_batch_file;
#endif /* Amiga.  */

#ifdef VMS
#include <time.h>
#include <processes.h>
#include <starlet.h>
#include <lib$routines.h>
#endif

#ifdef WINDOWS32
#include <windows.h>
#include <io.h>
#include <process.h>
#include "sub_proc.h"
#include "w32err.h"
#include "pathstuff.h"

/* this stuff used if no sh.exe is around */
static char *dos_bname;
static char *dos_bename;
static int dos_batch_file;
#endif /* WINDOWS32 */

#ifdef HAVE_FCNTL_H
#include <fcntl.h>
#else
#include <sys/file.h>
#endif

#if defined (HAVE_SYS_WAIT_H) || defined (HAVE_UNION_WAIT)
#include <sys/wait.h>
#endif

#ifdef	HAVE_WAITPID
#define	WAIT_NOHANG(status)	waitpid (-1, (status), WNOHANG)
#else	/* Don't have waitpid.  */
#ifdef	HAVE_WAIT3
#ifndef	wait3
extern int wait3 ();
#endif
#define	WAIT_NOHANG(status)	wait3 ((status), WNOHANG, (struct rusage *) 0)
#endif	/* Have wait3.  */
#endif	/* Have waitpid.  */

#if	!defined (wait) && !defined (POSIX)
extern int wait ();
#endif

#ifndef	HAVE_UNION_WAIT

#define	WAIT_T int

#ifndef	WTERMSIG
#define WTERMSIG(x) ((x) & 0x7f)
#endif
#ifndef	WCOREDUMP
#define WCOREDUMP(x) ((x) & 0x80)
#endif
#ifndef	WEXITSTATUS
#define WEXITSTATUS(x) (((x) >> 8) & 0xff)
#endif
#ifndef	WIFSIGNALED
#define WIFSIGNALED(x) (WTERMSIG (x) != 0)
#endif
#ifndef	WIFEXITED
#define WIFEXITED(x) (WTERMSIG (x) == 0)
#endif

#else	/* Have `union wait'.  */

#define WAIT_T union wait
#ifndef	WTERMSIG
#define WTERMSIG(x)	((x).w_termsig)
#endif
#ifndef	WCOREDUMP
#define WCOREDUMP(x)	((x).w_coredump)
#endif
#ifndef WEXITSTATUS
#define WEXITSTATUS(x)	((x).w_retcode)
#endif
#ifndef	WIFSIGNALED
#define	WIFSIGNALED(x)	(WTERMSIG(x) != 0)
#endif
#ifndef	WIFEXITED
#define	WIFEXITED(x)	(WTERMSIG(x) == 0)
#endif

#endif	/* Don't have `union wait'.  */

#ifdef VMS
static int vms_jobsefnmask=0;
#endif /* !VMS */

#ifndef	HAVE_UNISTD_H
extern int dup2 ();
extern int execve ();
extern void _exit ();
#ifndef VMS
extern int geteuid ();
extern int getegid ();
extern int setgid ();
extern int getgid ();
#endif
#endif

extern char *allocated_variable_expand_for_file PARAMS ((char *line, struct file *file));

extern int getloadavg PARAMS ((double loadavg[], int nelem));
extern int start_remote_job PARAMS ((char **argv, char **envp, int stdin_fd,
		int *is_remote, int *id_ptr, int *used_stdin));
extern int start_remote_job_p PARAMS ((void));
extern int remote_status PARAMS ((int *exit_code_ptr, int *signal_ptr,
		int *coredump_ptr, int block));

RETSIGTYPE child_handler PARAMS ((int));
static void free_child PARAMS ((struct child *));
static void start_job_command PARAMS ((struct child *child));
static int load_too_high PARAMS ((void));
static int job_next_command PARAMS ((struct child *));
static int start_waiting_job PARAMS ((struct child *));
#ifdef VMS
static void vmsWaitForChildren PARAMS ((int *));
#endif

/* Chain of all live (or recently deceased) children.  */

struct child *children = 0;

/* Number of children currently running.  */

unsigned int job_slots_used = 0;

/* Nonzero if the `good' standard input is in use.  */

static int good_stdin_used = 0;

/* Chain of children waiting to run until the load average goes down.  */

static struct child *waiting_jobs = 0;

/* Non-zero if we use a *real* shell (always so on Unix).  */

int unixy_shell = 1;

#ifdef WINDOWS32
/*
 * The macro which references this function is defined in make.h.
 */
int w32_kill(int pid, int sig)
{
       return ((process_kill(pid, sig) == TRUE) ? 0 : -1);
}
#endif /* WINDOWS32 */

/* Write an error message describing the exit status given in
   EXIT_CODE, EXIT_SIG, and COREDUMP, for the target TARGET_NAME.
   Append "(ignored)" if IGNORED is nonzero.  */

static void
child_error (target_name, exit_code, exit_sig, coredump, ignored)
     char *target_name;
     int exit_code, exit_sig, coredump;
     int ignored;
{
  if (ignored && silent_flag)
    return;

#ifdef VMS
  if (!(exit_code & 1))
      error("*** [%s] Error 0x%x%s", target_name, exit_code, ((ignored)? " (ignored)" : ""));
#else
  if (exit_sig == 0)
    error (ignored ? "[%s] Error %d (ignored)" :
	   "*** [%s] Error %d",
	   target_name, exit_code);
  else
    error ("*** [%s] %s%s",
	   target_name, strsignal (exit_sig),
	   coredump ? " (core dumped)" : "");
#endif /* VMS */
}

static unsigned int dead_children = 0;

#ifdef VMS
/* Wait for nchildren children to terminate */
static void
vmsWaitForChildren(int *status)
{
  while (1)
    {
      if (!vms_jobsefnmask)
	{
	  *status = 0;
	  return;
	}

      *status = sys$wflor (32, vms_jobsefnmask);
    }
  return;
}
#endif


/* Notice that a child died.
   reap_children should be called when convenient.  */
RETSIGTYPE
child_handler (sig)
     int sig;
{
  ++dead_children;

  if (debug_flag)
    printf ("Got a SIGCHLD; %d unreaped children.\n", dead_children);
}

extern int shell_function_pid, shell_function_completed;

/* Reap dead children, storing the returned status and the new command
   state (`cs_finished') in the `file' member of the `struct child' for the
   dead child, and removing the child from the chain.  If BLOCK nonzero,
   reap at least one child, waiting for it to die if necessary.  If ERR is
   nonzero, print an error message first.  */

void
reap_children (block, err)
     int block, err;
{
  WAIT_T status;

  while ((children != 0 || shell_function_pid != 0) &&
	 (block || dead_children > 0))
    {
      int remote = 0;
      register int pid;
      int exit_code, exit_sig, coredump;
      register struct child *lastc, *c;
      int child_failed;
      int any_remote, any_local;

      if (err && dead_children == 0)
	{
	  /* We might block for a while, so let the user know why.  */
	  fflush (stdout);
	  error ("*** Waiting for unfinished jobs....");
	}

      /* We have one less dead child to reap.
	 The test and decrement are not atomic; if it is compiled into:
	 	register = dead_children - 1;
		dead_children = register;
	 a SIGCHLD could come between the two instructions.
	 child_handler increments dead_children.
	 The second instruction here would lose that increment.  But the
	 only effect of dead_children being wrong is that we might wait
	 longer than necessary to reap a child, and lose some parallelism;
	 and we might print the "Waiting for unfinished jobs" message above
	 when not necessary.  */

      if (dead_children > 0)
	--dead_children;

      any_remote = 0;
      any_local = shell_function_pid != 0;
      for (c = children; c != 0; c = c->next)
	{
	  any_remote |= c->remote;
	  any_local |= ! c->remote;
	  if (debug_flag)
	    printf ("Live child 0x%08lx PID %d%s\n",
		    (unsigned long int) c,
		    c->pid, c->remote ? " (remote)" : "");
#ifdef VMS
	  break;
#endif
	}

      /* First, check for remote children.  */
      if (any_remote)
	pid = remote_status (&exit_code, &exit_sig, &coredump, 0);
      else
	pid = 0;

      if (pid < 0)
	{
	remote_status_lose:
#ifdef	EINTR
	  if (errno == EINTR)
	    continue;
#endif
	  pfatal_with_name ("remote_status");
	}
      else if (pid == 0)
	{
#if !defined(__MSDOS__) && !defined(_AMIGA) && !defined(WINDOWS32)
	  /* No remote children.  Check for local children.  */

	  if (any_local)
	    {
#ifdef VMS
	      vmsWaitForChildren (&status);
	      pid = c->pid;
#else
#ifdef WAIT_NOHANG
	      if (!block)
		pid = WAIT_NOHANG (&status);
	      else
#endif
		pid = wait (&status);
#endif /* !VMS */
	    }
	  else
	    pid = 0;

	  if (pid < 0)
	    {
#ifdef EINTR
	      if (errno == EINTR)
		continue;
#endif
	      pfatal_with_name ("wait");
	    }
	  else if (pid == 0)
	    {
	      /* No local children.  */
	      if (block && any_remote)
		{
		  /* Now try a blocking wait for a remote child.  */
		  pid = remote_status (&exit_code, &exit_sig, &coredump, 1);
		  if (pid < 0)
		    goto remote_status_lose;
		  else if (pid == 0)
		    /* No remote children either.  Finally give up.  */
		    break;
		  else
		    /* We got a remote child.  */
		    remote = 1;
		}
	      else
		break;
	    }
	  else
	    {
	      /* Chop the status word up.  */
	      exit_code = WEXITSTATUS (status);
	      exit_sig = WIFSIGNALED (status) ? WTERMSIG (status) : 0;
	      coredump = WCOREDUMP (status);
	    }
#else	/* __MSDOS__, Amiga, WINDOWS32.  */
#ifdef __MSDOS__
	  /* Life is very different on MSDOS.  */
	  pid = dos_pid - 1;
	  status = dos_status;
	  exit_code = WEXITSTATUS (status);
	  if (exit_code == 0xff)
	    exit_code = -1;
	  exit_sig = WIFSIGNALED (status) ? WTERMSIG (status) : 0;
	  coredump = 0;
#endif /* __MSDOS__ */
#ifdef _AMIGA
	  /* Same on Amiga */
	  pid = amiga_pid - 1;
	  status = amiga_status;
	  exit_code = amiga_status;
	  exit_sig = 0;
	  coredump = 0;
#endif /* _AMIGA */
#ifdef WINDOWS32
         {
           HANDLE hPID;
           int err;

           /* wait for anything to finish */
           if (hPID = process_wait_for_any()) {

             /* was an error found on this process? */
             err = process_last_err(hPID);

             /* get exit data */
             exit_code = process_exit_code(hPID);

             if (err)
               fprintf(stderr, "make (e=%d): %s",
                       exit_code, map_windows32_error_to_string(exit_code));

             exit_sig = process_signal(hPID);

             /* cleanup process */
             process_cleanup(hPID);

             if (dos_batch_file) {
               remove (dos_bname);
               remove (dos_bename);
               dos_batch_file = 0;
             }

             coredump = 0;
           }
           pid = (int) hPID;
         }
#endif /* WINDOWS32 */
#endif	/* Not __MSDOS__ */
	}
      else
	/* We got a remote child.  */
	remote = 1;

      /* Check if this is the child of the `shell' function.  */
      if (!remote && pid == shell_function_pid)
	{
	  /* It is.  Leave an indicator for the `shell' function.  */
	  if (exit_sig == 0 && exit_code == 127)
	    shell_function_completed = -1;
	  else
	    shell_function_completed = 1;
	  break;
	}

      child_failed = exit_sig != 0 || exit_code != 0;

      /* Search for a child matching the deceased one.  */
      lastc = 0;
      for (c = children; c != 0; lastc = c, c = c->next)
	if (c->remote == remote && c->pid == pid)
	  break;

      if (c == 0)
	{
	  /* An unknown child died.  */
	  char buf[100];
	  sprintf (buf, "Unknown%s job %d", remote ? " remote" : "", pid);
	  if (child_failed)
	    child_error (buf, exit_code, exit_sig, coredump,
			 ignore_errors_flag);
	  else
	    error ("%s finished.", buf);
	}
      else
	{
	  if (debug_flag)
	    printf ("Reaping %s child 0x%08lx PID %d%s\n",
		    child_failed ? "losing" : "winning",
		    (unsigned long int) c,
		    c->pid, c->remote ? " (remote)" : "");

	  /* If this child had the good stdin, say it is now free.  */
	  if (c->good_stdin)
	    good_stdin_used = 0;

	  if (child_failed && !c->noerror && !ignore_errors_flag)
	    {
	      /* The commands failed.  Write an error message,
		 delete non-precious targets, and abort.  */
	      static int delete_on_error = -1;
	      child_error (c->file->name, exit_code, exit_sig, coredump, 0);
	      c->file->update_status = 2;
	      if (delete_on_error == -1)
		{
		  struct file *f = lookup_file (".DELETE_ON_ERROR");
		  delete_on_error = f != 0 && f->is_target;
		}
	      if (exit_sig != 0 || delete_on_error)
		delete_child_targets (c);
	    }
	  else
	    {
	      if (child_failed)
		{
		  /* The commands failed, but we don't care.  */
		  child_error (c->file->name,
			       exit_code, exit_sig, coredump, 1);
		  child_failed = 0;
		}

	      /* If there are more commands to run, try to start them.  */
	      if (job_next_command (c))
		{
		  if (handling_fatal_signal)
		    {
		      /* Never start new commands while we are dying.
			 Since there are more commands that wanted to be run,
			 the target was not completely remade.  So we treat
			 this as if a command had failed.  */
		      c->file->update_status = 2;
		    }
		  else
		    {
		      /* Check again whether to start remotely.
			 Whether or not we want to changes over time.
			 Also, start_remote_job may need state set up
			 by start_remote_job_p.  */
		      c->remote = start_remote_job_p ();
		      start_job_command (c);
		      /* Fatal signals are left blocked in case we were
			 about to put that child on the chain.  But it is
			 already there, so it is safe for a fatal signal to
			 arrive now; it will clean up this child's targets.  */
		      unblock_sigs ();
		      if (c->file->command_state == cs_running)
			/* We successfully started the new command.
			   Loop to reap more children.  */
			continue;
		    }

		  if (c->file->update_status != 0)
		    /* We failed to start the commands.  */
		    delete_child_targets (c);
		}
	      else
		/* There are no more commands.  We got through them all
		   without an unignored error.  Now the target has been
		   successfully updated.  */
		c->file->update_status = 0;
	    }

	  /* When we get here, all the commands for C->file are finished
	     (or aborted) and C->file->update_status contains 0 or 2.  But
	     C->file->command_state is still cs_running if all the commands
	     ran; notice_finish_file looks for cs_running to tell it that
	     it's interesting to check the file's modtime again now.  */

	  if (! handling_fatal_signal)
	    /* Notice if the target of the commands has been changed.
	       This also propagates its values for command_state and
	       update_status to its also_make files.  */
	    notice_finished_file (c->file);

	  if (debug_flag)
	    printf ("Removing child 0x%08lx PID %d%s from chain.\n",
		    (unsigned long int) c,
		    c->pid, c->remote ? " (remote)" : "");

	  /* Block fatal signals while frobnicating the list, so that
	     children and job_slots_used are always consistent.  Otherwise
	     a fatal signal arriving after the child is off the chain and
	     before job_slots_used is decremented would believe a child was
	     live and call reap_children again.  */
	  block_sigs ();

	  /* Remove the child from the chain and free it.  */
	  if (lastc == 0)
	    children = c->next;
	  else
	    lastc->next = c->next;
	  if (! handling_fatal_signal) /* Don't bother if about to die.  */
	    free_child (c);

	  /* There is now another slot open.  */
	  if (job_slots_used > 0)
	    --job_slots_used;

	  unblock_sigs ();

	  /* If the job failed, and the -k flag was not given, die,
	     unless we are already in the process of dying.  */
	  if (!err && child_failed && !keep_going_flag &&
	      /* fatal_error_signal will die with the right signal.  */
	      !handling_fatal_signal)
	    die (2);
	}

      /* Only block for one child.  */
      block = 0;
    }
  return;
}

/* Free the storage allocated for CHILD.  */

static void
free_child (child)
     register struct child *child;
{
  if (child->command_lines != 0)
    {
      register unsigned int i;
      for (i = 0; i < child->file->cmds->ncommand_lines; ++i)
	free (child->command_lines[i]);
      free ((char *) child->command_lines);
    }

  if (child->environment != 0)
    {
      register char **ep = child->environment;
      while (*ep != 0)
	free (*ep++);
      free ((char *) child->environment);
    }

  free ((char *) child);
}

#ifdef POSIX
extern sigset_t fatal_signal_set;
#endif

void
block_sigs ()
{
#ifdef	 POSIX
  (void) sigprocmask (SIG_BLOCK, &fatal_signal_set, (sigset_t *) 0);
#else
#ifdef	HAVE_SIGSETMASK
  (void) sigblock (fatal_signal_mask);
#endif
#endif
}

#ifdef	POSIX
void
unblock_sigs ()
{
  sigset_t empty;
  sigemptyset (&empty);
  sigprocmask (SIG_SETMASK, &empty, (sigset_t *) 0);
}
#endif

/* Start a job to run the commands specified in CHILD.
   CHILD is updated to reflect the commands and ID of the child process.

   NOTE: On return fatal signals are blocked!  The caller is responsible
   for calling `unblock_sigs', once the new child is safely on the chain so
   it can be cleaned up in the event of a fatal signal.  */

static void
start_job_command (child)
     register struct child *child;
{
#ifndef _AMIGA
  static int bad_stdin = -1;
#endif
  register char *p;
  int flags;
#ifdef VMS
  char *argv;
#else
  char **argv;
#endif

  /* Combine the flags parsed for the line itself with
     the flags specified globally for this target.  */
  flags = (child->file->command_flags
	   | child->file->cmds->lines_flags[child->command_line - 1]);

  p = child->command_ptr;
  child->noerror = flags & COMMANDS_NOERROR;

  while (*p != '\0')
    {
      if (*p == '@')
	flags |= COMMANDS_SILENT;
      else if (*p == '+')
	flags |= COMMANDS_RECURSE;
      else if (*p == '-')
	child->noerror = 1;
      else if (!isblank (*p) && *p != '+')
	break;
      ++p;
    }

  /* If -q was given, just say that updating `failed'.  The exit status of
     1 tells the user that -q is saying `something to do'; the exit status
     for a random error is 2.  */
  if (question_flag && !(flags & COMMANDS_RECURSE))
    {
      child->file->update_status = 1;
      notice_finished_file (child->file);
      return;
    }

  /* There may be some preceding whitespace left if there
     was nothing but a backslash on the first line.  */
  p = next_token (p);

  /* Figure out an argument list from this command line.  */

  {
    char *end = 0;
#ifdef VMS
    argv = p;
#else
    argv = construct_command_argv (p, &end, child->file);
#endif
    if (end == NULL)
      child->command_ptr = NULL;
    else
      {
	*end++ = '\0';
	child->command_ptr = end;
      }
  }

  if (touch_flag && !(flags & COMMANDS_RECURSE))
    {
      /* Go on to the next command.  It might be the recursive one.
	 We construct ARGV only to find the end of the command line.  */
#ifndef VMS
      free (argv[0]);
      free ((char *) argv);
#endif
      argv = 0;
    }

  if (argv == 0)
    {
    next_command:
      /* This line has no commands.  Go to the next.  */
      if (job_next_command (child))
	start_job_command (child);
      else
	{
	  /* No more commands.  All done.  */
	  child->file->update_status = 0;
	  notice_finished_file (child->file);
	}
      return;
    }

  /* Print out the command.  If silent, we call `message' with null so it
     can log the working directory before the command's own error messages
     appear.  */

  message (0, (just_print_flag || (!(flags & COMMANDS_SILENT) && !silent_flag))
	   ? "%s" : (char *) 0, p);

  /* Optimize an empty command.  People use this for timestamp rules,
     and forking a useless shell all the time leads to inefficiency. */

#if !defined(VMS) && !defined(_AMIGA)
  if (
#ifdef __MSDOS__
      unixy_shell	/* the test is complicated and we already did it */
#else
      (argv[0] && !strcmp(argv[0], "/bin/sh"))
#endif
      && (argv[1]   && !strcmp(argv[1], "-c"))
      && (argv[2]   && !strcmp(argv[2], ":"))
      && argv[3] == NULL)
    {
      set_command_state (child->file, cs_running);
      goto next_command;
    }
#endif  /* !VMS && !_AMIGA */

  /* Tell update_goal_chain that a command has been started on behalf of
     this target.  It is important that this happens here and not in
     reap_children (where we used to do it), because reap_children might be
     reaping children from a different target.  We want this increment to
     guaranteedly indicate that a command was started for the dependency
     chain (i.e., update_file recursion chain) we are processing.  */

  ++commands_started;

  /* If -n was given, recurse to get the next line in the sequence.  */

  if (just_print_flag && !(flags & COMMANDS_RECURSE))
    {
#ifndef VMS
      free (argv[0]);
      free ((char *) argv);
#endif
      goto next_command;
    }

  /* Flush the output streams so they won't have things written twice.  */

  fflush (stdout);
  fflush (stderr);

#ifndef VMS
#if !defined(WINDOWS32) && !defined(_AMIGA) && !defined(__MSDOS__)

  /* Set up a bad standard input that reads from a broken pipe.  */

  if (bad_stdin == -1)
    {
      /* Make a file descriptor that is the read end of a broken pipe.
	 This will be used for some children's standard inputs.  */
      int pd[2];
      if (pipe (pd) == 0)
	{
	  /* Close the write side.  */
	  (void) close (pd[1]);
	  /* Save the read side.  */
	  bad_stdin = pd[0];

	  /* Set the descriptor to close on exec, so it does not litter any
	     child's descriptor table.  When it is dup2'd onto descriptor 0,
	     that descriptor will not close on exec.  */
#ifdef FD_SETFD
#ifndef FD_CLOEXEC
#define FD_CLOEXEC 1
#endif
	  (void) fcntl (bad_stdin, F_SETFD, FD_CLOEXEC);
#endif
	}
    }

#endif /* !WINDOWS32 && !_AMIGA && !__MSDOS__ */

  /* Decide whether to give this child the `good' standard input
     (one that points to the terminal or whatever), or the `bad' one
     that points to the read side of a broken pipe.  */

  child->good_stdin = !good_stdin_used;
  if (child->good_stdin)
    good_stdin_used = 1;

#endif /* !VMS */

  child->deleted = 0;

#ifndef _AMIGA
  /* Set up the environment for the child.  */
  if (child->environment == 0)
    child->environment = target_environment (child->file);
#endif

#if !defined(__MSDOS__) && !defined(_AMIGA) && !defined(WINDOWS32)

#ifndef VMS
  /* start_waiting_job has set CHILD->remote if we can start a remote job.  */
  if (child->remote)
    {
      int is_remote, id, used_stdin;
      if (start_remote_job (argv, child->environment,
			    child->good_stdin ? 0 : bad_stdin,
			    &is_remote, &id, &used_stdin))
	goto error;
      else
	{
	  if (child->good_stdin && !used_stdin)
	    {
	      child->good_stdin = 0;
	      good_stdin_used = 0;
	    }
	  child->remote = is_remote;
	  child->pid = id;
	}
    }
  else
#endif /* !VMS */
    {
      /* Fork the child process.  */

      char **parent_environ;

      block_sigs ();

      child->remote = 0;

#ifdef VMS

      if (!child_execute_job (argv, child)) {
        /* Fork failed!  */
        perror_with_name ("vfork", "");
        goto error;
      }

#else

      parent_environ = environ;
      child->pid = vfork ();
      environ = parent_environ;	/* Restore value child may have clobbered.  */
      if (child->pid == 0)
	{
	  /* We are the child side.  */
	  unblock_sigs ();
	  child_execute_job (child->good_stdin ? 0 : bad_stdin, 1,
			     argv, child->environment);
	}
      else if (child->pid < 0)
	{
	  /* Fork failed!  */
	  unblock_sigs ();
	  perror_with_name ("vfork", "");
	  goto error;
	}
#endif /* !VMS */
    }

#else	/* __MSDOS__ or Amiga or WINDOWS32 */
#ifdef __MSDOS__
  {
    int proc_return;

    block_sigs ();
    dos_status = 0;

    /* We call `system' to do the job of the SHELL, since stock DOS
       shell is too dumb.  Our `system' knows how to handle long
       command lines even if pipes/redirection is needed; it will only
       call COMMAND.COM when its internal commands are used.  */
    if (execute_by_shell)
      {
	char *cmdline = argv[0];
	/* We don't have a way to pass environment to `system',
	   so we need to save and restore ours, sigh...  */
	char **parent_environ = environ;

	environ = child->environment;

	/* If we have a *real* shell, tell `system' to call
	   it to do everything for us.  */
	if (unixy_shell)
	  {
	    /* A *real* shell on MSDOS may not support long
	       command lines the DJGPP way, so we must use `system'.  */
	    cmdline = argv[2];	/* get past "shell -c" */
	  }

	dos_command_running = 1;
	proc_return = system (cmdline);
	dos_command_running = 0;
	environ = parent_environ;
	execute_by_shell = 0;	/* for the next time */
      }
    else
      {
	dos_command_running = 1;
	proc_return = spawnvpe (P_WAIT, argv[0], argv, child->environment);
	dos_command_running = 0;
      }

    if (proc_return == -1)
      dos_status |= 0xff;
    else
      dos_status |= (proc_return & 0xff);
    ++dead_children;
    child->pid = dos_pid++;
  }
#endif /* __MSDOS__ */
#ifdef _AMIGA
  amiga_status = MyExecute (argv);

  ++dead_children;
  child->pid = amiga_pid++;
  if (amiga_batch_file)
  {
     amiga_batch_file = 0;
     DeleteFile (amiga_bname);        /* Ignore errors.  */
  }
#endif	/* Amiga */
#ifdef WINDOWS32
  {
      HANDLE hPID;
      char* arg0;

      /* make UNC paths safe for CreateProcess -- backslash format */
      arg0 = argv[0];
      if (arg0 && arg0[0] == '/' && arg0[1] == '/')
        for ( ; arg0 && *arg0; arg0++)
          if (*arg0 == '/')
            *arg0 = '\\';

      /* make sure CreateProcess() has Path it needs */
      sync_Path_environment();

      hPID = process_easy(argv, child->environment);

      if (hPID != INVALID_HANDLE_VALUE)
        child->pid = (int) hPID;
      else {
        int i;
        unblock_sigs();
        fprintf(stderr,
          "process_easy() failed failed to launch process (e=%d)\n",
          process_last_err(hPID));
               for (i = 0; argv[i]; i++)
                 fprintf(stderr, "%s ", argv[i]);
               fprintf(stderr, "\nCounted %d args in failed launch\n", i);
      }
  }
#endif /* WINDOWS32 */
#endif	/* __MSDOS__ or Amiga or WINDOWS32 */

  /* We are the parent side.  Set the state to
     say the commands are running and return.  */

  set_command_state (child->file, cs_running);

  /* Free the storage used by the child's argument list.  */
#ifndef VMS
  free (argv[0]);
  free ((char *) argv);
#endif

  return;

 error:
  child->file->update_status = 2;
  notice_finished_file (child->file);
  return;
}

/* Try to start a child running.
   Returns nonzero if the child was started (and maybe finished), or zero if
   the load was too high and the child was put on the `waiting_jobs' chain.  */

static int
start_waiting_job (c)
     struct child *c;
{
  /* If we can start a job remotely, we always want to, and don't care about
     the local load average.  We record that the job should be started
     remotely in C->remote for start_job_command to test.  */

  c->remote = start_remote_job_p ();

  /* If this job is to be started locally, and we are already running
     some jobs, make this one wait if the load average is too high.  */
  if (!c->remote && job_slots_used > 0 && load_too_high ())
    {
      /* Put this child on the chain of children waiting
	 for the load average to go down.  */
      set_command_state (c->file, cs_running);
      c->next = waiting_jobs;
      waiting_jobs = c;
      return 0;
    }

  /* Start the first command; reap_children will run later command lines.  */
  start_job_command (c);

  switch (c->file->command_state)
    {
    case cs_running:
      c->next = children;
      if (debug_flag)
	printf ("Putting child 0x%08lx PID %05d%s on the chain.\n",
		(unsigned long int) c,
		c->pid, c->remote ? " (remote)" : "");
      children = c;
      /* One more job slot is in use.  */
      ++job_slots_used;
      unblock_sigs ();
      break;

    case cs_not_started:
      /* All the command lines turned out to be empty.  */
      c->file->update_status = 0;
      /* FALLTHROUGH */

    case cs_finished:
      notice_finished_file (c->file);
      free_child (c);
      break;

    default:
      assert (c->file->command_state == cs_finished);
      break;
    }

  return 1;
}

/* Create a `struct child' for FILE and start its commands running.  */

void
new_job (file)
     register struct file *file;
{
  register struct commands *cmds = file->cmds;
  register struct child *c;
  char **lines;
  register unsigned int i;

  /* Let any previously decided-upon jobs that are waiting
     for the load to go down start before this new one.  */
  start_waiting_jobs ();

  /* Reap any children that might have finished recently.  */
  reap_children (0, 0);

  /* Chop the commands up into lines if they aren't already.  */
  chop_commands (cmds);

  if (job_slots != 0)
    /* Wait for a job slot to be freed up.  */
    while (job_slots_used == job_slots)
      reap_children (1, 0);

  /* Expand the command lines and store the results in LINES.  */
  lines = (char **) xmalloc (cmds->ncommand_lines * sizeof (char *));
  for (i = 0; i < cmds->ncommand_lines; ++i)
    {
      /* Collapse backslash-newline combinations that are inside variable
	 or function references.  These are left alone by the parser so
	 that they will appear in the echoing of commands (where they look
	 nice); and collapsed by construct_command_argv when it tokenizes.
	 But letting them survive inside function invocations loses because
	 we don't want the functions to see them as part of the text.  */

      char *in, *out, *ref;

      /* IN points to where in the line we are scanning.
	 OUT points to where in the line we are writing.
	 When we collapse a backslash-newline combination,
	 IN gets ahead of OUT.  */

      in = out = cmds->command_lines[i];
      while ((ref = index (in, '$')) != 0)
	{
	  ++ref;		/* Move past the $.  */

	  if (out != in)
	    /* Copy the text between the end of the last chunk
	       we processed (where IN points) and the new chunk
	       we are about to process (where REF points).  */
	    bcopy (in, out, ref - in);

	  /* Move both pointers past the boring stuff.  */
	  out += ref - in;
	  in = ref;

	  if (*ref == '(' || *ref == '{')
	    {
	      char openparen = *ref;
	      char closeparen = openparen == '(' ? ')' : '}';
	      int count;
	      char *p;

	      *out++ = *in++;	/* Copy OPENPAREN.  */
	      /* IN now points past the opening paren or brace.
		 Count parens or braces until it is matched.  */
	      count = 0;
	      while (*in != '\0')
		{
		  if (*in == closeparen && --count < 0)
		    break;
		  else if (*in == '\\' && in[1] == '\n')
		    {
		      /* We have found a backslash-newline inside a
			 variable or function reference.  Eat it and
			 any following whitespace.  */

		      int quoted = 0;
		      for (p = in - 1; p > ref && *p == '\\'; --p)
			quoted = !quoted;

		      if (quoted)
			/* There were two or more backslashes, so this is
			   not really a continuation line.  We don't collapse
			   the quoting backslashes here as is done in
			   collapse_continuations, because the line will
			   be collapsed again after expansion.  */
			*out++ = *in++;
		      else
			{
			  /* Skip the backslash, newline and
			     any following whitespace.  */
			  in = next_token (in + 2);

			  /* Discard any preceding whitespace that has
			     already been written to the output.  */
			  while (out > ref && isblank (out[-1]))
			    --out;

			  /* Replace it all with a single space.  */
			  *out++ = ' ';
			}
		    }
		  else
		    {
		      if (*in == openparen)
			++count;

		      *out++ = *in++;
		    }
		}
	    }
	}

      /* There are no more references in this line to worry about.
	 Copy the remaining uninteresting text to the output.  */
      if (out != in)
	strcpy (out, in);

      /* Finally, expand the line.  */
      lines[i] = allocated_variable_expand_for_file (cmds->command_lines[i],
						     file);
    }

  /* Start the command sequence, record it in a new
     `struct child', and add that to the chain.  */

  c = (struct child *) xmalloc (sizeof (struct child));
  c->file = file;
  c->command_lines = lines;
  c->command_line = 0;
  c->command_ptr = 0;
  c->environment = 0;

  /* Fetch the first command line to be run.  */
  job_next_command (c);

  /* The job is now primed.  Start it running.
     (This will notice if there are in fact no commands.)  */
  (void)start_waiting_job (c);

  if (job_slots == 1)
    /* Since there is only one job slot, make things run linearly.
       Wait for the child to die, setting the state to `cs_finished'.  */
    while (file->command_state == cs_running)
      reap_children (1, 0);

  return;
}

/* Move CHILD's pointers to the next command for it to execute.
   Returns nonzero if there is another command.  */

static int
job_next_command (child)
     struct child *child;
{
  while (child->command_ptr == 0 || *child->command_ptr == '\0')
    {
      /* There are no more lines in the expansion of this line.  */
      if (child->command_line == child->file->cmds->ncommand_lines)
	{
	  /* There are no more lines to be expanded.  */
	  child->command_ptr = 0;
	  return 0;
	}
      else
	/* Get the next line to run.  */
	child->command_ptr = child->command_lines[child->command_line++];
    }
  return 1;
}

static int
load_too_high ()
{
#if defined(__MSDOS__) || defined(VMS) || defined(_AMIGA)
  return 1;
#else
  double load;

  if (max_load_average < 0)
    return 0;

  make_access ();
  if (getloadavg (&load, 1) != 1)
    {
      static int lossage = -1;
      /* Complain only once for the same error.  */
      if (lossage == -1 || errno != lossage)
	{
	  if (errno == 0)
	    /* An errno value of zero means getloadavg is just unsupported.  */
	    error ("cannot enforce load limits on this operating system");
	  else
	    perror_with_name ("cannot enforce load limit: ", "getloadavg");
	}
      lossage = errno;
      load = 0;
    }
  user_access ();

  return load >= max_load_average;
#endif
}

/* Start jobs that are waiting for the load to be lower.  */

void
start_waiting_jobs ()
{
  struct child *job;

  if (waiting_jobs == 0)
    return;

  do
    {
      /* Check for recently deceased descendants.  */
      reap_children (0, 0);

      /* Take a job off the waiting list.  */
      job = waiting_jobs;
      waiting_jobs = job->next;

      /* Try to start that job.  We break out of the loop as soon
	 as start_waiting_job puts one back on the waiting list.  */
    }
  while (start_waiting_job (job) && waiting_jobs != 0);

  return;
}

#ifndef WINDOWS32
#ifdef VMS
#include <descrip.h>
#include <clidef.h>

/* This is called as an AST when a child process dies (it won't get
   interrupted by anything except a higher level AST).
*/
int vmsHandleChildTerm(struct child *child)
{
    int status;
    register struct child *lastc, *c;
    int child_failed;

    vms_jobsefnmask &= ~(1 << (child->efn - 32));

    lib$free_ef(&child->efn);

    (void) sigblock (fatal_signal_mask);

    child_failed = !(child->cstatus & 1 || ((child->cstatus & 7) == 0));

    /* Search for a child matching the deceased one.  */
    lastc = 0;
#if defined(RECURSIVEJOBS) /* I've had problems with recursive stuff and process handling */
    for (c = children; c != 0 && c != child; lastc = c, c = c->next);
#else
    c = child;
#endif

    if (child_failed && !c->noerror && !ignore_errors_flag)
      {
	/* The commands failed.  Write an error message,
	   delete non-precious targets, and abort.  */
	child_error (c->file->name, c->cstatus, 0, 0, 0);
	c->file->update_status = 1;
	delete_child_targets (c);
      }
    else
      {
	if (child_failed)
	  {
	    /* The commands failed, but we don't care.  */
	    child_error (c->file->name, c->cstatus, 0, 0, 1);
	    child_failed = 0;
	  }

#if defined(RECURSIVEJOBS) /* I've had problems with recursive stuff and process handling */
	/* If there are more commands to run, try to start them.  */
	start_job (c);

	switch (c->file->command_state)
	  {
	  case cs_running:
	    /* Successfully started.  */
	    break;

	  case cs_finished:
	    if (c->file->update_status != 0) {
		/* We failed to start the commands.  */
		delete_child_targets (c);
	    }
	    break;

	  default:
	    error ("internal error: `%s' command_state \
%d in child_handler", c->file->name);
	    abort ();
	    break;
	  }
#endif /* RECURSIVEJOBS */
      }

    /* Set the state flag to say the commands have finished.  */
    c->file->command_state = cs_finished;
    notice_finished_file (c->file);

#if defined(RECURSIVEJOBS) /* I've had problems with recursive stuff and process handling */
    /* Remove the child from the chain and free it.  */
    if (lastc == 0)
      children = c->next;
    else
      lastc->next = c->next;
    free_child (c);
#endif /* RECURSIVEJOBS */

    /* There is now another slot open.  */
    if (job_slots_used > 0)
      --job_slots_used;

    /* If the job failed, and the -k flag was not given, die.  */
    if (child_failed && !keep_going_flag)
      die (EXIT_FAILURE);

    (void) sigsetmask (sigblock (0) & ~(fatal_signal_mask));

    return 1;
}

/* VMS:
   Spawn a process executing the command in ARGV and return its pid. */

#define MAXCMDLEN 200

int
child_execute_job (argv, child)
     char *argv;
     struct child *child;
{
  int i;
  static struct dsc$descriptor_s cmddsc;
#ifndef DONTWAITFORCHILD
  int spflags = 0;
#else
  int spflags = CLI$M_NOWAIT;
#endif
  int status;
  char cmd[4096],*p,*c;
  char comname[50];

/* Remove backslashes */
  for (p = argv, c = cmd; *p; p++,c++)
    {
      if (*p == '\\') p++;
	*c = *p;
    }
  *c = *p;

  /* check for maximum dcl length and create *.com file if neccesary */

  comname[0] = '\0';

  if (strlen (cmd) > MAXCMDLEN)
    {
      FILE *outfile;
      char tmp;

      strcpy (comname, "sys$scratch:CMDXXXXXX.COM");
      (void) mktemp (comname);

      outfile = fopen (comname, "w");
      if (outfile == 0)
	pfatal_with_name (comname);

      fprintf (outfile, "$ ");
      c = cmd;

      while (c)
	{
	  p = strchr (c, ',');
	  if ((p == NULL) || (p-c > MAXCMDLEN))
	    p = strchr (c, ' ');
	  if (p != NULL)
	    {
	      p++;
	      tmp = *p;
	      *p = '\0';
	    }
	  else
	    tmp = '\0';
	  fprintf (outfile, "%s%s\n", c, (tmp == '\0')?"":" -");
	  if (p != NULL)
	    *p = tmp;
	  c = p;
	}

      fclose (outfile);

      sprintf (cmd, "$ @%s", comname);

      if (debug_flag)
	printf ("Executing %s instead\n", cmd);
    }

  cmddsc.dsc$w_length = strlen(cmd);
  cmddsc.dsc$a_pointer = cmd;
  cmddsc.dsc$b_dtype = DSC$K_DTYPE_T;
  cmddsc.dsc$b_class = DSC$K_CLASS_S;

  child->efn = 0;
  while (child->efn < 32 || child->efn > 63)
    {
      status = lib$get_ef(&child->efn);
      if (!(status & 1))
	return 0;
    }

  sys$clref(child->efn);

  vms_jobsefnmask |= (1 << (child->efn - 32));

#ifndef DONTWAITFORCHILD
  status = lib$spawn(&cmddsc,0,0,&spflags,0,&child->pid,&child->cstatus,
		       &child->efn,0,0);
  vmsHandleChildTerm(child);
#else
  status = lib$spawn(&cmddsc,0,0,&spflags,0,&child->pid,&child->cstatus,
		       &child->efn,vmsHandleChildTerm,child);
#endif

  if (!(status & 1))
    {
      printf("Error spawning, %d\n",status);
      fflush(stdout);
    }

  unlink (comname);

  return (status & 1);
}

#else /* !VMS */

#if !defined (_AMIGA) && !defined (__MSDOS__)
/* UNIX:
   Replace the current process with one executing the command in ARGV.
   STDIN_FD and STDOUT_FD are used as the process's stdin and stdout; ENVP is
   the environment of the new program.  This function does not return.  */

void
child_execute_job (stdin_fd, stdout_fd, argv, envp)
     int stdin_fd, stdout_fd;
     char **argv, **envp;
{
  if (stdin_fd != 0)
    (void) dup2 (stdin_fd, 0);
  if (stdout_fd != 1)
    (void) dup2 (stdout_fd, 1);
  if (stdin_fd != 0)
    (void) close (stdin_fd);
  if (stdout_fd != 1)
    (void) close (stdout_fd);

  /* Run the command.  */
  exec_command (argv, envp);
}
#endif /* !AMIGA && !__MSDOS__ */
#endif /* !VMS */
#endif /* !WINDOWS32 */

#ifndef _AMIGA
/* Replace the current process with one running the command in ARGV,
   with environment ENVP.  This function does not return.  */

void
exec_command (argv, envp)
     char **argv, **envp;
{
#ifdef VMS
  /* Run the program.  */
  execve (argv[0], argv, envp);
  perror_with_name ("execve: ", argv[0]);
  _exit (EXIT_FAILURE);
#else
#ifdef WINDOWS32
  HANDLE hPID;
  HANDLE hWaitPID;
  int err = 0;
  int exit_code = EXIT_FAILURE;

  /* make sure CreateProcess() has Path it needs */
  sync_Path_environment();

  /* launch command */
  hPID = process_easy(argv, envp);

  /* make sure launch ok */
  if (hPID == INVALID_HANDLE_VALUE)
    {
      int i;
      fprintf(stderr,
              "process_easy() failed failed to launch process (e=%d)\n",
              process_last_err(hPID));
      for (i = 0; argv[i]; i++)
          fprintf(stderr, "%s ", argv[i]);
      fprintf(stderr, "\nCounted %d args in failed launch\n", i);
      exit(EXIT_FAILURE);
    }

  /* wait and reap last child */
  while (hWaitPID = process_wait_for_any())
    {
      /* was an error found on this process? */
      err = process_last_err(hWaitPID);

      /* get exit data */
      exit_code = process_exit_code(hWaitPID);

      if (err)
          fprintf(stderr, "make (e=%d, rc=%d): %s",
                  err, exit_code, map_windows32_error_to_string(err));

      /* cleanup process */
      process_cleanup(hWaitPID);

      /* expect to find only last pid, warn about other pids reaped */
      if (hWaitPID == hPID)
          break;
      else
          fprintf(stderr,
                  "make reaped child pid %d, still waiting for pid %d\n",
                  hWaitPID, hPID);
    }

  /* return child's exit code as our exit code */
  exit(exit_code);

#else  /* !WINDOWS32 */

  /* Be the user, permanently.  */
  child_access ();

  /* Run the program.  */
  environ = envp;
  execvp (argv[0], argv);

  switch (errno)
    {
    case ENOENT:
      error ("%s: Command not found", argv[0]);
      break;
    case ENOEXEC:
      {
	/* The file is not executable.  Try it as a shell script.  */
	extern char *getenv ();
	char *shell;
	char **new_argv;
	int argc;

	shell = getenv ("SHELL");
	if (shell == 0)
	  shell = default_shell;

	argc = 1;
	while (argv[argc] != 0)
	  ++argc;

	new_argv = (char **) alloca ((1 + argc + 1) * sizeof (char *));
	new_argv[0] = shell;
	new_argv[1] = argv[0];
	while (argc > 0)
	  {
	    new_argv[1 + argc] = argv[argc];
	    --argc;
	  }

	execvp (shell, new_argv);
	if (errno == ENOENT)
	  error ("%s: Shell program not found", shell);
	else
	  perror_with_name ("execvp: ", shell);
	break;
      }

    default:
      perror_with_name ("execvp: ", argv[0]);
      break;
    }

  _exit (127);
#endif /* !WINDOWS32 */
#endif /* !VMS */
}
#else /* On Amiga */
void exec_command (argv)
     char **argv;
{
  MyExecute (argv);
}

void clean_tmp (void)
{
  DeleteFile (amiga_bname);
}

#endif /* On Amiga */

#ifndef VMS
/* Figure out the argument list necessary to run LINE as a command.  Try to
   avoid using a shell.  This routine handles only ' quoting, and " quoting
   when no backslash, $ or ` characters are seen in the quotes.  Starting
   quotes may be escaped with a backslash.  If any of the characters in
   sh_chars[] is seen, or any of the builtin commands listed in sh_cmds[]
   is the first word of a line, the shell is used.

   If RESTP is not NULL, *RESTP is set to point to the first newline in LINE.
   If *RESTP is NULL, newlines will be ignored.

   SHELL is the shell to use, or nil to use the default shell.
   IFS is the value of $IFS, or nil (meaning the default).  */

static char **
construct_command_argv_internal (line, restp, shell, ifs)
     char *line, **restp;
     char *shell, *ifs;
{
#ifdef __MSDOS__
  /* MSDOS supports both the stock DOS shell and ports of Unixy shells.
     We call `system' for anything that requires ``slow'' processing,
     because DOS shells are too dumb.  When $SHELL points to a real
     (unix-style) shell, `system' just calls it to do everything.  When
     $SHELL points to a DOS shell, `system' does most of the work
     internally, calling the shell only for its internal commands.
     However, it looks on the $PATH first, so you can e.g. have an
     external command named `mkdir'.

     Since we call `system', certain characters and commands below are
     actually not specific to COMMAND.COM, but to the DJGPP implementation
     of `system'.  In particular:

       The shell wildcard characters are in DOS_CHARS because they will
       not be expanded if we call the child via `spawnXX'.

       The `;' is in DOS_CHARS, because our `system' knows how to run
       multiple commands on a single line.

       DOS_CHARS also include characters special to 4DOS/NDOS, so we
       won't have to tell one from another and have one more set of
       commands and special characters.  */
  static char sh_chars_dos[] = "*?[];|<>%^&()";
  static char *sh_cmds_dos[] = { "break", "call", "cd", "chcp", "chdir", "cls",
				 "copy", "ctty", "date", "del", "dir", "echo",
				 "erase", "exit", "for", "goto", "if", "md",
				 "mkdir", "path", "pause", "prompt", "rd",
				 "rmdir", "rem", "ren", "rename", "set",
				 "shift", "time", "type", "ver", "verify",
				 "vol", ":", 0 };

  static char sh_chars_sh[]  = "#;\"*?[]&|<>(){}$`^";
  static char *sh_cmds_sh[]  = { "cd", "eval", "exec", "exit", "login",
				 "logout", "set", "umask", "wait", "while",
				 "for", "case", "if", ":", ".", "break",
				 "continue", "export", "read", "readonly",
				 "shift", "times", "trap", "switch", 0 };

  char *sh_chars;
  char **sh_cmds;
#else
#ifdef _AMIGA
  static char sh_chars[] = "#;\"|<>()?*$`";
  static char *sh_cmds[] = { "cd", "eval", "if", "delete", "echo", "copy",
			     "rename", "set", "setenv", "date", "makedir",
			     "skip", "else", "endif", "path", "prompt",
			     "unset", "unsetenv", "version",
			     0 };
#else
#ifdef WINDOWS32
  static char sh_chars_dos[] = "\"|<>";
  static char *sh_cmds_dos[] = { "break", "call", "cd", "chcp", "chdir", "cls",
			     "copy", "ctty", "date", "del", "dir", "echo",
			     "erase", "exit", "for", "goto", "if", "if", "md",
			     "mkdir", "path", "pause", "prompt", "rem", "ren",
			     "rename", "set", "shift", "time", "type",
			     "ver", "verify", "vol", ":", 0 };
  static char sh_chars_sh[] = "#;\"*?[]&|<>(){}$`^";
  static char *sh_cmds_sh[] = { "cd", "eval", "exec", "exit", "login",
			     "logout", "set", "umask", "wait", "while", "for",
			     "case", "if", ":", ".", "break", "continue",
			     "export", "read", "readonly", "shift", "times",
			     "trap", "switch", "test", 0 };
  char*  sh_chars;
  char** sh_cmds;
#else  /* WINDOWS32 */
  static char sh_chars[] = "#;\"*?[]&|<>(){}$`^";
  static char *sh_cmds[] = { "cd", "eval", "exec", "exit", "login",
			     "logout", "set", "umask", "wait", "while", "for",
			     "case", "if", ":", ".", "break", "continue",
			     "export", "read", "readonly", "shift", "times",
			     "trap", "switch", 0 };
#endif /* WINDOWS32 */
#endif /* Amiga */
#endif /* __MSDOS__ */
  register int i;
  register char *p;
  register char *ap;
  char *end;
  int instring, word_has_equals, seen_nonequals, last_argument_was_empty;
  char **new_argv = 0;
#ifdef WINDOWS32
  int slow_flag = 0;

  if (no_default_sh_exe) {
    sh_cmds = sh_cmds_dos;
    sh_chars = sh_chars_dos;
  } else {
    sh_cmds = sh_cmds_sh;
    sh_chars = sh_chars_sh;
  }
#endif /* WINDOWS32 */

  if (restp != NULL)
    *restp = NULL;

  /* Make sure not to bother processing an empty line.  */
  while (isblank (*line))
    ++line;
  if (*line == '\0')
    return 0;

  /* See if it is safe to parse commands internally.  */
  if (shell == 0)
    shell = default_shell;
#ifdef WINDOWS32
  else if (strcmp (shell, default_shell))
  {
    char *s1 = _fullpath(NULL, shell, 0);
    char *s2 = _fullpath(NULL, default_shell, 0);

    slow_flag = strcmp((s1 ? s1 : ""), (s2 ? s2 : ""));

    if (s1);
      free(s1);
    if (s2);
      free(s2);
  }
  if (slow_flag)
    goto slow;
#else  /* not WINDOWS32 */
#ifdef __MSDOS__
  else if (stricmp (shell, default_shell))
    {
      extern int _is_unixy_shell (const char *_path);

      message (1, "$SHELL changed (was `%s', now `%s')", default_shell, shell);
      unixy_shell = _is_unixy_shell (shell);
      default_shell = shell;
    }
  if (unixy_shell)
    {
      sh_chars = sh_chars_sh;
      sh_cmds  = sh_cmds_sh;
    }
  else
    {
      sh_chars = sh_chars_dos;
      sh_cmds  = sh_cmds_dos;
    }
#else  /* not __MSDOS__ */
  else if (strcmp (shell, default_shell))
    goto slow;
#endif /* not __MSDOS__ */
#endif /* not WINDOWS32 */

  if (ifs != 0)
    for (ap = ifs; *ap != '\0'; ++ap)
      if (*ap != ' ' && *ap != '\t' && *ap != '\n')
	goto slow;

  i = strlen (line) + 1;

  /* More than 1 arg per character is impossible.  */
  new_argv = (char **) xmalloc (i * sizeof (char *));

  /* All the args can fit in a buffer as big as LINE is.   */
  ap = new_argv[0] = (char *) xmalloc (i);
  end = ap + i;

  /* I is how many complete arguments have been found.  */
  i = 0;
  instring = word_has_equals = seen_nonequals = last_argument_was_empty = 0;
  for (p = line; *p != '\0'; ++p)
    {
      if (ap > end)
	abort ();

      if (instring)
	{
	string_char:
	  /* Inside a string, just copy any char except a closing quote
	     or a backslash-newline combination.  */
	  if (*p == instring)
	    {
	      instring = 0;
	      if (ap == new_argv[0] || *(ap-1) == '\0')
		last_argument_was_empty = 1;
	    }
	  else if (*p == '\\' && p[1] == '\n')
	    goto swallow_escaped_newline;
	  else if (*p == '\n' && restp != NULL)
	    {
	      /* End of the command line.  */
	      *restp = p;
	      goto end_of_line;
	    }
	  /* Backslash, $, and ` are special inside double quotes.
	     If we see any of those, punt.
	     But on MSDOS, if we use COMMAND.COM, double and single
	     quotes have the same effect.  */
	  else if (instring == '"' && index ("\\$`", *p) != 0 && unixy_shell)
	    goto slow;
	  else
	    *ap++ = *p;
	}
      else if (index (sh_chars, *p) != 0)
	/* Not inside a string, but it's a special char.  */
	goto slow;
#ifdef  __MSDOS__
      else if (*p == '.' && p[1] == '.' && p[2] == '.' && p[3] != '.')
	/* `...' is a wildcard in DJGPP.  */
	goto slow;
#endif
      else
	/* Not a special char.  */
	switch (*p)
	  {
	  case '=':
	    /* Equals is a special character in leading words before the
	       first word with no equals sign in it.  This is not the case
	       with sh -k, but we never get here when using nonstandard
	       shell flags.  */
	    if (! seen_nonequals && unixy_shell)
	      goto slow;
	    word_has_equals = 1;
	    *ap++ = '=';
	    break;

	  case '\\':
	    /* Backslash-newline combinations are eaten.  */
	    if (p[1] == '\n')
	      {
	      swallow_escaped_newline:

		/* Eat the backslash, the newline, and following whitespace,
		   replacing it all with a single space.  */
		p += 2;

		/* If there is a tab after a backslash-newline,
		   remove it from the source line which will be echoed,
		   since it was most likely used to line
		   up the continued line with the previous one.  */
		if (*p == '\t')
		  strcpy (p, p + 1);

		if (instring)
		  goto string_char;
		else
		  {
		    if (ap != new_argv[i])
		      /* Treat this as a space, ending the arg.
			 But if it's at the beginning of the arg, it should
			 just get eaten, rather than becoming an empty arg. */
		      goto end_of_arg;
		    else
		      p = next_token (p) - 1;
		  }
	      }
	    else if (p[1] != '\0')
	      /* Copy and skip the following char.  */
	      *ap++ = *++p;
	    break;

	  case '\'':
	  case '"':
	    instring = *p;
	    break;

	  case '\n':
	    if (restp != NULL)
	      {
		/* End of the command line.  */
		*restp = p;
		goto end_of_line;
	      }
	    else
	      /* Newlines are not special.  */
	      *ap++ = '\n';
	    break;

	  case ' ':
	  case '\t':
	  end_of_arg:
	    /* We have the end of an argument.
	       Terminate the text of the argument.  */
	    *ap++ = '\0';
	    new_argv[++i] = ap;
	    last_argument_was_empty = 0;

	    /* Update SEEN_NONEQUALS, which tells us if every word
	       heretofore has contained an `='.  */
	    seen_nonequals |= ! word_has_equals;
	    if (word_has_equals && ! seen_nonequals)
	      /* An `=' in a word before the first
		 word without one is magical.  */
	      goto slow;
	    word_has_equals = 0; /* Prepare for the next word.  */

	    /* If this argument is the command name,
	       see if it is a built-in shell command.
	       If so, have the shell handle it.  */
	    if (i == 1)
	      {
		register int j;
		for (j = 0; sh_cmds[j] != 0; ++j)
		  if (streq (sh_cmds[j], new_argv[0]))
		    goto slow;
	      }

	    /* Ignore multiple whitespace chars.  */
	    p = next_token (p);
	    /* Next iteration should examine the first nonwhite char.  */
	    --p;
	    break;

	  default:
	    *ap++ = *p;
	    break;
	  }
    }
 end_of_line:

  if (instring)
    /* Let the shell deal with an unterminated quote.  */
    goto slow;

  /* Terminate the last argument and the argument list.  */

  *ap = '\0';
  if (new_argv[i][0] != '\0' || last_argument_was_empty)
    ++i;
  new_argv[i] = 0;

  if (i == 1)
    {
      register int j;
      for (j = 0; sh_cmds[j] != 0; ++j)
	if (streq (sh_cmds[j], new_argv[0]))
	  goto slow;
    }

  if (new_argv[0] == 0)
    /* Line was empty.  */
    return 0;
  else
    return new_argv;

 slow:;
  /* We must use the shell.  */

  if (new_argv != 0)
    {
      /* Free the old argument list we were working on.  */
      free (new_argv[0]);
      free ((void *)new_argv);
    }

#ifdef __MSDOS__
  execute_by_shell = 1;	/* actually, call `system' if shell isn't unixy */
#endif

#ifdef _AMIGA
  {
    char *ptr;
    char *buffer;
    char *dptr;

    buffer = (char *)xmalloc (strlen (line)+1);

    ptr = line;
    for (dptr=buffer; *ptr; )
    {
      if (*ptr == '\\' && ptr[1] == '\n')
	ptr += 2;
      else if (*ptr == '@') /* Kludge: multiline commands */
      {
	ptr += 2;
	*dptr++ = '\n';
      }
      else
	*dptr++ = *ptr++;
    }
    *dptr = 0;

    new_argv = (char **) xmalloc(2 * sizeof(char *));
    new_argv[0] = buffer;
    new_argv[1] = 0;
  }
#else	/* Not Amiga  */
#ifdef WINDOWS32
  /*
   * Not eating this whitespace caused things like
   *
   *    sh -c "\n"
   *
   * which gave the shell fits. I think we have to eat
   * whitespace here, but this code should be considered
   * suspicious if things start failing....
   */

  /* Make sure not to bother processing an empty line.  */
  while (isspace (*line))
    ++line;
  if (*line == '\0')
    return 0;

  /*
   * only come here if no sh.exe command
   */
  if (no_default_sh_exe)
  {
    FILE *batch;
    dos_batch_file = 1;
    if (dos_bname == 0)
    {
      dos_bname = tempnam (".", "mk");
      for (i = 0; dos_bname[i] != '\0'; ++i)
	if (dos_bname[i] == '/')
	  dos_bname[i] = '\\';
      dos_bename = (char *) xmalloc (strlen (dos_bname) + 5);
      strcpy (dos_bename, dos_bname);
      strcat (dos_bname, ".bat");
      strcat (dos_bename, ".err");
    }
    batch = fopen (dos_bename, "w"); /* Create a file.  */
    if (batch != NULL)
      fclose (batch);
    batch = fopen (dos_bname, "w");
    fputs ("@echo off\n", batch);
    fputs (line, batch);
    fprintf (batch, "\nif errorlevel 1 del %s\n", dos_bename);
    fclose (batch);
    new_argv = (char **) xmalloc(2 * sizeof(char *));
    new_argv[0] = strdup (dos_bname);
    new_argv[1] = 0;
  }
  else
#endif /* WINDOWS32 */
  {
    /* SHELL may be a multi-word command.  Construct a command line
       "SHELL -c LINE", with all special chars in LINE escaped.
       Then recurse, expanding this command line to get the final
       argument list.  */

    unsigned int shell_len = strlen (shell);
    static char minus_c[] = " -c ";
    unsigned int line_len = strlen (line);

    char *new_line = (char *) alloca (shell_len + (sizeof (minus_c) - 1)
				      + (line_len * 2) + 1);

    ap = new_line;
    bcopy (shell, ap, shell_len);
    ap += shell_len;
    bcopy (minus_c, ap, sizeof (minus_c) - 1);
    ap += sizeof (minus_c) - 1;
    for (p = line; *p != '\0'; ++p)
      {
	if (restp != NULL && *p == '\n')
	  {
	    *restp = p;
	    break;
	  }
	else if (*p == '\\' && p[1] == '\n')
	  {
	    /* Eat the backslash, the newline, and following whitespace,
	       replacing it all with a single space (which is escaped
	       from the shell).  */
	    p += 2;

	    /* If there is a tab after a backslash-newline,
	       remove it from the source line which will be echoed,
	       since it was most likely used to line
	       up the continued line with the previous one.  */
	    if (*p == '\t')
	      bcopy (p + 1, p, strlen (p));

	    p = next_token (p);
	    --p;
            if (unixy_shell)
              *ap++ = '\\';
	    *ap++ = ' ';
	    continue;
	  }

        /* DOS shells don't know about backslash-escaping.  */
	if (unixy_shell &&
            (*p == '\\' || *p == '\'' || *p == '"'
             || isspace (*p)
             || index (sh_chars, *p) != 0))
	  *ap++ = '\\';
#ifdef __MSDOS__
        else if (unixy_shell && strncmp (p, "...", 3) == 0)
          {
            /* The case of `...' wildcard again.  */
            strcpy (ap, "\\.\\.\\");
            ap += 5;
            p  += 2;
          }
#endif
	*ap++ = *p;
      }
    if (ap == new_line + shell_len + sizeof (minus_c) - 1)
      /* Line was empty.  */
      return 0;
    *ap = '\0';

    if (unixy_shell)
      new_argv = construct_command_argv_internal (new_line, (char **) NULL,
                                                  (char *) 0, (char *) 0);
#ifdef  __MSDOS__
    else
      {
      /* With MSDOS shells, we must construct the command line here
         instead of recursively calling ourselves, because we
         cannot backslash-escape the special characters (see above).  */
      new_argv = (char **) xmalloc (sizeof (char *));
      line_len = strlen (new_line) - shell_len - sizeof (minus_c) + 1;
      new_argv[0] = xmalloc (line_len + 1);
      strncpy (new_argv[0],
               new_line + shell_len + sizeof (minus_c) - 1, line_len);
      new_argv[0][line_len] = '\0';
      }
#endif
  }
#endif	/* ! AMIGA */

  return new_argv;
}

/* Figure out the argument list necessary to run LINE as a command.  Try to
   avoid using a shell.  This routine handles only ' quoting, and " quoting
   when no backslash, $ or ` characters are seen in the quotes.  Starting
   quotes may be escaped with a backslash.  If any of the characters in
   sh_chars[] is seen, or any of the builtin commands listed in sh_cmds[]
   is the first word of a line, the shell is used.

   If RESTP is not NULL, *RESTP is set to point to the first newline in LINE.
   If *RESTP is NULL, newlines will be ignored.

   FILE is the target whose commands these are.  It is used for
   variable expansion for $(SHELL) and $(IFS).  */

char **
construct_command_argv (line, restp, file)
     char *line, **restp;
     struct file *file;
{
  char *shell, *ifs;
  char **argv;

  {
    /* Turn off --warn-undefined-variables while we expand SHELL and IFS.  */
    int save = warn_undefined_variables_flag;
    warn_undefined_variables_flag = 0;

    shell = allocated_variable_expand_for_file ("$(SHELL)", file);
#ifdef WINDOWS32
    /*
     * Convert to forward slashes so that construct_command_argv_internal()
     * is not confused.
     */
    if (shell) {
      char *p = w32ify(shell, 0);
      strcpy(shell, p);
    }
#endif
    ifs = allocated_variable_expand_for_file ("$(IFS)", file);

    warn_undefined_variables_flag = save;
  }

  argv = construct_command_argv_internal (line, restp, shell, ifs);

  free (shell);
  free (ifs);

  return argv;
}
#endif /* !VMS */

#if !defined(HAVE_DUP2) && !defined(_AMIGA)
int
dup2 (old, new)
     int old, new;
{
  int fd;

  (void) close (new);
  fd = dup (old);
  if (fd != new)
    {
      (void) close (fd);
      errno = EMFILE;
      return -1;
    }

  return fd;
}
#endif /* !HAPE_DUP2 && !_AMIGA */
