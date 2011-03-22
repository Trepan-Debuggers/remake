/*
Copyright (C) 2004, 2005, 2007, 2008 Rocky Bernstein <rocky@gnu.org>

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

/** \file trace.h
 *
 *  \brief Header for routines related to tracing and debugging support.
 */

#ifndef REMAKE_TRACE_H
#define REMAKE_TRACE_H

#include "filedef.h"

typedef enum {
  continue_execution,   /**< Get out of debug read loop and continue execution
			     as normal.  */
  next_execution,       /**< Get out of debug read loop and continue execution
			     as but don't enter debugger for the any remaining
			     commands.  */
  skip_execution,       /**< Get out of debug read loop, but skip execution 
			     of next command or action. */
  debug_readloop,       /**< Stay in debugger read loop - used only
			   inside debugger read loop. */
  debug_cmd_error       /**< Command error but stay in debugger read loop - 
                             used only inside debugger read loop. */
} debug_return_t;

typedef enum 
  {
    DEBUG_BRKPT_BEFORE_PREREQ     = 0,
    DEBUG_BRKPT_AFTER_PREREQ      = 1,
    DEBUG_GOAL_UPDATED_HIT        = 2,
    DEBUG_READ_HIT                = 3,
    DEBUG_ERROR_HIT               = 4,
    DEBUG_STEP_HIT                = 5,
    DEBUG_NOT_GIVEN               = 100
  } debug_enter_reason_t;

/*!
  debugger command interface. 
*/
/*! A call "stack". Well, since we'll have to deal with multiple child
   "jobs" it's not really a stack but a tree. 
*/

/*! \brief Node for an item in the target call stack */
typedef struct target_stack_node
  {
    file_t                   *p_target;
    file_t                   *p_shared_target;
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

/*! \brief Node for an item in the "include Makefile" stack */
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

#endif /*REMAKE_TRACE_H*/
