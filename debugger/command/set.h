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
static debug_return_t 
dbg_cmd_set(void) 
{
  char *psz_args = psz_debugger_args;
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
      psz_debugger_args = "basename";
      dbg_cmd_show();
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
      psz_debugger_args = "ignore_errors";
      dbg_cmd_show();
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
    } else {
      /* Treat as set variable */
      return dbg_cmd_set_var(psz_args, 1);
    }
  }
  return debug_readloop;
}
