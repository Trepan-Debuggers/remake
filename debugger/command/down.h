/** Move reported target frame postition down by psz_amount. */
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
dbg_cmd_frame_down(char *psz_amount) 
{
  int i=0;
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
    
    print_debugger_location(p_stack->p_target, DEBUG_NOT_GIVEN, NULL);
    
  } else if (p_stack_floc_top) {
    /* We have a Makefile stack */
    for ( p_floc_stack=p_stack_floc_top; 
	  p_floc_stack ; p_floc_stack = p_floc_stack->p_parent ) {
      if (i_stack_pos == i)
	break;
      i++;
    }

    print_debugger_location(NULL, DEBUG_NOT_GIVEN, p_floc_stack);

  }
  
  return debug_readloop;
}

static void
dbg_cmd_down_init(unsigned int c) 
{
  short_command[c].func = &dbg_cmd_frame_down;
  short_command[c].use  = _("down [AMOUNT]");
  short_command[c].doc  = 
    _("Select and print the target this one caused to be examined.\n"
      "\tAn argument says how many targets down to go.");
}

/* 
 * Local variables:
 * eval: (c-set-style "gnu")
 * indent-tabs-mode: nil
 * End:
 */
