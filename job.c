/* Job execution and handling for GNU Make.
Copyright (C) 1988, 1989, 1990, 1991, 1992, 1993 Free Software Foundation, Inc.
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
#include "commands.h"
#include "job.h"
#include "file.h"
#include "variable.h"

/* Default path to search for executables.  */
static char default_path[] = ":/bin:/usr/bin";

/* Default shell to use.  */
char default_shell[] = "/bin/sh";

/* If NGROUPS_MAX == 0 then try other methods for finding a real value.  */
#if defined (NGROUPS_MAX) && NGROUPS_MAX == 0
#undef NGROUPS_MAX
#endif /* NGROUPS_MAX == 0 */

#ifndef	NGROUPS_MAX
#ifdef	POSIX
#define	GET_NGROUPS_MAX	sysconf (_SC_NGROUPS_MAX)
#else	/* Not POSIX.  */
#define	NGROUPS_MAX	NGROUPS
#endif	/* POSIX.  */
#endif

#ifdef	HAVE_SYS_WAIT_H
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


#ifndef	HAVE_UNISTD_H
extern int dup2 ();
extern int execve ();
extern void _exit ();
extern int geteuid (), getegid ();
extern int setgid (), getgid ();
#endif

#ifndef	getdtablesize
#ifdef HAVE_GETDTABLESIZE
extern int getdtablesize ();
#else
#include <sys/param.h>
#define getdtablesize() NOFILE
#if !defined (NOFILE) && defined (NOFILES_MAX)
/* SCO 3.2 "devsys 4.2" defines NOFILES_{MIN,MAX} in lieu of NOFILE.  */
#define NOFILE	NOFILES_MAX
#endif
#endif
#endif

extern int getloadavg ();
extern int start_remote_job_p ();
extern int start_remote_job (), remote_status ();

RETSIGTYPE child_handler ();
static void free_child (), start_job_command ();
static int load_too_high (), job_next_command ();

/* Chain of all live (or recently deceased) children.  */

struct child *children = 0;

/* Number of children currently running.  */

unsigned int job_slots_used = 0;

/* Nonzero if the `good' standard input is in use.  */

static int good_stdin_used = 0;

/* Chain of children waiting to run until the load average goes down.  */

static struct child *waiting_jobs = 0;

/* Write an error message describing the exit status given in
   EXIT_CODE, EXIT_SIG, and COREDUMP, for the target TARGET_NAME.
   Append "(ignored)" if IGNORED is nonzero.  */

static void
child_error (target_name, exit_code, exit_sig, coredump, ignored)
     char *target_name;
     int exit_code, exit_sig, coredump;
     int ignored;
{
  if (exit_sig == 0)
    error (ignored ? "[%s] Error %d (ignored)" :
	   "*** [%s] Error %d",
	   target_name, exit_code);
  else
    {
      char *coredump_string = coredump ? " (core dumped)" : "";
      if (exit_sig > 0 && exit_sig < NSIG)
	error ("*** [%s] %s%s",
	       target_name, sys_siglist[exit_sig], coredump_string);
      else
	error ("*** [%s] Signal %d%s", target_name, exit_sig, coredump_string);
    }
}

static unsigned int dead_children = 0;

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

      if (dead_children != 0)
	--dead_children;

      any_remote = 0;
      any_local = shell_function_pid != -1;
      for (c = children; c != 0; c = c->next)
	{
	  any_remote |= c->remote;
	  any_local |= ! c->remote;
	  if (debug_flag)
	    printf ("Live child 0x%08lx PID %d%s\n",
		    (unsigned long int) c,
		    c->pid, c->remote ? " (remote)" : "");
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
	  /* No remote children.  Check for local children.  */

	  if (any_local)
	    {
#ifdef	WAIT_NOHANG
	      if (!block)
		pid = WAIT_NOHANG (&status);
	      else
#endif
		pid = wait (&status);
	    }
	  else
	    pid = 0;

	  if (pid < 0)
	    {
#ifdef	EINTR
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
	      child_error (c->file->name, exit_code, exit_sig, coredump, 0);
	      c->file->update_status = 1;
	      if (exit_sig != 0)
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
		      c->file->command_state = cs_finished;
		      c->file->update_status = 1;
		    }
		  else
		    {
		      /* Check again whether to start remotely.
			 Whether or not we want to changes over time.
			 Also, start_remote_job may need state set up
			 by start_remote_job_p.  */
		      c->remote = start_remote_job_p ();
		      start_job_command (c);
		    }
		}

	      switch (c->file->command_state)
		{
		case cs_running:
		  /* Successfully started.  Loop to reap more children.  */
		  continue;

		case cs_finished:
		  if (c->file->update_status != 0)
		    /* We failed to start the commands.  */
		    delete_child_targets (c);
		  break;

		default:
		  error ("internal error: `%s' has bogus command_state \
%d in reap_children",
			 c->file->name, (int) c->file->command_state);
		  abort ();
		  break;
		}
	    }

	  if (! handling_fatal_signal)
	    /* Notice if the target of the commands has been changed.  */
	    notice_finished_file (c->file);

	  if (debug_flag)
	    printf ("Removing child 0x%08lx PID %d%s from chain.\n",
		    (unsigned long int) c,
		    c->pid, c->remote ? " (remote)" : "");

	  /* Remove the child from the chain and free it.  */
	  if (lastc == 0)
	    children = c->next;
	  else
	    lastc->next = c->next;
	  if (! handling_fatal_signal) /* Avoid nonreentrancy.  */
	    free_child (c);

	  /* There is now another slot open.  */
	  --job_slots_used;

	  /* If the job failed, and the -k flag was not given, die,
	     unless we are already in the process of dying.  */
	  if (!err && child_failed && !keep_going_flag)
	    die (1);
	}

      /* Only block for one child.  */
      block = 0;
    }
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

