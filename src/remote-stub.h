/* Template for the remote job exportation interface to GNU Make.
Copyright (C) 2004 Free Software Foundation, Inc.
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

/*! Return nonzero if the next job should be done remotely.  
*/

#ifndef REMOTE_STUB_H
#define REMOTE_STUB_H

/*! Return nonzero if the next job should be done remotely.  
*/
extern int start_remote_job_p PARAMS ((int first_p UNUSED));

/*! Start a remote job running the command in ARGV, with environment
   from ENVP.  It gets standard input from STDIN_FD.  On failure,
   return nonzero.  On success, return zero, and set *USED_STDIN to
   nonzero if it will actually use STDIN_FD, zero if not, set *ID_PTR
   to a unique identification, and set *IS_REMOTE to zero if the job
   is local, nonzero if it is remote (meaning *ID_PTR is a process
   ID).  */
extern int start_remote_job PARAMS ((char **argv, char **envp, int stdin_fd,
				     int *is_remote, int *id_ptr, 
				     int *used_stdin));

/*! Get the status of a dead remote child.  Block waiting for one to die
   if BLOCK is nonzero.  Set *EXIT_CODE_PTR to the exit status, *SIGNAL_PTR
   to the termination signal or zero if it exited normally, and *COREDUMP_PTR
   nonzero if it dumped core.  Return the ID of the child that died,
   0 if we would have to block and !BLOCK, or < 0 if there were none.
*/
extern int remote_status PARAMS ((int *exit_code_ptr, int *signal_ptr,
				  int *coredump_ptr, int block));

#endif /* REMOTE_STUB_H*/
