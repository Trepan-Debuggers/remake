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

/* debugger command interface. */

#include "msg.h"
#include "debug.h"
#include "file.h"
#include "print.h"
#include "break.h"
#include "cmd.h"
#include "fns.h"
#include "function.h"
#include "info.h"
#include "stack.h"
#include "commands.h"
#include "expand.h"
#include "debug.h"

#ifdef HAVE_READLINE_READLINE_H
#include <readline/readline.h>
#endif

#ifdef HAVE_HISTORY_LIST
#include <readline/history.h>
#endif

/**
   Think of the below not as an enumeration but as #defines done in a
   way that we'll be able to use the value in a gdb. 
 **/
enum {
  MAX_FILE_LENGTH   = 1000,
} debugger_enum1;

  

/** True if we are inside the debugger, false otherwise. */
int in_debugger = false;

/**
   Command-line args after the command-name part. For example in:
   break foo
   the below will be "foo".
 **/ 
char *psz_debugger_args;

debug_enter_reason_t last_stop_reason;

#ifdef HAVE_LIBREADLINE
#include <stdio.h>
#include <stdlib.h>
/* The following line makes Solaris' gcc/cpp not puke. */
#undef HAVE_READLINE_READLINE_H
#include <readline/readline.h>

/* From readline. ?? Should this be in configure?  */
#ifndef whitespace
#define whitespace(c) (((c) == ' ') || ((c) == '\t'))
#endif

/* Common debugger command function prototype */
typedef debug_return_t (*dbg_cmd_t) (char *psz_args);

/* A structure which contains information on the commands this program
   can understand. */

typedef struct {
  dbg_cmd_t func;               /* Function to call to do the job. */
  const char *doc;		/* Documentation for this function.  */
  const char *use;		/* short command usage.  */
} short_cmd_t;

static debug_return_t dbg_cmd_set_var (char *psz_arg, int expand);

typedef struct {
  const char *long_name;	/* long name of command. */
  const char short_name;	/* Index into short_cmd array. */
} long_cmd_t;

/* Should be in alphabetic order by command name. */
long_cmd_t commands[] = {
  { "break",    'b' },
  { "cd",       'C' },
  { "comment",  '#' },
  { "continue", 'c' },
  { "delete",   'd' },
  { "down",     'D' },
  { "edit" ,    'e' },
  { "eval" ,    'E' },
  { "expand" ,  'x' },
  { "finish"  , 'F' },
  { "frame"   , 'f' },
  { "help"    , 'h' },
  { "info"    , 'i' },
  { "list"    , 'l' },
  { "next"    , 'n' },
  { "print"   , 'p' },
  { "pwd"     , 'P' },
  { "quit"    , 'q' },
  { "run"     , 'R' },
  { "set"     , '=' },
  { "setq"    , '"' },
  { "shell"   , '!' },
  { "show"    , 'S' },
  { "source"  , '<' },
  { "skip"    , 'k' },
  { "step"    , 's' },
  { "target"  , 't' },
  { "up"      , 'u' },
  { "where"   , 'T' },
  { "write"   , 'w' },
  { (char *)NULL, ' '}
};

typedef struct {
  const char *command;	        /* real command name. */
  const char *alias;	        /* alias for command. */
} alias_cmd_t;

/* Should be in alphabetic order by ALIAS name. */

alias_cmd_t aliases[] = {
  { "shell",    "!!" },
  { "help",     "?" },
  { "help",     "??" },
  { "break",    "L" },
  { "where",    "backtrace" },
  { "where",    "bt" },
  { "quit",     "exit" },
  { "run",      "restart" },
  { "quit",     "return" },
  { (char *)NULL, (char *) NULL}
};


short_cmd_t short_command[256] = { { NULL, '\0', '\0' }, };

typedef struct {
  const char *name;	/* name of subcommand command. */
  const char *doc;	/* short description of subcommand */
} subcommand_info_t;

typedef struct {
  const char *name;	/* name of subcommand command. */
  const char *doc;	/* short description of subcommand */
  void *var;	        /* address of variable setting. NULL if no
			   setting. */
  bool b_onoff;         /* True if on/off variable, false if int. 
			   FIXME: generalize into enumeration.
			 */
  unsigned int min_abbrev; /* Fewest number of characters needed
                              to match name. */
} subcommand_var_info_t;

