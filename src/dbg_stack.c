/* $Id: dbg_stack.c,v 1.5 2005/12/18 12:30:49 rockyb Exp $
Copyright (C) 2005 rocky@panix.com
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

/* debugger command interface. */

#include "dbg_fns.h"
#include "dbg_stack.h"
#include "commands.h"

int i_stack_pos = 0;

/** Pointer to current target call stack at the place we are currently
   focused on.
 */
target_stack_node_t *p_stack = NULL;
floc_stack_node_t *p_floc_stack = NULL;

/** Move reported target frame postition down by psz_amount. */
debug_return_t 
dbg_cmd_frame_down (char *psz_amount) 
{
  unsigned int i=0;
  int i_amount = 1;

  if (!psz_amount || !*psz_amount) {
    i_amount = 1;
  } else if (!get_int(psz_amount, &i_amount, true)) {
      return debug_readloop;
  }

  if (i_stack_pos - i_amount < 0) {
    printf(_("Move down by %d would be below bottom-most frame position.\n"),
	   i_amount);
    return debug_readloop;
  }
  
  i_stack_pos -= i_amount;

  if (p_stack_top) {
    /* We have a target stack  */
    for ( p_stack=p_stack_top; p_stack ; p_stack = p_stack->p_parent ) {
      if (i_stack_pos == i)
	break;
      i++;
    }

    p_target_loc    = &(p_stack->p_target->floc);
    psz_target_name = p_stack->p_target->name;
    
    print_debugger_location(p_stack->p_target, NULL);
    
  } else if (p_stack_floc_top) {
    /* We have a Makefile stack */
    for ( p_floc_stack=p_stack_floc_top; 
	  p_floc_stack ; p_floc_stack = p_floc_stack->p_parent ) {
      if (i_stack_pos == i)
	break;
      i++;
    }

    print_debugger_location(NULL, p_floc_stack);

  }
  
  return debug_readloop;
}

/** Move reported target frame postition to absolute position psz_frame. */
debug_return_t 
dbg_cmd_frame (char *psz_frame) 
{
  int i, i_frame;

  if (!psz_frame || !*psz_frame) {
    return debug_readloop;
  } else {
    if (!get_int(psz_frame, &i_frame, true))
      return debug_readloop;
  }

  i = i_frame + 1;

  if (p_stack_top) {
    for ( p_stack=p_stack_top; p_stack ; p_stack = p_stack->p_parent ) {
      i--;
      if (0 == i)
	break;
    }

    if (0 != i) {
      printf(_("Can't set frame to position %d; "
	       "%d is the highest position.\n"),
	     i_frame, i_frame - i);
      return debug_readloop;
    }
    
    i_stack_pos     = i_frame;
    p_target_loc    = &(p_stack->p_target->floc);
    psz_target_name = p_stack->p_target->name;
    
    print_debugger_location(p_stack->p_target, NULL);
  } else if (p_stack_floc_top) {
    /* We have a Makefile stack */
    for ( p_floc_stack=p_stack_floc_top; 
	  p_floc_stack ; p_floc_stack = p_floc_stack->p_parent ) {
      i--;
      if (0 == i)
	break;
    }

    if (0 != i) {
      printf(_("Can't set frame to position %d; "
	       "%d is the highest position.\n"),
	     i_frame, i_frame - i);
      return debug_readloop;
    }
    i_stack_pos     = i_frame;

    print_debugger_location(NULL, p_floc_stack);
  }
  
  return debug_readloop;
}

/** Move reported target frame postition up by psz_amount. */
debug_return_t 
dbg_cmd_frame_up (char *psz_amount) 
{
  unsigned int i_amount=1;
  unsigned int i = 0;

  if (!psz_amount || !*psz_amount) {
    i_amount = 1;
  } else {
    if (!get_uint(psz_amount, &i_amount))
      return debug_readloop;
  }

  if (p_stack_top) {
    /* We have a target stack  */
    target_stack_node_t *p=p_stack;

    for ( ; p ; p = p->p_parent, i++ ) {
      if (i_amount == i) break;
    }
    
    if (p) {
      i_stack_pos    += i_amount;
      p_stack         = p;
      psz_target_name = p->p_target->name;
      
      p_target_loc    = &(p->p_target->floc);
      if (!p->p_target->floc.filenm && p->p_target->cmds->fileinfo.filenm) {
	/* Fake the location based on the commands - it's better than
	   nothing...
	*/
	memcpy(&fake_floc, &(p->p_target->cmds->fileinfo),
	       sizeof(floc_t));
	/* HACK: is it okay to assume that the target is on the line
	   before the first command? Or should we list the line
	   that the command starts on - so we know we've faked the location?
	*/
	fake_floc.lineno--;
	p_target_loc = &fake_floc;
      }
    } else {
      printf("Can't move up %d - would be beyond top-most frame position.\n",
	     i_amount);
    }
    
    print_debugger_location(p_stack->p_target, NULL);
  } else if (p_floc_stack) {
    /* We don't have a target stack, but we have a Makefile read stack  */
    floc_stack_node_t *p=p_floc_stack;
    for ( ; p ; p = p->p_parent, i++ ) {
      if (i_amount == i) break;
    }
    
    if (p) {
      i_stack_pos    += i_amount;
      p_floc_stack   = p;
      
    } else {
      printf("Can't move up %d - would be beyond top-most frame position.\n",
	     i_amount);
    }
    print_debugger_location(NULL, p_floc_stack);
  }
  
  return debug_readloop;
}
