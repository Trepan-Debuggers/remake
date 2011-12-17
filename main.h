/* 
Copyright (C) 2011 R. Bernstein <rocky@gnu.org>
This file is part of GNU Make (remake variant).

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

/*! If nonzero, the basename of filenames is in giving locations. Normally,
    giving a file directory location helps a debugger frontend
    when we change directories. For regression tests it is helpful to 
    list just the basename part as that doesn't change from installation
    to installation. Users may have their preferences too.
*/
extern int basename_filenames;

/*! Nonzero means --trace=noshell.  */

extern int no_shell_trace;

/*! Nonzero gives a list of explicit target names and exits. Set by option
  --tasks
 */

extern int show_tasks_flag;

/*! Nonzero gives a list of explicit target names and exits. Set by option
  --targets
 */

extern int show_targets_flag;

/*! Nonzero means use GNU readline in the debugger. */
extern int use_readline_flag;

/* is default_shell unixy? */
extern int unixy_shell;

/**! The default value of SHELL and the shell that is used when issuing
   commands on targets.
*/
extern char default_shell[];

/*! Print version information. */
extern void print_version (void);
