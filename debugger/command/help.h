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
/* Debugger help command. */


void 
dbg_help_subcmd_entry(const char *psz_subcmd_name, const char *psz_fmt,
		      subcommand_var_info_t *p_subcmd)
{
    printf("%s ", psz_subcmd_name);
    printf(psz_fmt, p_subcmd->name, p_subcmd->doc );
    if (p_subcmd->var) {
	if (p_subcmd->b_onoff)
	    printf(" is %s.", 
		   var_to_on_off(* (int *) p_subcmd->var));
	else 
	    printf(" is %d.", *(int *)(p_subcmd->var));
    }
    printf("\n");
}

debug_return_t 
dbg_help_subcmd(const char *psz_subcmd_name, 
		short_cmd_t *p_command, const char *psz_args, 
		subcommand_var_info_t *subcommands)
{
    unsigned int i;
    if (!psz_args || !*psz_args) {
	printf("%s:\n\t%s\n", p_command->use, p_command->doc);
	printf ("Available info subcommands are:\n");
	for (i = 0; subcommands[i].name; i++) {
	    dbg_help_subcmd_entry(psz_subcmd_name, "%-10s -- %s", &(subcommands[i]));
	}
    } else {
	for (i = 0; subcommands[i].name; i++) {
	    if (is_abbrev_of(psz_args, subcommands[i].name, subcommands[i].min_abbrev)) {
		dbg_help_subcmd_entry(psz_subcmd_name, "%-10s -- %s", &(subcommands[i]));
		return debug_readloop;
	    }
	}
	printf("There is no \"%s %s\" command.\n", psz_subcmd_name, psz_args);
    }
    return debug_readloop;
}

debug_return_t 
dbg_cmd_help(char *psz_args)
{
  unsigned int i;

  if (!psz_args || !*psz_args) {
    printf ("  Command                  Short Name  Aliases\n");
    printf ("  ----------------------   ----------  ---------\n");
    for (i = 0; commands[i].long_name; i++) {
      unsigned int j;
      bool b_alias = false;
      uint8_t s=commands[i].short_name;
      printf("  %-31s (%c)", 
	     short_command[s].use, commands[i].short_name);
      for (j = 0; aliases[j].alias; j++) {
	if (strcmp (commands[i].long_name, aliases[j].command) == 0) {
	  if (!b_alias) {
	    printf("  %s", aliases[j].alias);
	    b_alias = true;
	  } else {
	    printf(", %s", aliases[j].alias);
	  }
	}
      }
      printf("\n");
    }

    printf("\nReadline command line editing (emacs/vi mode) is available.\n");
    printf("For more detailed help, type h <cmd> or consult "
	   "online-documentation.\n");
    
  } else {
      short_cmd_t *p_command;
      char *psz_command = "";
      
      if (1 == strlen(psz_args)) {
	  if ( NULL != short_command[(uint8_t)psz_args[0]].func ) 
	      p_command = &short_command[(uint8_t)psz_args[0]];
	  else
	      p_command = NULL;
      } else {
	  psz_command = get_word(&psz_args);
	  p_command = find_command (psz_command);
      }
      if (p_command) {
	  if ( p_command->func == &dbg_cmd_info ) {
	      return dbg_help_subcmd("info", p_command, psz_args, info_subcommands);
	  } else if ( p_command->func == &dbg_cmd_show ) {
	      return dbg_help_subcmd("show", p_command, psz_args, show_subcommands);
	  } else if ( p_command->func == &dbg_cmd_set ) {
	      return dbg_help_subcmd("set", p_command, psz_args, set_subcommands);
	  } else {
	      printf("%s\n\n", p_command->use);
	      printf("%s\n", p_command->doc);
	  }
      } else {
	  printf("Undefined command %s. Try help for a list of commands\n", 
		 psz_command);
      }
  }
  
  return debug_readloop;
}

static void
dbg_cmd_help_init(unsigned int c) 
{
  short_command[c].func = &dbg_cmd_help;
  short_command[c].use  = _("help [COMMAND]");
  short_command[c].doc  = 
    _("Display list of commands (i.e. this help text.)\n"		\
      "\twith an command name, give only the help for that command.");
}


/* 
 * Local variables:
 * eval: (c-set-style "gnu")
 * indent-tabs-mode: nil
 * End:
 */
