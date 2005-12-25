/* $Id: commands.h,v 1.11 2005/12/25 10:08:35 rockyb Exp $
Copyright (C) 1988, 1989, 1991, 1993, 2004, 2005 Free Software Foundation, Inc.
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

/** \file commands.h
 *
 *  \brief Definition of data structures describing shell commands for
 *  GNU Make.
 */

#ifndef COMMANDS_H
#define COMMANDS_H

#include "trace.h"
#include "job.h"

/*! \brief bits in `lines_flags'.  */
typedef enum {
   COMMANDS_RECURSE  = 	1, /**< Recurses: + or $(MAKE).  */
   COMMANDS_SILENT   =	2, /**< Silent: @.  */
   COMMANDS_NOERROR  =	4  /**< No errors: -.  */
} line_flags_enum_t;
  
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
    line_flags_enum_t *lines_flags; /**< A set of flag bits for each line.  */
    int any_recurse;		    /**< Nonzero if any `lines_recurse' 
				         elt has the COMMANDS_RECURSE bit
					 set.  */
};

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

/*! 
  Delete all non-precious targets of P_CHILD unless they were already
  deleted.  Set the flag in P_CHILD to say they've been deleted.  

  @param p_child  a pointer to the child target to work on.
*/
extern void delete_child_targets (child_t *p_child);

/*! 
   Chop commands into individual command lines if necessary.  Also set
   the `lines_flags' and `any_recurse' members.

   @param p_cmds a pointer to the commands to chop up.
*/
extern void chop_commands (commands_t *p_cmds);

/*! 
  Set up automatic variables for a file.  
  @param p_file a pointer to the file to set up.
*/
extern void set_file_variables(file_t *p_file);


#endif /*COMMANDS_H*/
