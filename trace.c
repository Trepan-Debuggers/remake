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
#include "debug.h"

/*! Push "target" to the call stack. */
extern target_stack_node_t *
trace_push_target (target_stack_node_t *p, file_t *p_target) {
  target_stack_node_t *new_node = 
    (target_stack_node_t *) xmalloc (sizeof(target_stack_node_t));

  if ( (tracing || p_target->tracing)  && p_target->floc.filenm != NULL) {
    show_file_target_prefix(p_target);
    printf("\n");
  }
  /* +++ HERE IS WHERE DEBUG WOULD STOP ++++ */
  new_node->p_target = p_target;
  new_node->p_parent = p;
  return new_node;
};

/*! Pop the next target from the call stack.. */
extern void
trace_pop_target (target_stack_node_t *p) 
{
  /* +++ HERE IS WHERE DEBUG MIGHT STOP ++++ */
  if (NULL == p) return;
  free(p);
}




