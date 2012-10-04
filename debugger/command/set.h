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
  { "basename",
    "Set if we are to show short or long filenames",
    " {on|off|toggle} \n\nSet if we are to show short or long filenames.",
    &basename_filenames, true, 1},
  { "debug",
    "Set GNU Make debug mask (set via --debug or -d)",
    " VALUE \n\nSet GNU Make debug mask (set via --debug or -d).",
    &db_level, false, 3},
  { "ignore-errors", 
    "Set value of GNU Make --ignore-errors (or -i) flag",
    " {on|off|toggle} \n\nSet value of GNU Make --ignore-errors (or -i) flag.",
    &ignore_errors_flag, true, 3},
  { "keep-going",
    "Set value of GNU Make --keep-going (or -k) flag",
    " {on|off|toggle}\n\nSet value of GNU Make --keep-going (or -k) flag.",
    &keep_going_flag,    true, 1},
  { "silent",
    "Set value of GNU Make --silent (or -s) flags.",
    " {on|off|toggle} \n\nSet value of GNU Make --silent (or -s) flags.",
    &silent_flag,        true, 1},
#ifdef FIXED
  { "trace",
    "Set value of shell_tracing.",
    NULL, 
    &no_shell_trace,    false, 3},
#endif
  { "variable",      
    "Set a GNU Make variable VARIABLE",
    " VARIABLE VALUE\n\n"
"Set GNU Make variable VARIABLE to value VALUE\n"
"VALUE is expanded before assignment.\n"
"\n"
"See also 'setq'.",
    NULL,
    false, 0},
  { NULL, NULL, NULL, NULL, false, 0}
};

static bool
dbg_cmd_set_bool(const char *psz_varname, const char *psz_flag_name,
                 const char *psz_flag_value, 
                 unsigned int min, int *p_bool_flag) 
{
  if (is_abbrev_of (psz_varname, psz_flag_name, min)) {
      if (!psz_flag_value || 0==strlen(psz_flag_value))
	on_off_toggle(psz_flag_value, p_bool_flag);
      else
	on_off_toggle(psz_flag_value, p_bool_flag);
      dbg_cmd_show((char *) psz_flag_name);
      return true;
  }
  return false;
}


static debug_return_t 
dbg_cmd_set(char *psz_args) 
{
  if (!psz_args || 0==strlen(psz_args)) {
    unsigned int i;
    for (i = 0; set_subcommands[i].name; i++) {
      dbg_help_subcmd_entry("set", "%-10s -- %s", 
                            &(set_subcommands[i]), false);
    }
    return debug_readloop;
  } else {
    char *psz_varname = get_word(&psz_args);
    subcommand_var_info_t *p_subcmd_info;

    while (*psz_args && whitespace (*psz_args))
      *psz_args +=1;

    /* FIXME, add min to above table and DRY below code. */
    if (is_abbrev_of (psz_varname, "debug", 3)) {
      int dbg_mask;
      if (get_int(psz_args, &dbg_mask, true)) {
	db_level = dbg_mask;
      }
      dbg_cmd_show(psz_varname);
      return debug_readloop;
    } else if (is_abbrev_of (psz_varname, "variable", 3)) {
      /* Treat as set variable */
      return dbg_cmd_set_var(psz_args, 1);
#if FIXME_SET_ARGS
    } else if (is_abbrev_of (psz_varname, "args", 3)) {
        ...
#endif
    } else 
      for (p_subcmd_info = set_subcommands; *p_subcmd_info->name; 
           p_subcmd_info++) {
        if (dbg_cmd_set_bool(psz_varname, p_subcmd_info->name, 
                             psz_args, p_subcmd_info->min_abbrev,
                             p_subcmd_info->var))
          return debug_readloop;
      }
    dbg_errmsg("Unknown set option %s\n", psz_varname);
    return debug_cmd_error;
  }
}

static void
dbg_cmd_set_init(unsigned int c) 
{
    
  short_command[c].func = &dbg_cmd_set;
  short_command[c].use =  
    _("set OPTION {on|off|toggle}\n"
"  set variable VARIABLE VALUE    ");
  short_command[c].doc  = 
    _("In the first form, Set debugger value for OPTION.\n"
"In the second form, set a GNU make variable\n"
"Run `set' for a list of options and current values\n"
"\n"
"See also 'setq'."
      );
}

/* 
 * Local variables:
 * eval: (c-set-style "gnu")
 * indent-tabs-mode: nil
 * End:
 */
