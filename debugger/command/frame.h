/** Move reported target frame position to absolute position psz_frame. */
/* 
Copyright (C) 2011 R. Bernstein <rocky@gnu.org>
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
debug_return_t 
dbg_cmd_frame(void) 
{
  char *psz_frame = psz_debugger_args;
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
    
    print_debugger_location(p_stack->p_target, DEBUG_NOT_GIVEN, NULL);
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

    print_debugger_location(NULL, DEBUG_NOT_GIVEN, p_floc_stack);
  }
  
  return debug_readloop;
}