#ifdef	POSIX
extern sigset_t fatal_signal_set;

void
unblock_sigs ()
{
  sigset_t empty;
  sigemptyset (&empty);
  sigprocmask (SIG_SETMASK, &empty, (sigset_t *) 0);
}
#endif

/* Start a job to run the commands specified in CHILD.
   CHILD is updated to reflect the commands and ID of the child process.  */

static void
start_job_command (child)
     register struct child *child;
{
  static int bad_stdin = -1;
  register char *p;
  int flags = child->file->cmds->lines_flags[child->command_line - 1];
  char **argv;

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

  /* If -q was given, just say that updating `failed'.  */
  if (question_flag && !(flags & COMMANDS_RECURSE))
    goto error;

  /* There may be some preceding whitespace left if there
     was nothing but a backslash on the first line.  */
  p = next_token (p);
  
  /* Figure out an argument list from this command line.  */
  
  {
    char *end;
    argv = construct_command_argv (p, &end, child->file);
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
      free (argv[0]);
      free ((char *) argv);
      argv = 0;
    }

  if (argv == 0)
    {
      /* This line has no commands.  Go to the next.  */
      if (job_next_command (child))
	start_job_command (child);
      return;
    }

  /* Print out the command.  */

  if (just_print_flag || (!(flags & COMMANDS_SILENT) && !silent_flag))
    puts (p);

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
      free (argv[0]);
      free ((char *) argv);
      if (job_next_command (child))
	start_job_command (child);
      return;
    }

  /* Flush the output streams so they won't have things written twice.  */

  fflush (stdout);
  fflush (stderr);
  
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
	}
    }

  /* Decide whether to give this child the `good' standard input
     (one that points to the terminal or whatever), or the `bad' one
     that points to the read side of a broken pipe.  */

  child->good_stdin = !good_stdin_used;
  if (child->good_stdin)
    good_stdin_used = 1;

  child->deleted = 0;

  /* Set up the environment for the child.  */
  if (child->environment == 0)
    child->environment = target_environment (child->file);

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
    {
      /* Fork the child process.  */

#ifdef	 POSIX
      (void) sigprocmask (SIG_BLOCK, &fatal_signal_set, (sigset_t *) 0);
#else
#ifdef	HAVE_SIGSETMASK
      (void) sigblock (fatal_signal_mask);
#endif
#endif

      child->remote = 0;
      child->pid = vfork ();
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
    }

  /* We are the parent side.  Set the state to
     say the commands are running and return.  */

  child->file->command_state = cs_running;

  /* Free the storage used by the child's argument list.  */

  free (argv[0]);
  free ((char *) argv);

  return;

 error:;
  child->file->update_status = 1;
  child->file->command_state = cs_finished;
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
      c->file->command_state = cs_running;
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

    case cs_finished:
      notice_finished_file (c->file);
      free_child (c);
      break;

    default:
      error ("internal error: `%s' command_state == %d in new_job",
	     c->file->name, (int) c->file->command_state);
      abort ();
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
	 IN gets ahead out OUT.  */

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
  if (! job_next_command (c))
    /* There were no commands!  */
    free_child (c);
  else
    {
      /* The job is now primed.  Start it running.  */
      start_waiting_job (c);

      if (job_slots == 1)
	/* Since there is only one job slot, make things run linearly.
	   Wait for the child to die, setting the state to `cs_finished'.  */
	while (file->command_state == cs_running)
	  reap_children (1, 0);
    }
}

/* Move CHILD's pointers to the next command for it to execute.
   Returns nonzero if there is another command.  */

