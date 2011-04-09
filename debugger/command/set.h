/* Set a variable definition with all variable references in the value
   part of psz_string expanded. */
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

/* Documentation for help set, and help set xxx. Note the format has
   been customized to make ddd work. In particular for "basename" it should
   be 
     set basename -- Set if were are to show shor or long filenames is off.
   (or "is on").
*/

#include "../subcmd.h"

subcommand_var_info_t set_subcommands[] = {
  { "basename", "Set if we are to show short or long filenames",
    &basename_filenames, true, 1},
  { "debug",    "Set GNU Make debug mask (set via --debug or -d).",
    &db_level, false, 1},
  { "ignore-errors", "Set value of GNU Make --ignore-errors (or -i) flag.",
    &ignore_errors_flag, true, 1},
  { "keep-going",    "Set value of GNU Make --keep-going (or -k) flag.",
    &keep_going_flag,    true, 1},
  { "silent",        "Set value of GNU Make --silent (or -s) flags.",
    &silent_flag,        true, 1},
  { "trace",        "Set value of shell_tracing.",
    &no_shell_trace,    false, 3},
  { "variable",      "Set a GNU Make variable.",
    NULL,                false, 0},
  { NULL, NULL, NULL, false, 0}
};

static debug_return_t 
dbg_cmd_set(char *psz_args) 
{
  if (!psz_args || 0==strlen(psz_args)) {
    printf(_("You need to supply a variable name\n"));
  } else {
    char *psz_varname = get_word(&psz_args);

    while (*psz_args && whitespace (*psz_args))
      *psz_args +=1;

    if (is_abbrev_of (psz_varname, "variable", 3)) {
      return dbg_cmd_set_var(psz_args, 3);
#if FIXME_SET_ARGS
    } else if (is_abbrev_of (psz_varname, "args", 3)) {
      ...
      }
#endif
    } else if (is_abbrev_of (psz_varname, "basename", 4)) {
      if (!psz_args || 0==strlen(psz_args))
	on_off_toggle("toggle", &basename_filenames);
      else
	on_off_toggle(psz_args, &basename_filenames);
      dbg_cmd_show("basename");
    } else if (is_abbrev_of (psz_varname, "debug", 3)) {
      int dbg_mask;
      if (get_int(psz_args, &dbg_mask, true)) {
	db_level = dbg_mask;
      }
    } else if (is_abbrev_of (psz_varname, "ignore-errors", 3)) {
      if (!psz_args || 0==strlen(psz_args))
	on_off_toggle("toggle", &ignore_errors_flag);
      else
	on_off_toggle(psz_args, &ignore_errors_flag);
      dbg_cmd_show("ignore_errors");
    } else if (is_abbrev_of (psz_varname, "keep-going", 3)) {
      if (!psz_args || !*psz_args)
	on_off_toggle("toggle", &keep_going_flag);
      else
	on_off_toggle(psz_args, &keep_going_flag);
      dbg_cmd_show("keep-going");
    } else if (is_abbrev_of (psz_varname, "silent", 3)) {
      if (!psz_args || !*psz_args)
	on_off_toggle("toggle", &silent_flag);
      else
	on_off_toggle(psz_args, &silent_flag);
      dbg_cmd_show("silent");
    } else if (is_abbrev_of (psz_varname, "trace", 3)) {
      if (!psz_args || !*psz_args)
	on_off_toggle("toggle", &no_shell_trace);
      else
	on_off_toggle(psz_args, &silent_flag);
      dbg_cmd_show("silent");
    } else {
      /* Treat as set variable */
      return dbg_cmd_set_var(psz_args, 1);
    }
  }
  return debug_readloop;
}

static void
dbg_cmd_set_init(unsigned int c) 
{
    
  short_command[c].func = &dbg_cmd_set;
  short_command[c].use =  
    _("set {*option*|variable} VALUE");
  short_command[c].doc  = 
    _("Change debugger setting or GNU Make variable\n"
      "If \"set\" is given by itself a list of \"set\" subcommands is listed.\n"
      );
}

/* 
 * Local variables:
 * eval: (c-set-style "gnu")
 * indent-tabs-mode: nil
 * End:
 */
