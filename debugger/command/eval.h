/* Parse and evaluate buffer and return the results. 
*/
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
dbg_cmd_eval(void)
{
  char *psz_evalstring=psz_debugger_args;
#if 0
  if ('\0' == *psz_evalstring) {
    file_t *p_target;
    if (p_stack && p_stack->p_target) {
      p_target = p_stack->p_target;
    } else {
      printf(_("Default target not found here. You must supply one\n"));
    }

    if (p_target->name)
      printf(_("Running commands for target %s\n"), p_target->name);
    else
      printf(_("Running commands\n"), p_target->name);
    target_environment (p_target);
    new_job(p_target, p_stack);
  } else 
#else
  {
    func_eval(NULL, &psz_evalstring, NULL);
    reading_file = 0;

  }
#endif
  return debug_readloop;
}

static void
dbg_cmd_eval_init(void) 
{
  short_command['E'].func = &dbg_cmd_eval;
  short_command['E'].use  = _("eval STRING");
  short_command['E'].doc  = _("parse and evaluate a string.");

}

/* 
 * Local variables:
 * eval: (c-set-style "gnu")
 * indent-tabs-mode: nil
 * End:
 */
