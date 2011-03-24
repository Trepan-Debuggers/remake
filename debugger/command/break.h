/* 
Set a breakpoint at a target.  With a target name, set a break before
running commands of that target.  Without argument, list all breaks.
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
dbg_cmd_break (char *psz_args)
{
  if (!psz_args || !*psz_args) {
    list_breakpoints();
    return debug_readloop;
  } else {
    char *psz_target = get_word(&psz_args);
    char *psz_break_type; 
    file_t *p_target;
    unsigned int i_brkpt_mask = BRK_ALL;
    
    p_target = lookup_file (psz_target);
    if (!p_target) {
	printf("Can't find target %s; breakpoint not set.\n", psz_target);
	return debug_cmd_error;
    }
    psz_break_type = get_word(&psz_args);
    if (is_abbrev_of (psz_break_type, "all", 1)) {
      i_brkpt_mask = BRK_ALL;
    } else if (is_abbrev_of (psz_break_type, "prerequisite", 3)) {
      i_brkpt_mask = BRK_BEFORE_PREREQ;
    } else if (is_abbrev_of (psz_break_type, "run", 1)) {
      i_brkpt_mask = BRK_BEFORE_PREREQ;
    } else if (is_abbrev_of (psz_break_type, "end", 1)) {
      i_brkpt_mask = BRK_AFTER_CMD;
    } else if (is_abbrev_of (psz_break_type, "temp", 1)) {
      i_brkpt_mask |= BRK_TEMP;
    }
    add_breakpoint(p_target, i_brkpt_mask);
  }

  return debug_readloop;
};

static void
dbg_cmd_break_init(void) 
{
  short_command['b'].func = &dbg_cmd_break;
  short_command['b'].use  = _("break TARGET [all|run|prereq]");
  short_command['b'].doc  = _("Set a breakpoint at a target.\n"
"With a target name, set a break before running commands\n"
"of that target.  Without argument, list all breaks.\n"
"There are 3 place where one may want to stop at:\n"
"  before prerequisite checking (prereq)\n"
"  after prerequisite checking but before running commands (run)\n"
"  after target is complete (end)\n"
                              );
}


/* 
 * Local variables:
 * eval: (c-set-style "gnu")
 * indent-tabs-mode: nil
 * End:
 */
