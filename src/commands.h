/* $Id: commands.h,v 1.9 2005/12/10 02:50:32 rockyb Exp $
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

/*! \brief Command structure. This gives the commands to make a file
   and information about where these commands came from.  */
struct commands
  {
    floc_t fileinfo;	        /**< Where commands were defined.  */
    char *commands;		/**< Commands text.  */
    unsigned int ncommand_lines;/**< Number of command lines.  */
    char **command_lines;	/**< Commands chopped up into lines.  */
    unsigned int *line_no;	/**< line number offsets of chopped
				     commands.  */
    char *lines_flags;		/**< One set of flag bits for each line.  */
    int any_recurse;		/**< Nonzero if any `lines_recurse' elt has */
				/**< the COMMANDS_RECURSE bit set.  */
};

/** Bits in `lines_flags'.  */
#define	COMMANDS_RECURSE	1 /* Recurses: + or $(MAKE).  */
#define	COMMANDS_SILENT		2 /* Silent: @.  */
#define	COMMANDS_NOERROR	4 /* No errors: -.  */

/*! Execute the commands to remake FILE.  If they are currently
   executing, return or have already finished executing, just return.
   Otherwise, fork off a child process to run the first command line
   in the sequence.  
*/
extern void execute_file_commands (file_t *p_file, 
				   target_stack_node_t *p_call_stack);

/*! 
  Print out the commands in p_CMDS. If b_expand is true expand the
  commands to remove MAKE variables. p_target is used to set automatic
  variables if it is non-null
*/
extern void print_commands (file_t *p_target, commands_t *p_cmds, 
			    bool b_expand);

/*! Delete all non-precious targets of CHILD unless they were already
   deleted.  Set the flag in CHILD to say they've been deleted.  
*/
extern void delete_child_targets (child_t *p_child);

/*! Chop CMDS up into individual command lines if necessary.  Also set
   the `lines_flags' and `any_recurse' members.
*/
extern void chop_commands (commands_t *p_cmds);

/*! Set FILE's automatic variables up.  */
extern void set_file_variables(file_t *file);


#endif /*COMMANDS_H*/