static int
job_next_command (child)
     struct child *child;
{
  if (child->command_ptr == 0 || *child->command_ptr == '\0')
    {
      /* There are no more lines in the expansion of this line.  */
      if (child->command_line == child->file->cmds->ncommand_lines)
	{
	  /* There are no more lines to be expanded.  */
	  child->command_ptr = 0;
	  child->file->command_state = cs_finished;
	  child->file->update_status = 0;
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
  extern int getloadavg ();
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
    } while (start_waiting_job (job) && waiting_jobs != 0);
}

/* Replace the current process with one executing the command in ARGV.
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

  /* Free up file descriptors.  */
  {
    register int d;
    int max = getdtablesize ();
    for (d = 3; d < max; ++d)
      (void) close (d);
  }

  /* Run the command.  */
  exec_command (argv, envp);
}

/* Search PATH for FILE.
   If successful, store the full pathname in PROGRAM and return 1.
   If not sucessful, return zero.  */

static int
search_path (file, path, program)
     char *file, *path, *program;
{
  if (path == 0 || path[0] == '\0')
    path = default_path;

  if (index (file, '/') != 0)
    {
      strcpy (program, file);
      return 1;
    }
  else
    {
      unsigned int len;

#ifdef	HAVE_GETGROUPS
#ifndef	HAVE_UNISTD_H
      extern int getgroups ();
#endif
      static int ngroups = -1;
#ifdef	NGROUPS_MAX
      static GETGROUPS_T groups[NGROUPS_MAX];
#define	ngroups_max	NGROUPS_MAX
#else
      static GETGROUPS_T *groups = 0;
      static int ngroups_max;
      if (groups == 0)
	{
	  ngroups_max = GET_NGROUPS_MAX;
	  groups = (GETGROUPS_T *) malloc (ngroups_max * sizeof (GETGROUPS_T));
	}
#endif
      if (groups != 0 && ngroups == -1)
	ngroups = getgroups (ngroups_max, groups);
#endif	/* Have getgroups.  */

      len = strlen (file) + 1;
      do
	{
	  struct stat st;
	  int perm;
	  char *p;

	  p = index (path, ':');
	  if (p == 0)
	    p = path + strlen (path);

	  if (p == path)
	    bcopy (file, program, len);
	  else
	    {
	      bcopy (path, program, p - path);
	      program[p - path] = '/';
	      bcopy (file, program + (p - path) + 1, len);
	    }

	  if (stat (program, &st) == 0
	      && S_ISREG (st.st_mode))
	    {
	      if (st.st_uid == geteuid ())
		perm = (st.st_mode & 0100);
	      else if (st.st_gid == getegid ())
		perm = (st.st_mode & 0010);
	      else
		{
#ifdef	HAVE_GETGROUPS
		  register int i;
		  for (i = 0; i < ngroups; ++i)
		    if (groups[i] == st.st_gid)
		      break;
		  if (i < ngroups)
		    perm = (st.st_mode & 0010);
		  else
#endif	/* Have getgroups.  */
		    perm = (st.st_mode & 0001);
		}

	      if (perm != 0)
		return 1;
	    }

	  path = p + 1;
	} while (*path != '\0');
    }

  return 0;
}

/* Replace the current process with one running the command in ARGV,
   with environment ENVP.  This function does not return.  */

void
exec_command (argv, envp)
     char **argv, **envp;
{
  char *shell, *path;
  PATH_VAR (program);
  register char **ep;

  shell = path = 0;
  for (ep = envp; *ep != 0; ++ep)
    {
      if (shell == 0 && !strncmp(*ep, "SHELL=", 6))
	shell = &(*ep)[6];
      else if (path == 0 && !strncmp(*ep, "PATH=", 5))
	path = &(*ep)[5];
      else if (path != 0 && shell != 0)
	break;
    }

  /* Be the user, permanently.  */
  child_access ();

  if (!search_path (argv[0], path, program))
    error ("%s: Command not found", argv[0]);
  else
    {
      /* Run the program.  */
      execve (program, argv, envp);

      if (errno == ENOEXEC)
	{
	  PATH_VAR (shell_program);
	  char *shell_path;
	  if (shell == 0)
	    shell_path = default_shell;
	  else
	    {
	      if (search_path (shell, path, shell_program))
		shell_path = shell_program;
	      else
		{
		  shell_path = 0;
		  error ("%s: Shell program not found", shell);
		}
	    }

	  if (shell_path != 0)
	    {
	      char **new_argv;
	      int argc;

	      argc = 1;
	      while (argv[argc] != 0)
		++argc;

	      new_argv = (char **) alloca ((1 + argc + 1) * sizeof (char *));
	      new_argv[0] = shell_path;
	      new_argv[1] = program;
	      while (argc > 0)
		{
		  new_argv[1 + argc] = argv[argc];
		  --argc;
		}

	      execve (shell_path, new_argv, envp);
	      perror_with_name ("execve: ", shell_path);
	    }
	}
      else
	perror_with_name ("execve: ", program);
    }

  _exit (127);
}

