/* Copyright (C) 1988, 1989, 1991 Free Software Foundation, Inc.
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

/* Structure that gives the commands to make a file
   and information about where these commands came from.  */

struct commands
  {
    char *filename;		/* File that contains commands.  */
    unsigned int lineno;	/* Line number in file.  */
    char *commands;		/* Commands text.  */
    unsigned int ncommand_lines;/* Number of command lines.  */
    char **command_lines;	/* Commands chopped up into lines.  */
    char *lines_recurse;	/* One flag for each line.  */
    char any_recurse;		/* Nonzero if any `lines_recurse' elt is.  */
  };


extern void execute_file_commands ();
extern void print_commands ();
extern void delete_child_targets ();
extern void chop_commands ();
