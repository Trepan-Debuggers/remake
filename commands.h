/* Definition of data structures describing shell commands for GNU Make.
Copyright (C) 1988, 1989, 1990, 1991, 1992, 1993, 1994, 1995, 1996, 1997,
1998, 1999, 2000, 2001, 2002, 2003, 2004, 2005, 2006 Free Software
Foundation, Inc.

   Copyright (C) 2008 R. Bernstein <rocky@gnu.org>

This file is part of GNU Make.

GNU Make is free software; you can redistribute it and/or modify it under the
terms of the GNU General Public License as published by the Free Software
Foundation; either version 2, or (at your option) any later version.

GNU Make is distributed in the hope that it will be useful, but WITHOUT ANY
WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR
A PARTICULAR PURPOSE.  See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License along with
GNU Make; see the file COPYING.  If not, write to the Free Software
Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301 USA.  */

/* Structure that gives the commands to make a file
   and information about where these commands came from.  */

#ifndef COMMANDS_H
#define COMMANDS_H

#include "trace.h"
#include "job.h"

/*! \brief Command structure. 

   This gives the commands to make a file
   and information about where these commands came from.  */
struct commands
  {
    floc_t fileinfo;	            /**< Where commands were defined.  */
    char *commands;		    /**< Commands text.  */
    unsigned int ncommand_lines;    /**< Number of command lines.  */
    char **command_lines;	    /**< Commands chopped up into lines.  */
    unsigned int *line_no;	    /**< line number offsets of chopped
				         commands.  */
    char *lines_flags;		    /**< A set of flag bits for each line.  */
    int any_recurse;		    /**< Nonzero if any `lines_recurse' 
				         elt has the COMMANDS_RECURSE bit
					 set.  */
};

/* Bits in `lines_flags'.  */
#define	COMMANDS_RECURSE	1 /* Recurses: + or $(MAKE).  */
#define	COMMANDS_SILENT		2 /* Silent: @.  */
#define	COMMANDS_NOERROR	4 /* No errors: -.  */

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
  Print out the commands.

  @param p_cmds location of commands to print out.
  @param p_target used to set automatic variables if it is non-null.
  @param b_expand if true, expand the commands to remove MAKE variables.
*/
extern void print_commands (file_t *p_target, commands_t *p_cmds, 
			    bool b_expand);

extern void delete_child_targets PARAMS ((struct child *child));
extern void chop_commands PARAMS ((struct commands *cmds));
extern void set_file_variables PARAMS ((struct file *file));

#endif /*COMMANDS_H*/