/* Figure out the argument list necessary to run LINE as a command.
   Try to avoid using a shell.  This routine handles only ' quoting.
   Starting quotes may be escaped with a backslash.  If any of the
   characters in sh_chars[] is seen, or any of the builtin commands
   listed in sh_cmds[] is the first word of a line, the shell is used.

   If RESTP is not NULL, *RESTP is set to point to the first newline in LINE.
   If *RESTP is NULL, newlines will be ignored.

   SHELL is the shell to use, or nil to use the default shell.
   IFS is the value of $IFS, or nil (meaning the default).  */

static char **
construct_command_argv_internal (line, restp, shell, ifs)
     char *line, **restp;
     char *shell, *ifs;
{
  static char sh_chars[] = "#;\"*?[]&|<>(){}$`^";
  static char *sh_cmds[] = { "cd", "eval", "exec", "exit", "login",
			     "logout", "set", "umask", "wait", "while", "for",
			     "case", "if", ":", ".", "break", "continue",
			     "export", "read", "readonly", "shift", "times",
			     "trap", "switch", 0 };
  register int i;
  register char *p;
  register char *ap;
  char *end;
  int instring, word_has_equals, seen_nonequals;
  char **new_argv = 0;

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
  else if (strcmp (shell, default_shell))
    goto slow;

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
  instring = word_has_equals = seen_nonequals = 0;
  for (p = line; *p != '\0'; ++p)
    {
      if (ap > end)
	abort ();

      if (instring)
	{
	  /* Inside a string, just copy any char except a closing quote.  */
	  if (*p == '\'')
	    instring = 0;
	  else
	    *ap++ = *p;
	}
      else if (index (sh_chars, *p) != 0)
	/* Not inside a string, but it's a special char.  */
	goto slow;
      else
	/* Not a special char.  */
	switch (*p)
	  {
	  case '=':
	    /* Equals is a special character in leading words before the
	       first word with no equals sign in it.  This is not the case
	       with sh -k, but we never get here when using nonstandard
	       shell flags.  */
	    if (! seen_nonequals)
	      goto slow;
	    word_has_equals = 1;
	    *ap++ = '=';
	    break;

	  case '\\':
	    /* Backslash-newline combinations are eaten.  */
	    if (p[1] == '\n')
	      {
		/* Eat the backslash, the newline, and following whitespace,
		   replacing it all with a single space.  */
		p += 2;

		/* If there is a tab after a backslash-newline,
		   remove it from the source line which will be echoed,
		   since it was most likely used to line
		   up the continued line with the previous one.  */
		if (*p == '\t')
		  strcpy (p, p + 1);

		if (ap != new_argv[i])
		  /* Treat this as a space, ending the arg.
		     But if it's at the beginning of the arg, it should
		     just get eaten, rather than becoming an empty arg. */
		  goto end_of_arg;
		else
		  p = next_token (p) - 1;
	      }
	    else if (p[1] != '\0')
	      /* Copy and skip the following char.  */
	      *ap++ = *++p;
	    break;

	  case '\'':
	    instring = 1;
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
  if (new_argv[i][0] != '\0')
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
      free (new_argv);
    }

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
	      strcpy (p, p + 1);

	    p = next_token (p);
	    --p;
	    *ap++ = '\\';
	    *ap++ = ' ';
	    continue;
	  }

	if (*p == '\\' || *p == '\''
	    || isspace (*p)
	    || index (sh_chars, *p) != 0)
	  *ap++ = '\\';
	*ap++ = *p;
      }
    *ap = '\0';
    
    new_argv = construct_command_argv_internal (new_line, (char **) NULL,
						(char *) 0, (char *) 0);
  }

  return new_argv;
}

/* Figure out the argument list necessary to run LINE as a command.
   Try to avoid using a shell.  This routine handles only ' quoting.
   Starting quotes may be escaped with a backslash.  If any of the
   characters in sh_chars[] is seen, or any of the builtin commands
   listed in sh_cmds[] is the first word of a line, the shell is used.

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
    ifs = allocated_variable_expand_for_file ("$(IFS)", file);

    warn_undefined_variables_flag = save;
  }

  argv = construct_command_argv_internal (line, restp, shell, ifs);

  free (shell);
  free (ifs);

  return argv;
}

#ifndef	HAVE_DUP2
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
#endif