subcommand_var_info_t show_subcommands[] = {
  { "args",     "Show argument list to give program when it is started",
    NULL, false, 1},
  { "basename", "Show if we are to show short or long filenames",
    &basename_filenames, true, 1},
  { "commands", "Show the history of commands you typed.",
    NULL, false, 1},
  { "debug",    "GNU Make debug mask (set via --debug or -d)",
    &db_level, false, 1},
  { "ignore-errors", "Value of GNU Make --ignore-errors (or -i) flag",
    &ignore_errors_flag, true, 1},
  { "keep-going",    "Value of GNU Make --keep-going (or -k) flag",
    &keep_going_flag,    true, 1},
  { "silent",        "Value of GNU Make --silent (or -s) flags",
    &silent_flag,        true, 1},
  { "version",       "Show the version of GNU Make + dbg.",
    NULL,                false, 1},
  { "warranty",      "Various kinds of warranty you do not have.",
    NULL,                false, 1},
  { NULL, NULL, NULL,    false, 0}
};

/* Documentation for help set, and help set xxx. Note the format has
   been customized to make ddd work. In particular for "basename" it should
   be 
     set basename -- Set if were are to show shor or long filenames is off.
   (or "is on").
*/
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

subcommand_var_info_t info_subcommands[] = {
  { "break",     "Show list of target breakpoints",
    NULL, false, 1},
  { "line", "Show line and Makefile name of where we are currently stopped ",
    NULL, true, 2},
  { "locals", "Show target local variables and their values",
    NULL, true, 2},
  { "files",    "Show read-in Makefiles. The last is the one initially named",
    NULL, false, 2},
  { "frame",    "Show target-stack frame",
    NULL, false, 2},
  { "rules", "Value of GNU Make --ignore-errors (or -i) flag",
    NULL, true, 1},
  { "program",    "Show program information and why we are stopped",
    NULL,    true, 1},
  { "targets", "Show the explicitly-named targets found in read Makefiles.",
    NULL, false, 1},
  { "variables",  "Show all GNU Make variables",
    NULL,        true, 2},
  { "warranty",      "Various kinds of warranty you do not have.",
    NULL,                false, 1},
  { NULL, NULL, NULL,    false, 0}
};

/* Look up NAME as the name of a command, and return a pointer to that
   command.  Return a NULL pointer if NAME isn't a command name. */
static short_cmd_t *
find_command (const char *psz_name)
{
  unsigned int i;

  for (i = 0; aliases[i].alias; i++) {
    const int cmp = strcmp (psz_name, aliases[i].alias);
    if ( 0 == cmp ) {
      psz_name = aliases[i].command;
      break;
    } else 
      /* Words should be in alphabetic order by alias name.
	 Have we gone too far? */
      if (cmp < 0) break;
  }
  
  for (i = 0; commands[i].long_name; i++) {
    const int cmp = strcmp (psz_name, commands[i].long_name);
    if ( 0 == cmp ) {
      return (&short_command[(uint8_t) commands[i].short_name]);
    } else 
      /* Words should be in alphabetic order by command name.
	 Have we gone too far? */
      if (cmp < 0) break;
  }

  return ((short_cmd_t *)NULL);
}

#include "command/break.h"
#include "command/chdir.h"
#include "command/comment.h"
#include "command/continue.h"
#include "command/delete.h"
#include "command/down.h"
#include "command/edit.h"
#include "command/eval.h"
#include "command/expand.h"
#include "command/finish.h"
#include "command/frame.h"
#include "command/info.h"
#include "command/next.h"
#include "command/list.h"
#include "command/print.h"
#include "command/pwd.h"
#include "command/quit.h"
#include "command/run.h"
#include "command/set.h"
#include "command/setq.h"
#include "command/shell.h"
#include "command/show.h"
#include "command/skip.h"
#include "command/source.h"
#include "command/step.h"
#include "command/target.h"
#include "command/up.h"
#include "command/where.h"
#include "command/write.h"

/* Needs to come after dbg_cmd_show */
#include "command/help.h"

