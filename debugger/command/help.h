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

    if (1 == strlen(psz_args)) {
      if ( NULL != short_command[(uint8_t)psz_args[0]].func ) 
	p_command = &short_command[(uint8_t)psz_args[0]];
      else
	p_command = NULL;
    } else {
      char *psz_command = get_word(&psz_args);
      p_command = find_command (psz_command);
    }
    if (p_command) {
      if ( p_command->func == &dbg_cmd_info ) {
	printf("  %s:\n\t%s\n", p_command->use, p_command->doc);
	printf ("\tAvailable info subcommands are:\n\t");
	for (i = 0; info_subcommands[i]; i++) {
	  printf(" %s", info_subcommands[i]);
	}
	printf("\n");
      } else if ( p_command->func == &dbg_cmd_show ) {
	for (i = 0; show_subcommands[i].name; i++) {
	  help_cmd_set_show("show %-15s -- %s", &(show_subcommands[i]));
	}
      } else if ( p_command->func == &dbg_cmd_set ) {
	if (!psz_args || !*psz_args) {
	  for (i = 0; set_subcommands[i].name; i++) {
	    help_cmd_set_show("set %s -- %s", &(set_subcommands[i]));
	  }
	} else {
	  for (i = 0; set_subcommands[i].name; i++) {
	    if ( !strcmp(psz_args, set_subcommands[i].name) ) {
	      help_cmd_set_show("set %s -- %s", &(set_subcommands[i]));
	      return debug_readloop;
	    }
	  }
	  printf("There is no \"set %s\" command.\n", psz_args);
	}
      } else {
	printf("%s\n\n", p_command->use);
	printf("%s\n", p_command->doc);
      }
      
    } else {
      printf("Undefined command %s. Try help for a list of commands\n", 
	     psz_args);
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
