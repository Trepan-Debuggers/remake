/* This file implements an interface to the Customs daemon to do
   remote execution of commands under GNU make.
   THIS CODE IS NOT SUPPORTED BY THE GNU PROJECT.
   Please do not send bug reports or questions about it to
   the Make maintainers.

Copyright (C) 1988, 1989 Free Software Foundation, Inc.
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
#include "customs.h"
#include <sys/time.h>


char *remote_description = "Customs";


/* ExportPermit gotten by start_remote_job_p, and used by start_remote_job.  */
static ExportPermit permit;

/* Return nonzero if the next job should be done remotely.  */

int
start_remote_job_p ()
{
  if (Customs_Host (EXPORT_SAME, &permit) != RPC_SUCCESS)
    return 0;

  return !CUSTOMS_FAIL (&permit.addr);
}

/* Start a remote job running the command in ARGV.
   It gets standard input from STDIN_FD.  On failure,
   return nonzero.  On success, return zero, and set
   *USED_STDIN to nonzero if it will actually use STDIN_FD,
   zero if not, set *ID_PTR to a unique identification, and
   set *IS_REMOTE to zero if the job is local, nonzero if it
   is remote (meaning *ID_PTR is a process ID).  */

int
start_remote_job (argv, stdin_fd, is_remote, id_ptr, used_stdin)
     char **argv;
     int stdin_fd;
     int *is_remote;
     int *id_ptr;
     int *used_stdin;
{
  extern char **environ;
  extern int vfork (), execve ();
  PATH_VAR (cwd);
  char waybill[MAX_DATA_SIZE], msg[128];
  struct timeval timeout;
  struct sockaddr_in sin;
  int len;
  int retsock, retport, sock;
  Rpc_Stat status;
  int pid;

  /* Find the current directory.  */
  if (getwd (cwd) == 0)
    {
      error ("exporting: getwd: %s", cwd);
      return 1;
    }

  /* Create the return socket.  */
  retsock = Rpc_UdpCreate (True, 0);
  if (retsock < 0)
    {
      error ("exporting: Couldn't create return socket.");
      return 1;
    }

  /* Get the return socket's port number.  */
  len = sizeof(sin);
  if (getsockname (retsock, (struct sockaddr *) &sin, &len) < 0)
    {
      (void) close (retsock);
      perror_with_name ("exporting: ", "getsockname");
      return 1;
    }
  retport = sin.sin_port;

  /* Create the TCP socket for talking to the remote child.  */
  sock = Rpc_TcpCreate (False, 0);

  /* Create a WayBill to give to the server.  */
  len = Customs_MakeWayBill (&permit, cwd, argv[0], argv,
			     environ, retport, waybill);

  /* Send the request to the server, timing out in 20 seconds.  */
  timeout.tv_usec = 0;
  timeout.tv_sec = 20;
  sin.sin_family = AF_INET;
  sin.sin_port = htons (Customs_Port ());
  sin.sin_addr = permit.addr;
  status = Rpc_Call (sock, &sin, (Rpc_Proc) CUSTOMS_IMPORT,
		     len, (Rpc_Opaque) waybill,
		     sizeof(msg), (Rpc_Opaque) msg,
		     1, &timeout);
  if (status != RPC_SUCCESS)
    {
      (void) close (retsock);
      (void) close (sock);
      error ("exporting: %s", Rpc_ErrorMessage (status));
      return 1;
    }
  else if (msg[0] != 'O' || msg[1] != 'k' || msg[2] != '\0')
    {
      (void) close (retsock);
      (void) close (sock);
      error ("CUSTOMS_IMPORT: %s", msg);
      return 1;
    }

  fflush (stdout);
  fflush (stderr);

  pid = vfork ();
  if (pid < 0)
    {
      /* The fork failed!  */
      perror_with_name ("vfork", "");
      return 1;
    }
  else if (pid == 0)
    {
      /* Child side.  Run `export' to handle the connection.  */
      static char sock_buf[20], retsock_buf[20], id_buf[20];
      static char *new_argv[6] =
	{ "export", "-id", sock_buf, retsock_buf, id_buf, 0 };

      /* Set up the arguments.  */
      (void) sprintf (sock_buf, "%d", sock);
      (void) sprintf (retsock_buf, "%d", retsock);
      (void) sprintf (id_buf, "%x", permit.id);

      /* Run the command.  */
      (void) execvp (new_argv[0], new_argv);
      perror_with_name ("execvp: ", new_argv[0]);
      _exit (127);
    }

  /* Parent side.  Return the `export' process's ID.  */
  (void) close (retsock);
  (void) close (sock);
  *is_remote = 0;
  *id_ptr = pid;
  return 0;
}

/* Get the status of a dead remote child.  Block waiting for one to die
   if BLOCK is nonzero.  Set *EXIT_CODE_PTR to the exit status, *SIGNAL_PTR
   to the termination signal or zero if it exited normally, and *COREDUMP_PTR
   nonzero if it dumped core.  Return the ID of the child that died,
   0 if we would have to block and !BLOCK, or < 0 if there were none.  */

int
remote_status (exit_code_ptr, signal_ptr, coredump_ptr, block)
     int *exit_code_ptr, *signal_ptr, *coredump_ptr;
     int block;
{
  return -1;
}

/* Block asynchronous notification of remote child death.
   If this notification is done by raising the child termination
   signal, do not block that signal.  */
void
block_remote_children ()
{
  return;
}

/* Restore asynchronous notification of remote child death.
   If this is done by raising the child termination signal,
   do not unblock that signal.  */
void
unblock_remote_children ()
{
  return;
}

/* Send signal SIG to child ID.  Return 0 if successful, -1 if not.  */
int
remote_kill (id, sig)
     int id;
     int sig;
{
  return -1;
}