static void 
cmd_initialize(void) 
{
  dbg_cmd_break_init   ('b');
  dbg_cmd_chdir_init   ('C');
  dbg_cmd_continue_init('c');
  dbg_cmd_delete_init  ('d');
  dbg_cmd_down_init    ('D');
  dbg_cmd_edit_init    ('e');
  dbg_cmd_eval_init    ('E');
  dbg_cmd_finish_init  ('F');
  dbg_cmd_frame_init   ('f');
  dbg_cmd_help_init    ('h');
  dbg_cmd_info_init    ('i');
  dbg_cmd_skip_init    ('k');
  dbg_cmd_list_init    ('l');
  dbg_cmd_next_init    ('n');
  dbg_cmd_print_init   ('p');
  dbg_cmd_pwd_init     ('P');
  dbg_cmd_quit_init    ('q');
  dbg_cmd_run_init     ('R');
  dbg_cmd_source_init  ('<');
  dbg_cmd_show_init    ('S');
  dbg_cmd_step_init    ('s');
  dbg_cmd_target_init  ('t');
  dbg_cmd_up_init      ('u');
  dbg_cmd_where_init   ('T');
  dbg_cmd_write_init   ('w');
  dbg_cmd_expand_init  ('x');
  dbg_cmd_comment_init ('#');
  dbg_cmd_set_init     ('=');
  dbg_cmd_setq_init    ('"');
  dbg_cmd_shell_init   ('!');
}

/* Execute a command line. */
extern debug_return_t
execute_line (char *psz_line)
{
  unsigned int i = 0;
  short_cmd_t *command;
  char *psz_word = get_word(&psz_line);

  if (1 == strlen(psz_word)) {
    if ( NULL != short_command[(uint8_t) psz_word[0]].func ) 
      command = &short_command[(uint8_t) psz_word[0]];
    else
      command = NULL;
  } else {
    command = find_command (psz_word);
  }
  if (!command)
    {
      dbg_errmsg(_("No such debugger command: %s."), psz_word);
      return debug_readloop;
    }

  /* Get argument to command, if any. */
  while (whitespace (psz_line[i]))
    i++;

  psz_debugger_args = psz_line + i;

  /* Call the function. */
  return ((*(command->func)) (psz_debugger_args));
}

/* Show history. */
debug_return_t 
dbg_cmd_show_command (char *psz_args)
{
 /*
  if (!psz_arg || *psz_arg) {
    ;
    } */

#ifdef HAVE_HISTORY_LIST
  HIST_ENTRY **hist_list = history_list();
  unsigned int i;
  UNUSED_ARGUMENT(psz_args);
  if (!hist_list) return debug_readloop;
  for (i=0; hist_list[i]; i++) {
    dbg_msg("%5d  %s", i, hist_list[i]->line);
  }
#endif
  return debug_readloop;
}

/* Set a variable. Set "expand' to 1 if you want variable 
   definitions inside the value getting passed in to be expanded
   before assigment. */
static debug_return_t dbg_cmd_set_var (char *psz_args, int expand) 
{
  if (!psz_args || 0==strlen(psz_args)) {
    dbg_msg(_("You need to supply a variable name."));
  } else {
    variable_t *p_v;
    char *psz_varname = get_word(&psz_args);
    unsigned int u_len = strlen(psz_varname);

    while (*psz_args && whitespace (*psz_args))
      *psz_args +=1;

    p_v = lookup_variable (psz_varname, u_len);

    if (p_v) {
      char *psz_value =  expand ? variable_expand(psz_args) : psz_args;
      
      define_variable_in_set(p_v->name, u_len, psz_value,
			     o_debugger, 0, NULL,
			     &(p_v->fileinfo));
      dbg_msg(_("Variable %s now has value '%s'\n"), psz_varname,
	     psz_value);
    } else {
      try_without_dollar(psz_varname);
    }
  }
  return debug_readloop;
}

#endif /* HAVE_LIBREADLINE */

#define PROMPT_LENGTH 300

/* Should be less that PROMPT_LENGTH / 2 - strlen("remake ") + log(history) 
   We will make it much less that since people can't count more than
   10 or so nested <<<<>>>>'s easily.
*/
#define MAX_NEST_DEPTH 10

