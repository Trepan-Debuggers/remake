/* Show debugger settings. */
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
debug_return_t 
dbg_cmd_show(void)
{
  char *psz_arg = psz_debugger_args;
  if (!psz_arg || 0==strlen(psz_arg)) {
    unsigned int i;
    for (i=0; show_subcommands[i].name; i++) {
      if ( 0 == strcmp(show_subcommands[i].name, "warranty") ||
	   0 == strcmp(show_subcommands[i].name, "history"))
	continue;
      psz_debugger_args = (char *) show_subcommands[i].name;
      dbg_cmd_show();
    }
  } else {
    if (is_abbrev_of (psz_arg, "args", 3)) {
      print_cmdline();
    } else if (is_abbrev_of (psz_arg, "basename", 4)) {
      printf("basename is %s.\n", var_to_on_off(basename_filenames));
    } else if (is_abbrev_of (psz_arg, "debug", 3)) {
      printf("debug is 0x%x.\n", db_level);
      print_db_level(db_level);
    } else if (is_abbrev_of (psz_arg, "commands", 3)) {
      dbg_cmd_show_command(psz_arg);
    } else if (is_abbrev_of (psz_arg, "ignore-errors", 3)) {
      printf("ignore-errors is %s.\n", var_to_on_off(ignore_errors_flag));
    } else if (is_abbrev_of (psz_arg, "keep-going", 4)) {
      printf("keep-going is %s.\n", var_to_on_off(keep_going_flag));
    } else if (is_abbrev_of (psz_arg, "silent", 3)) {
      printf("silent is %s.\n", var_to_on_off(silent_flag));
    } else if (is_abbrev_of (psz_arg, "version", 3)) {
      printf("version: ");
      print_version();
    } else if (is_abbrev_of (psz_arg, "warranty", 3)) {
      printf("warranty: ");
      printf("%s", WARRANTY);
    } else {
      printf("Undefined command \"%s\". Try \"help show\"\n", psz_arg);
    }
  }
  
  return debug_readloop;
}
