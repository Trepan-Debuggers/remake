/* Terminate execution. */
/* 
Copyright (C) 2004, 2005, 2007, 2008, 2009, 2011 R. Bernstein 
<rocky@gnu.org>
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
static debug_return_t 
dbg_cmd_finish(void)
{
  char *psz_amount=psz_debugger_args;
  target_stack_node_t *p=p_stack;
  unsigned int i_amount=0;
  if ('\0' != *psz_debugger_args) {
    if (!get_uint(psz_amount, &i_amount))
      return debug_readloop;
    
    if (p_stack_top) {
      /* We have a target stack  */
      unsigned int i=0;
      
      for (i=0 ; p ; p = p->p_parent, i++ ) {
        if (i_amount == i) break;
      }
      
    }
  }
  if (p) {
    p->p_shared_target->tracing |= (BRK_AFTER_PREREQ|BRK_TEMP);
  }
  return continue_execution;
}

static void
dbg_cmd_finish_init(void) 
{
  short_command['F'].func = &dbg_cmd_finish;
  short_command['F'].use  = _("finish");
  short_command['F'].doc  = 
    _("Continue execution until the end of the Makefile without the "
      "usual tracing\n" 
      "\tracing that \"continue\" would give.");
}


/* 
 * Local variables:
 * eval: (c-set-style "gnu")
 * indent-tabs-mode: nil
 * End:
 */
