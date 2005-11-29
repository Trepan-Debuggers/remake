/* 
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

/* Header for routines related to tracing and debugging support */


#ifndef TRACE_H
#define TRACE_H

#include "make.h"
#include "filedef.h"

typedef enum {
  continue_execution,   /**< Get out of debug read loop and continue execution
			     as normal.  */
  next_execution,       /**< Get out of debug read loop and continue execution
			     as but don't enter debugger for the any remaining
			     commands.  */
  skip_execution,       /**< Get out of debug read loop, but skip execution 
			     of next command or action. */
  debug_readloop        /**< Stay in debugger read loop - used only
			   inside debugger read loop. */
} debug_return_t;

/*! A call "stack". Well, since we'll have to deal with multiple child
   "jobs" it's not really a stack but a tree. 
*/

/*! Node for an item in the target call stack */
typedef struct target_stack_node
  {
    file_t                   *p_target;
    struct target_stack_node *p_parent;
  } target_stack_node_t; 

/** Pointer to top of current target call stack */
extern target_stack_node_t *p_stack_top;

/*! Push "p_target" to the call stack. Return the new stack top. 
    if b_debugger is true we might enter the debugger.
*/
extern target_stack_node_t *trace_push_target (target_stack_node_t *p, 
					       file_t *p_target,
					       int b_debugger);

/*! Pop the next target from the call stack.. */
extern void trace_pop_target (target_stack_node_t *p);

/*! Node for an item in the "include Makefile" stack */
typedef struct floc_stack_node
  {
    floc_t                 *p_floc;
    struct floc_stack_node *p_parent;
  } floc_stack_node_t;

/** Pointer to top of current target floc stack */
extern floc_stack_node_t *p_stack_floc_top;

/*! Push "p_floc" to the floc stack. Return the new stack top. 
*/
extern void trace_push_floc (floc_t *p_floc);

/*! Pop the next floc from the call stack.. */
extern void trace_pop_floc (void);

#endif /*TRACE_H*/
