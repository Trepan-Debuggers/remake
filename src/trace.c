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

#include "trace.h"
#include "print.h"
#include "debug.h"
#include "dbg_cmd.h"

/*! Push "target" to the call stack. */
extern target_stack_node_t *
trace_push_target (target_stack_node_t *p, file_t *p_target,
		   int b_debugger) {
  target_stack_node_t *new_node = 
    (target_stack_node_t *) xmalloc (sizeof(target_stack_node_t));

  /* We allocate and make a copy of p_target in case we want to
     modify information, like the file location or target name
     on the fly as we process file commands or handle dependencies from
     target patterns.
   */
  new_node->p_target = (file_t *) xmalloc (sizeof(file_t));
  memcpy(new_node->p_target, p_target, sizeof(file_t));

  new_node->p_parent = p;

  /* We don't want to trace file dependencies -- there or too often
     too many of them. Instead if the dependency has commands to run
     or is a phony target, then we'll call that interesting.
  */
  if (p_target && p_target->floc.filenm != NULL) {

    if ( tracing && (p_target->cmds || p_target->phony) ) {
      print_file_target_prefix(p_target);
      printf("\n");
    } 

    if (b_debugger && debugger_stepping && p_target->cmds )
      enter_debugger(new_node, p_target, 0);
    else if ( p_target->tracing )
      enter_debugger(new_node, p_target, 0);
  }
  
  return new_node;
};

/*! Pop the next target from the call stack.. */
extern void
trace_pop_target (target_stack_node_t *p) 
{
  /*if ( debugger_stepping ) enter_debugger(p, NULL, 0);*/
  if (NULL == p) return;
  free(p->p_target);
  free(p);
}




