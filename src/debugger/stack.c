/* 
Copyright (C) 2005, 2008, 2009, 2011 R. Bernstein <rocky@gnu.org>
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

/* debugger command interface. */

#include "fns.h"
#include "msg.h"
#include "print.h"
#include "stack.h"
#include "commands.h"

int i_stack_pos = 0;

/** Pointer to current target call stack at the place we are currently
   focused on.
 */
target_stack_node_t *p_stack = NULL;
floc_stack_node_t *p_floc_stack = NULL;


unsigned int dbg_stack_size() 
{
  int i=0;
  
  if (p_stack_top) {
    for ( p_stack=p_stack_top; p_stack ; p_stack = p_stack->p_parent ) {
      i++;
    }
    return i;
  } else if (p_stack_floc_top) {
    /* We have a Makefile stack */
    for ( p_floc_stack=p_stack_floc_top; 
	  p_floc_stack ; p_floc_stack = p_floc_stack->p_parent ) {
      i++;
    }

  }
  return i;
}


debug_return_t 
dbg_adjust_frame(int i_amount, int b_absolute) 
{
  int i=0;
  int i_try_frame_pos;

  if (b_absolute) {
    if (i_amount < 0)
      i_try_frame_pos = dbg_stack_size() + i_amount;
    else    
      i_try_frame_pos = i_amount;
  } else
    i_try_frame_pos = i_stack_pos + i_amount;

  if (i_try_frame_pos < 0) {
    dbg_errmsg(_("Moving target would go beyond bottom-most target position."));
    return debug_cmd_error;
  }

  i = i_try_frame_pos + 1;

  if (p_stack_top) {
    for ( p_stack=p_stack_top; p_stack ; p_stack = p_stack->p_parent ) {
      i--;
      if (0 == i)
	break;
    }

    if (0 != i) {
     dbg_errmsg(_("Can't set frame to position %d; "
	       "%d is the highest target position."),
	     i_try_frame_pos, i_try_frame_pos - i);
      return debug_cmd_error;
    }
    
    i_stack_pos     = i_try_frame_pos;
    p_target_loc    = &(p_stack->p_target->floc);
    
    print_target_stack_entry(p_stack->p_target, i_stack_pos, i_stack_pos);
    print_debugger_location(p_stack->p_target, DEBUG_STACK_CHANGING, NULL);
  } else if (p_stack_floc_top) {
    /* We have a Makefile stack */
    for ( p_floc_stack=p_stack_floc_top; 
	  p_floc_stack ; p_floc_stack = p_floc_stack->p_parent ) {
      i--;
      if (0 == i)
	break;
    }

    if (0 != i) {
      dbg_errmsg(_("Can't set frame to position %d; "
	       "%d is the highest target position."),
	     i_try_frame_pos, i_try_frame_pos - i);
      return debug_cmd_error;
    }
    i_stack_pos     = i_try_frame_pos;

    print_debugger_location(NULL, DEBUG_NOT_GIVEN, p_floc_stack);
  }
  
  return debug_readloop;
}

