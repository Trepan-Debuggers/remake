/** Move reported target frame position up by psz_amount. */
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
      
      p_target_loc    = &(p->p_target->floc);
      if (!p->p_target->floc.filenm && p->p_target->cmds &&
	  p->p_target->cmds->fileinfo.filenm) {
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
    
    print_debugger_location(p_stack->p_target, DEBUG_NOT_GIVEN, NULL);
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
    print_debugger_location(NULL, DEBUG_NOT_GIVEN, p_floc_stack);
  }
  
  return debug_readloop;
}

static void
dbg_cmd_up_init(void) 
{
  short_command['u'].func = &dbg_cmd_frame_up;
  short_command['u'].use  = _("up [AMOUNT]");
  short_command['u'].doc  = 
    _("Select and print target that caused this one to be examined.\n"
      "An argument says how many targets up to go.");
}



/* 
 * Local variables:
 * eval: (c-set-style "gnu")
 * indent-tabs-mode: nil
 * End:
 */

