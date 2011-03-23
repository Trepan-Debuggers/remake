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
/* Continue running program. */
static debug_return_t 
dbg_cmd_continue (void)
{
  char *psz_arg=psz_debugger_args;
  if (psz_arg && *psz_arg) {
    if (debug_cmd_error == dbg_cmd_break()) {
      printf(_("Not continuing under these circumstances.\n"));
      return debug_cmd_error;
    }
  } else  {
    db_level = 0;
  }

  i_debugger_stepping = 0;
  i_debugger_nexting  = 0;
  return continue_execution;
};

static void
dbg_cmd_continue_init(void) 
{
    short_command['c'].func = &dbg_cmd_continue;
    short_command['c'].use  = _("continue [TARGET]");
    short_command['c'].doc  = 
	_("Continue executing debugged Makefile until another breakpoint\n"
	  "or stopping point. If a target is given and valid we set a breakpoint at\n"
	  "that target before continuing.");
}

/* 
 * Local variables:
 * eval: (c-set-style "gnu")
 * indent-tabs-mode: nil
 * End:
 */
