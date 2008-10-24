/* $Id: trace.c,v 1.6 2005/12/20 15:11:24 rockyb Exp $
Copyright (C) 2004, 2005 Free Software Foundation, Inc.
Copyright (C) 2008 R. Bernstein <rocky@gnu.org>

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

#include "trace.h"
#include "print.h"
#include "debug.h"
#include "dbg_cmd.h"

/** Pointer to top of current target call stack */
target_stack_node_t *p_stack_top;

/** Pointer to top of current target floc stack */
floc_stack_node_t *p_stack_floc_top = NULL;

/*! Push "target" to the call stack. */
extern target_stack_node_t *
trace_push_target (target_stack_node_t *p, file_t *p_target,
		   int b_debugger) {
  target_stack_node_t *new_node = CALLOC(target_stack_node_t, 1);

  /* We allocate and make a copy of p_target in case we want to
     modify information, like the file location or target name
     on the fly as we process file commands or handle dependencies from
     target patterns.
   */
  new_node->p_target = CALLOC (file_t, 1);
  memcpy(new_node->p_target, p_target, sizeof(file_t));

  new_node->p_parent = p;

  /* We don't want to trace file dependencies -- there or too often
     too many of them. Instead if the dependency has commands to run
     or is a phony target, then we'll call that interesting.
  */
  if (p_target && p_target->floc.filenm != NULL) {

    if ( db_level & DB_VERBOSE ) {
      print_file_target_prefix(p_target);
      printf("\n");
    } 

    if (b_debugger && i_debugger_stepping && p_target->cmds )
      enter_debugger(new_node, p_target, 0, DEBUG_STEP_HIT);
    else if ( p_target->tracing )
      enter_debugger(new_node, p_target, 0, DEBUG_STEP_HIT);
  }
  
  return new_node;
};

/*! Pop the next target from the call stack.. */
extern void
trace_pop_target (target_stack_node_t *p) 
{
  if (NULL == p) return;
  free(p->p_target);
  free(p);
}

/*! Push "p_floc" to the floc stack. Return the new stack top. 
*/
extern void
trace_push_floc (floc_t *p_floc) 
{
  floc_stack_node_t *new_node = CALLOC (floc_stack_node_t, 1);

  /* We DO NOT allocate and make a copy of p_floc so that as we
     read the Makefile, the line number gets updated automatically.
     Slick, huh? Also it shortens and simplifies code a bit.
   */
  new_node->p_floc = p_floc;
  new_node->p_parent = p_stack_floc_top;
  p_stack_floc_top = new_node;
};

/*! Pop the next target from the floc stack. */
extern void
trace_pop_floc (void) 
{
  if (NULL == p_stack_floc_top) return;
  else {
    floc_stack_node_t *p_new_top = p_stack_floc_top->p_parent;
    free(p_stack_floc_top);
    p_stack_floc_top = p_new_top;
  }
}
