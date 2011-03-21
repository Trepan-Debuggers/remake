/* Write commands associated with a given target. */
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
/* Show target call stack info. */

debug_return_t 
dbg_cmd_where(void)
{

  char *psz_amount = psz_debugger_args;
  int i_amount;
  
  if (!psz_amount || !*psz_amount) {
    i_amount = MAX_STACK_SHOW;
  } else if (!get_int(psz_amount, &i_amount, true)) {
      return debug_readloop;
  }

  if (p_stack_top)
    print_target_stack (p_stack_top, i_stack_pos, i_amount);

  if (p_stack_floc_top) 
    print_floc_stack (i_stack_pos, i_amount);
  
  /* If we are in a recursive Make, show the command invocation */
  if (makelevel > 0) 
    {
      printf("Most-recent (level %d) invocation:\n\t", makelevel);
      dbg_print_invocation();
    }
  return debug_readloop;
}


/* 
 * Local variables:
 * eval: (c-set-style "gnu")
 * indent-tabs-mode: nil
 * End:
 */