debug_return_t enter_debugger (target_stack_node_t *p, 
			       file_t *p_target, int errcode,
			       debug_enter_reason_t reason)
{
  debug_return_t debug_return = debug_readloop;
  static int i_init = 0;
  char open_depth[MAX_NEST_DEPTH];
  char close_depth[MAX_NEST_DEPTH];
  unsigned int i = 0;

  last_stop_reason = reason;
  
  if ( in_debugger == DEBUGGER_QUIT_RC ) {
    return continue_execution;
  }
  
  if ( i_debugger_stepping > 1 || i_debugger_nexting > 1 ) {
    /* Don't stop unless we are here from a breakpoint. But
       do decrement the step count. */
    if (i_debugger_stepping)  i_debugger_stepping--;
    if (i_debugger_nexting)   i_debugger_nexting--;
    if (!p_target->tracing) return continue_execution;
  } else if ( !debugger_on_error 
	      && !(i_debugger_stepping || i_debugger_nexting) 
	      && p_target && !p_target->tracing && -2 != errcode )
    return continue_execution;
  
  /* Clear temporary breakpoints. */
  if (p_target && p_target->tracing & BRK_TEMP)
    switch(last_stop_reason) 
      {
      case DEBUG_BRKPT_AFTER_CMD:
      case DEBUG_BRKPT_BEFORE_PREREQ:
      case DEBUG_BRKPT_AFTER_PREREQ:
        p_target->tracing = BRK_NONE;
      default:
        ;
      }

  if (0 == i_init) {
#ifdef HAVE_LIBREADLINE
    rl_initialize ();
    using_history ();
    add_history ("");
#endif
    cmd_initialize();
    i_init = 1;
  }


  /* Set initial frame position reporting area: 0 is bottom. */
  p_target_loc    = NULL;
  psz_target_name = "";
  i_stack_pos      = 0;

  p_stack = p_stack_top = p;
  p_floc_stack = p_stack_floc_top;

  /* Get the target name either from the stack top (preferred) or
     the passed in target.
   */
  if (p && p->p_target) {
    p_target_loc    = &(p->p_target->floc);  
    psz_target_name = (char *) p->p_target->name;
  } else if (p_target) {
    p_target_loc    = &(p_target->floc);
    psz_target_name = (char *) p_target->name;
  }

  for (i=0; i<=makelevel && i < MAX_NEST_DEPTH-5; i++) {
    open_depth[i]  = '<';
    close_depth[i] = '>';
  }

  if ( MAX_NEST_DEPTH - 5 == i ) {
    close_depth[i] = open_depth[i]  = '.'; i++;
    close_depth[i] = open_depth[i]  = '.'; i++;
    close_depth[i] = open_depth[i]  = '.'; i++;
  }
  
  open_depth[i] = close_depth[i] = '\0';

  in_debugger = true;

  if (errcode) {
    if (-1 == errcode) {
      printf("\n***Entering debugger because we encountered an error.\n");
    } else if (-2 == errcode) {
      if (0 == makelevel) {
	printf("\nMakefile terminated.\n");
	dbg_msg("Use q to quit or R to restart");
      } else {
	printf("\nMakefile finished at level %d. Use R to restart\n", 
	       makelevel);
	dbg_msg("the makefile at this level or 's', 'n', or 'F' to continue "
	       "in parent");
	in_debugger = DEBUGGER_QUIT_RC;
      }
    } else {
      printf("\n***Entering debugger because we encountered a fatal error.\n");
      dbg_errmsg("Exiting the debugger will exit make with exit code %d.", 
                 errcode);
    }
  }

  print_debugger_location(p_target, reason, NULL);

  /* Loop reading and executing lines until the user quits. */
  for ( debug_return = debug_readloop; 
	debug_return == debug_readloop || debug_return == debug_cmd_error; ) {
    char prompt[PROMPT_LENGTH];
    char *line=NULL;
    char *s;
    
#ifdef HAVE_LIBREADLINE
    if (use_readline_flag) {
      snprintf(prompt, PROMPT_LENGTH, "remake%s%d%s ", 
               open_depth, where_history(), close_depth);
      
      line = readline (prompt);
    } else 
#endif
      {
        snprintf(prompt, PROMPT_LENGTH, "remake%s0%s ", open_depth, 
                 close_depth);
        printf("%s", prompt);
        if (line == NULL) line = calloc(1, 2048);
        line = fgets(line, 2048, stdin);
        chomp(line);
      }

    if ( line ) {
      if ( *(s=stripwhite(line)) ) {
	add_history (s);
	debug_return=execute_line(s);
      } else {
	add_history ("step");
	debug_return=dbg_cmd_step("");
      }
      free (line);
    } else {
      dbg_cmd_quit(NULL);
    }
  }

  if (in_debugger != DEBUGGER_QUIT_RC)
    in_debugger=false;
  
  return debug_return;
}

/* 
 * Local variables:
 * eval: (c-set-style "gnu")
 * indent-tabs-mode: nil
 * End:
 */
