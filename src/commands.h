/* Definition of data structures describing shell commands for GNU Make.
Copyright (C) 1988-2022 Free Software Foundation, Inc.
This file is part of GNU Make.

GNU Make is free software; you can redistribute it and/or modify it under the
terms of the GNU General Public License as published by the Free Software
Foundation; either version 3 of the License, or (at your option) any later
version.

GNU Make is distributed in the hope that it will be useful, but WITHOUT ANY
WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR
A PARTICULAR PURPOSE.  See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License along with
this program.  If not, see <https://www.gnu.org/licenses/>.  */

/* Structure that gives the commands to make a file
   and information about where these commands came from.  */

#ifndef REMAKE_COMMANDS_H
#define REMAKE_COMMANDS_H

#include "makeint.h"
#include "filedef.h"
#include "trace.h"

struct commands
  {
    gmk_floc fileinfo;          /* Where commands were defined.  */
    char *commands;             /* Commands text.  */
    char **command_lines;       /* Commands chopped up into lines.  */
    unsigned char *lines_flags; /* One set of flag bits for each line.  */
    unsigned short ncommand_lines;/* Number of command lines.  */
    char recipe_prefix;         /* Recipe prefix for this command set.  */
    unsigned int any_recurse:1; /* Nonzero if any 'lines_flags' elt has */
                                /* the COMMANDS_RECURSE bit set.  */
  };

/* Bits in 'lines_flags'.  */
#define COMMANDS_RECURSE        1 /* Recurses: + or $(MAKE).  */
#define COMMANDS_SILENT         2 /* Silent: @.  */
#define COMMANDS_NOERROR        4 /* No errors: -.  */

RETSIGTYPE fatal_error_signal (int sig);
/*!
  Execute the commands to remake P_FILE.  If they are currently
  executing, return or have already finished executing, just return.
  Otherwise, fork off a child process to run the first command line
  in the sequence.

  @param p_file  pointer to file to remake.

  @param p_call_stack pointer to current target call stack. This is
  passed down for information reporting.

*/
extern void execute_file_commands (file_t *p_file,
				   target_stack_node_t *p_call_stack);

/*!
+  Print out the commands.
+
+  @param p_cmds location of commands to print out.
+  @param p_target used to set automatic variables if it is non-null.
+  @param b_expand if true, expand the commands to remove MAKE variables.
+*/
extern void print_commands (file_t *p_target, commands_t *p_cmds, bool b_expand);

void delete_child_targets (struct child *child);
void chop_commands (struct commands *cmds);
void set_file_variables (struct file *file, const char *stem);

#endif /*REMAKE_COMMANDS_H*/
