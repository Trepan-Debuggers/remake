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

#include "make.h"
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

/* Think of the below not as an enumeration but as #defines done in a
   way that we'll be able to use the value in a gdb. */
enum {
  MAX_FILE_LENGTH   = 1000,
} debugger_enum1;

  

/* True if we are inside the debugger, false otherwise. */
int in_debugger = false;

debug_enter_reason_t last_stop_reason;

#ifdef HAVE_LIBREADLINE
#include <stdio.h>
#include <stdlib.h>
/* The following line makes Solaris' gcc/cpp not puke. */
#undef HAVE_READLINE_READLINE_H
#include <config/readline.h>

/* From readline. ?? Should this be in configure?  */
#ifndef whitespace
#define whitespace(c) (((c) == ' ') || ((c) == '\t'))
#endif

/* Common debugger command function prototype */
typedef debug_return_t (*dbg_cmd_t) (char *);

/* A structure which contains information on the commands this program
   can understand. */

typedef struct {
  dbg_cmd_t func;               /* Function to call to do the job. */
  const char *doc;		/* Documentation for this function.  */
  const char *use;		/* short command usage.  */
} short_cmd_t;

static debug_return_t dbg_cmd_set_var (char *psz_arg, int expand);

static debug_return_t dbg_cmd_break            (char *psz_arg);
static debug_return_t dbg_cmd_chdir            (char *psz_arg);
static debug_return_t dbg_cmd_comment          (char *psz_arg);
static debug_return_t dbg_cmd_continue         (char *psz_arg);
static debug_return_t dbg_cmd_delete           (char *psz_arg);
static debug_return_t dbg_cmd_eval             (char *psz_arg);
static debug_return_t dbg_cmd_expand           (char *psz_arg);
static debug_return_t dbg_cmd_finish           (char *psz_arg);
static debug_return_t dbg_cmd_next             (char *psz_arg);
static debug_return_t dbg_cmd_list             (char *psz_arg);
static debug_return_t dbg_cmd_pwd              (char *psz_arg);
static debug_return_t dbg_cmd_print            (char *psz_arg);
static debug_return_t dbg_cmd_quit             (char *psz_arg);
static debug_return_t dbg_cmd_run              (char *psz_arg);
static debug_return_t dbg_cmd_set              (char *psz_arg);
static debug_return_t dbg_cmd_set_var_noexpand (char *psz_arg);
static debug_return_t dbg_cmd_shell            (char *psz_arg);
static debug_return_t dbg_cmd_show             (char *psz_arg);
static debug_return_t dbg_cmd_show_command     (char *psz_arg);
static debug_return_t dbg_cmd_skip             (char *psz_arg);
static debug_return_t dbg_cmd_step             (char *psz_arg);
static debug_return_t dbg_cmd_write_cmds       (char *psz_arg);

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
  { "eval" ,    'e' },
  { "examine" , 'x' },
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

char *info_subcommands[] = {
  "break",
  "line",
  "locals",
  "files",
  "frame",
  "makefiles",
  "program",
  "rules",
  "target",
  "variables",
  "warranty",
  NULL
};


typedef struct {
  const char *name;	/* name of subcommand command. */
  const char *doc;	/* short description of subcommand */
  void *var;	        /* address of variable setting. NULL if no
			   setting. */
  bool b_onoff;         /* True if on/off variable, false if int. 
			   FIXME: generalize into enumeration.
			 */
} subcommand_var_info_t;

subcommand_var_info_t show_subcommands[] = {
  { "args",     "Show argument list to give program when it is started",
    NULL, false},
  { "basename", "Show if we are to show short or long filenames",
    &basename_filenames, true},
  { "commands", "Show the history of commands you typed.",
    NULL, false},
  { "debug",    "GNU Make debug mask (set via --debug or -d)",
    &db_level, false},
  { "ignore-errors", "Value of GNU Make --ignore-errors (or -i) flag",
    &ignore_errors_flag, true},
  { "keep-going",    "Value of GNU Make --keep-going (or -k) flag",
    &keep_going_flag,    true},
  { "silent",        "Value of GNU Make --silent (or -s) flags",
    &silent_flag,        true},
  { "version",       "Show the version of GNU Make + dbg.",
    NULL,                false},
  { "warranty",      "Various kinds of warranty you do not have.",
    NULL,                false},
  { NULL, NULL, NULL,    false}
};

/* Documentation for help set, and help set xxx. Note the format has
   been customized to make ddd work. In particular for "basename" it should
   be 
     set basename -- Set if were are to show shor or long filenames is off.
   (or "is on").
*/
subcommand_var_info_t set_subcommands[] = {
  { "basename", "Set if we are to show short or long filenames",
    &basename_filenames, true},
  { "debug",    "Set GNU Make debug mask (set via --debug or -d)",
    &db_level, false},
  { "ignore-errors", "Set value of GNU Make --ignore-errors (or -i) flag",
    &ignore_errors_flag, true},
  { "keep-going",    "Set value of GNU Make --keep-going (or -k) flag",
    &keep_going_flag,    true},
  { "silent",        "Set value of GNU Make --silent (or -s) flags",
    &silent_flag,        true},
  { "variable",      "Set a GNU Make variable",
    NULL,                false},
  { NULL, NULL, NULL, false }
};

static void 
cmd_initialize(void) 
{
  short_command['b'].func = &dbg_cmd_break;
  short_command['b'].use  = _("break TARGET");
  short_command['b'].doc  = _("Set a breakpoint at a target.\n"
"With a target name, set a break before running commands\n"
"of that target.  Without argument, list all breaks.");

  short_command['C'].func = &dbg_cmd_chdir;
  short_command['C'].use  = _("cd DIR");
  short_command['C'].doc  = 
    _("Set the working directory to DIR.");

  short_command['c'].func = &dbg_cmd_continue;
  short_command['c'].use  = _("continue [TARGET]");
  short_command['c'].doc  = 
    _("Continue executing debugged Makefile until another breakpoint\n"
"or stopping point. If a target is given and valid we set a breakpoint at\n"
"that target before continuing.");

  short_command['d'].func = &dbg_cmd_delete;
  short_command['d'].use  = _("delete breakpoint numbers..");
  short_command['d'].doc  = _("Delete some breakpoints\n."
"Arguments are breakpoint numbers with spaces in between.\n"
"To delete all breakpoints, give no argument.");

  short_command['D'].func = &dbg_cmd_frame_down;
  short_command['D'].use  = _("down [AMOUNT]");
  short_command['D'].doc  = 
    _("Select and print the target this one caused to be examined.\n"
      "\tAn argument says how many targets down to go.");

  short_command['e'].func = &dbg_cmd_eval;
  short_command['e'].use  = _("eval STRING");
  short_command['e'].doc  = _("parse and evaluate a string.");

  short_command['F'].func = &dbg_cmd_finish;
  short_command['F'].use  = _("finish");
  short_command['F'].doc  = 
    _("Continue execution until the end of the Makefile without the "
      "usual tracing\n" 
      "\tracing that \"continue\" would give.");

  short_command['f'].func = &dbg_cmd_frame;
  short_command['f'].use  = _("frame N");
  short_command['f'].doc  = 
    _("Move target frame to N; In contrast to \"up\" or \"down\",\n"
      "\tthis sets to an absolute position. 0 is the top.");

  short_command['h'].func = &dbg_cmd_help;
  short_command['h'].use  = _("help [COMMAND]");
  short_command['h'].doc = 
    _("Display list of commands (i.e. this help text.)\n"		\
      "\twith an command name, give only the help for that command.");

  short_command['i'].func = &dbg_cmd_info;
  short_command['i'].use = _("info [THING]");
  short_command['i'].doc = 
    _("Show the state of thing.\n" \
      "\tIf no 'thing' is specified, show everything there is to show.");

  short_command['k'].func = &dbg_cmd_skip;
  short_command['k'].use = _("skip");
  short_command['k'].doc = 
    _("Skip execution of next command or action." );

  short_command['l'].func = &dbg_cmd_list;
  short_command['l'].use = _("list [TARGET]");
  short_command['l'].doc = 
    _("List target dependencies and commands. Without a target name we\n"
"use the current target. A target name of '-' will use the parent target on\n"
"the target stack.\n"
 );

  short_command['n'].func = &dbg_cmd_next;
  short_command['n'].use = _("next [AMOUNT]");
  short_command['n'].doc = 
    _("Continue until the next command to be executed.\n"
      "Argument AMOUNT means do this AMOUNT times (or until there's another\n"
      "reason to stop.");

  short_command['p'].func = &dbg_cmd_print;
  short_command['p'].use = _("print {VARIABLE [attrs...]}");
  short_command['p'].doc = 
    _("Show a variable definition.\n"
      "The value is shown with embedded\n"
      "variable-references unexpanded. Don't include $ before a variable\n"
      "name. See also \"examine\".\n\n"
      "If no variable is supplied, we try to use the\n"
      "last value given."				
      );

  short_command['P'].func = &dbg_cmd_pwd;
  short_command['P'].use = _("Print working directory");
  short_command['P'].doc = 
    _("Print working directory.  This is used for your program as well.");

  short_command['q'].func = &dbg_cmd_quit;
  short_command['q'].use = _("quit [exit-status]");
  short_command['q'].doc = 
    _("Exit make. If a numeric argument is given, it will be the exit\n"
      "status reported back. A status of 77 in a nested make will signals\n"
      "termination in the parent. So if no numeric argument is given and\n"
      "MAKELEVEL is 0, then status 0 is set; otherwise it is 77."
      );

  short_command['R'].func = &dbg_cmd_run;
  short_command['R'].doc = _("Run Makefile from the beginning.\n"
   "You may specify arguments to give it.\n" \
   "With no arguments, uses arguments last specified (with \"run\")");
  short_command['R'].use = _("run");

  short_command['s'].func = &dbg_cmd_step;
  short_command['s'].use = _("step [AMOUNT]");
  short_command['s'].doc = 
    _("Step execution until another stopping point is reached.\n"
      "Argument AMOUNT means do this AMOUNT times (or until there's another\n"
      "reason to stop.");

  short_command['S'].func = &dbg_cmd_show;
  short_command['S'].use = _("show [thing]");
  short_command['S'].doc = 
    _("Show the state of thing.\n" \
      "If no 'thing' is specified, show everything there is to show.");

  short_command['t'].func = &dbg_cmd_target;
  short_command['t'].use = _("target [target-name] [info1 [info2...]]");
  short_command['t'].doc = 
    _("Show information about a target.\n" 
      "target information is printed.\n"
      "The following attributes names can be given after a target name:\n"
      "\t'attributes', 'commands', 'expand', 'depends', 'nonorder',\n"
      "\t'previous', 'state', 'time', 'variables'\n"
      "If no variable or target name is supplied, we try to use the\n"
      "current target name.\n"				
      );

  short_command['T'].func = &dbg_cmd_where;
  short_command['T'].use  = _("where");
  short_command['T'].doc  = 
    _("Print target stack or Makefile include stack.\n" \
      "An argument specifies the maximum amount of entries to show.");

  short_command['u'].func = &dbg_cmd_frame_up;
  short_command['u'].use  = _("up [AMOUNT]");
  short_command['u'].doc  = 
    _("Select and print target that caused this one to be examined.\n"
      "An argument says how many targets up to go.");

  short_command['w'].func = &dbg_cmd_write_cmds;
  short_command['w'].use =  _("write [TARGET [FILENAME]]");
  short_command['w'].doc  = 
    _("writes the commands associated of a target to a file with MAKE\n"
      "variables expanded. If no target given, the basename of the current\n"
      "is used. If a filename is supplied it is used. If it is the string\n"
      "\"here\", we write the output to stdout. If no filename is\n"
      "given then create the filename by prepending a directory name to\n"
      "the target name and then append \".sh\".");

  short_command['x'].func = &dbg_cmd_expand;
  short_command['x'].use =  _("examine STRING");
  short_command['x'].doc  = 
    _("Show string with internal variables references expanded. See also \n"
      "\t\"print\".");

  short_command['#'].func = &dbg_cmd_comment;
  short_command['#'].use =  _("comment TEXT");
  short_command['#'].doc  = 
    _("Ignore this line.");

  short_command['!'].func = &dbg_cmd_shell;
  short_command['!'].use =  _("shell STRING");
  short_command['!'].doc  = 
    _("Execute the rest of the line as a shell.");

  short_command['='].func = &dbg_cmd_set;
  short_command['='].use =  
    _("set {*option*|variable} VALUE");
  short_command['='].doc  = 
    _("set basename {on|off|toggle} - show full name or basename?\n"
      "set debug debug-mask - like --debug value.\n\n"
      "set ignore-errors {on|off|toggle} - like --ignore-errors option\n\n"
      "set keep-going {on|off|toggle} - like --keep-going option\n\n"
      "set silent {on|off|toggle} - like --silent option\n\n"
      "set trace {on|off|toggle} - set tracing status\n"
      "set variable *var* *value*\n"
      "Set MAKE variable to value. Variable definitions\n"
      "inside VALUE are expanded before assignment occurs."
      );

  short_command['"'].func = &dbg_cmd_set_var_noexpand;
  short_command['"'].use =  _("setq *variable* VALUE");
  short_command['"'].doc  = 
    _("Set MAKE variable to value. Variable definitions\n"
      "\tinside VALUE are not expanded before assignment occurs.");
}

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

/* Execute a command line. */
static debug_return_t
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
      fprintf (stderr, _("No such debugger command: %s.\n"), psz_word);
      return debug_readloop;
    }

  /* Get argument to command, if any. */
  while (whitespace (psz_line[i]))
    i++;

  psz_word = psz_line + i;

  /* Call the function. */
  return ((*(command->func)) (psz_word));
}

void 
help_cmd_set_show(const char *psz_fmt, subcommand_var_info_t *p_subcmd) 
{
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

#include "command/break.h"
#include "command/chdir.h"
#include "command/comment.h"
#include "command/continue.h"
#include "command/delete.h"

/* Parse and evaluate buffer and return the results. */
static debug_return_t 
dbg_cmd_eval (char *psz_evalstring)
{
  func_eval(NULL, &psz_evalstring, NULL);
  reading_file = 0;
  return debug_readloop;
}

/* Show a string with variable references expanded. */
static debug_return_t 
dbg_cmd_expand (char *psz_string) 
{
  static char *psz_last_string = NULL;

  if (!psz_string || !*psz_string) {
    /* Use last target value */
    if (psz_last_string)
      psz_string = psz_last_string;
    else {
      printf("No current expand string - must supply something to print\n");
      return debug_readloop;
    }
  }
  
  if (dbg_cmd_show_exp(psz_string, true)) {
    if (psz_last_string) free(psz_last_string);
    psz_last_string = strdup(psz_string);
  }
  return debug_readloop;
}

/* Terminate execution. */
static debug_return_t 
dbg_cmd_finish (char *psz_arg)
{
  i_debugger_stepping = 0;
  i_debugger_nexting  = 0;
  db_level            = 0;
  return continue_execution;
}

#include "command/help.h"
#include "command/list.h"

/* Continue until the next command to be executed. */
static debug_return_t 
dbg_cmd_next (char *psz_arg)
{

  if (!psz_arg || !*psz_arg) {
    i_debugger_nexting  = 1;
    i_debugger_stepping = 0;
    return continue_execution;
  } 
  if (get_uint(psz_arg, &i_debugger_nexting)) 
    return continue_execution;
  else 
    return continue_execution;
}

/* Show a variable definition. */
static debug_return_t 
dbg_cmd_print (char *psz_args) 
{
  char   *psz_name;
  static char *psz_last_name = NULL;

  if (!psz_args || 0==strlen(psz_args)) {
    /* Use last value */
    if (psz_last_name)
      psz_name = psz_last_name;
    else {
      printf("No current variable - must supply something to print\n");
      return debug_readloop;
    }
  } else {
    psz_name = get_word(&psz_args);
  }
  
  if (dbg_cmd_show_exp(psz_name, false)) {
    if (psz_last_name) free(psz_last_name);
    psz_last_name = strdup(psz_name);
  }

  return debug_readloop;
}

/* Print working directory. */
static debug_return_t 
dbg_cmd_pwd (char *psz_args) 
{
  if (!psz_args || 0==strlen(psz_args)) {
    char wd[300];
    if (NULL == getcwd (wd, sizeof(wd))) {
      printf (_("cannot get current directory %s\n"), strerror(errno));
    } else {
      printf (_("Working directory %s.\n"), wd);
    }
  } else {
    printf(_("The \"pwd\" does not take an argument: %s\n"), psz_args);
  }

  return debug_readloop;
}

/* Terminate execution. */
static debug_return_t 
dbg_cmd_quit (char *psz_arg)
{
  if (!psz_arg || !*psz_arg) {
    in_debugger = DEBUGGER_QUIT_RC;
    die(DEBUGGER_QUIT_RC);
  } else {
    int rc;
    if (get_int(psz_arg, &rc, true)) 
      die(rc);
  }
  return debug_readloop;
}

/* Restart/run program. */
static debug_return_t 
dbg_cmd_run (char *psz_arg)
{
  char **ppsz_argv = global_argv;
  const char *psz_make_cmd = global_argv[0];
  int rc;
  printf("Changing directory to %s and restarting...\n", 
	 directory_before_chdir);
  rc = chdir (directory_before_chdir);
  if (psz_arg && strlen(psz_arg)) {
    unsigned int len = strlen(global_argv[0]) + strlen(psz_arg) + 2;
    char *psz_full_args = CALLOC(char, len);
    snprintf(psz_full_args, len, "%s %s", global_argv[0], psz_arg);
    ppsz_argv = buildargv(psz_full_args);
    free(psz_full_args);
  }
  execvp (psz_make_cmd, ppsz_argv);
  /* NOT USED: */
  return debug_readloop;
}

/* Show history. */
static debug_return_t 
dbg_cmd_show_command (char *psz_arg)
{
  /*
  if (!psz_arg || *psz_arg) {
    ;
    } */

#ifdef HAVE_HISTORY_LIST
  HIST_ENTRY **hist_list = history_list();
  unsigned int i;
  if (!hist_list) return debug_readloop;
  for (i=0; hist_list[i]; i++) {
    printf("%5d  %s\n", i, hist_list[i]->line);
  }
#endif
  return debug_readloop;
}

/* Show target call stack info. */
debug_return_t 
dbg_cmd_where (char *psz_amount)
{
  int i_amount;
  
  if (!psz_amount || !*psz_amount) {
    i_amount = MAX_STACK_SHOW;
  } else if (!get_int(psz_amount, &i_amount, true)) {
      return debug_readloop;
  }

  if (p_stack_top)
    print_target_stack (p_stack_top, i_stack_pos, i_amount);

  if (p_stack_floc_top) 
    print_floc_stack (i_stack_pos, i_amount);
  
  /* If we are in a recursive Make, show the command invocation */
  if (makelevel > 0) 
    {
      printf("Most-recent (level %d) invocation:\n\t", makelevel);
      dbg_print_invocation();
    }
  return debug_readloop;
}

/* Give some help info. */
static debug_return_t dbg_cmd_show (char *psz_arg)
{
  if (!psz_arg || 0==strlen(psz_arg)) {
    unsigned int i;
    for (i=0; show_subcommands[i].name; i++) {
      if ( 0 == strcmp(show_subcommands[i].name, "warranty") ||
	   0 == strcmp(show_subcommands[i].name, "history"))
	continue;
      dbg_cmd_show((char *) show_subcommands[i].name);
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

/* Skip over next comand or action. */
static debug_return_t 
dbg_cmd_skip (char *psz_arg)
{
  return skip_execution;
}

/* Step execution until another stopping point is reached Argument N
   means do this N times (or until there's another reason to stop. */

static debug_return_t 
dbg_cmd_step (char *psz_arg)
{

  if (!psz_arg || !*psz_arg) {
    i_debugger_stepping = 1;
    i_debugger_nexting  = 0;
    return continue_execution;
  } 
  if (get_uint(psz_arg, &i_debugger_stepping)) 
    return continue_execution;
  else 
    return continue_execution;
}

/* Show a variable or target definition. */
debug_return_t dbg_cmd_target (char *psz_args) 
{
  file_t *p_target;
  const char *psz_target;

  p_target = get_target(&psz_args, &psz_target);
  if (p_target) {
    print_target_mask_t i_mask = 0;
    char *psz_word;
    
    while( (psz_word = get_word(&psz_args))) {
      if (!*psz_word) {
	break;
      } else if (is_abbrev_of(psz_word, "depends", 1)) {
	i_mask |= PRINT_TARGET_DEPEND;
      } else if (is_abbrev_of(psz_word, "nonorder", 1)) {
	i_mask |= PRINT_TARGET_NONORDER;
      } else if (is_abbrev_of(psz_word, "attributes", 1)) {
	i_mask |= PRINT_TARGET_ATTRS;
      } else if (is_abbrev_of(psz_word, "state", 1)) {
	i_mask |= PRINT_TARGET_STATE;
      } else if (is_abbrev_of(psz_word, "time", 1)) {
	i_mask |= PRINT_TARGET_TIME;
      } else if (is_abbrev_of(psz_word, "variables", 1)) {
	i_mask |= PRINT_TARGET_VARS;
      } else if (is_abbrev_of(psz_word, "commands", 1)) {
	i_mask |= PRINT_TARGET_CMDS;
      } else if (is_abbrev_of(psz_word, "expand", 1)) {
	i_mask |= (PRINT_TARGET_CMDS|PRINT_TARGET_CMDS_EXP);
      } else if (is_abbrev_of(psz_word, "previous", 1)) {
	i_mask |= PRINT_TARGET_PREV;
      } else {
	printf("Don't understand attribute '%s'\n", psz_word);
	return debug_readloop;
      }
    }
    
    if (0 == i_mask) i_mask = PRINT_TARGET_ALL & (~PRINT_TARGET_VARS_HASH);

    if (i_mask & PRINT_TARGET_VARS) {
      initialize_file_variables (p_target, 0);
      set_file_variables (p_target);
    }

    print_target_props(p_target, i_mask);
  }
  return debug_readloop;
}

/* Write commands associated with a given target. */
static debug_return_t dbg_cmd_write_cmds (char *psz_args) 
{
  file_t *p_target = NULL;
  const char *psz_target = NULL;
  int b_stdout = 0;

  p_target = get_target(&psz_args, &psz_target);
  if (p_target) {
    variable_t *p_v = lookup_variable ("SHELL", strlen ("SHELL"));
    char *psz_filename = NULL;
    FILE *outfd;
    char *s;
    
    if (! p_target->cmds || ! p_target->cmds->commands) {
      printf(_("Target \"%s\" doesn't have commands associated with it.\n"), 
	     psz_target);
      return debug_readloop;
    }
    
    s = p_target->cmds->commands;
    
    /* FIXME: should get directory from a variable, e.g. TMPDIR */

    if (psz_args && *psz_args) {
      if (strcmp (psz_args, "here") == 0)
	b_stdout = 1;
      else 
	psz_filename = strdup(psz_args);
    } else {
      /* Create target from the basename of the target name. */
      char *psz_target_basename = strrchr(psz_target, '/');
      if (!psz_target_basename) 
	psz_target_basename = psz_target;
      else 
	psz_target_basename++; /* Skip delimiter */
      psz_filename = CALLOC(char, strlen(psz_target_basename) + 10);
      snprintf(psz_filename, MAX_FILE_LENGTH, "/tmp/%s.sh", 
	       psz_target_basename);
    }
    
    /* Skip leading space, MAKE's command prefixes:
          echo suppression  @,
	  ignore-error  -, 
	  and recursion +
    */
    while (*s != '\0')
      {
	switch (*s) {
	case '@':
	case '+':
	case '-':
	  ++s;
	  break;
	default:
	  if (!isblank ((unsigned char)*s))
	    goto found_begin;
	  ++s;
	}
      }

  found_begin:
    if ( '\0' == *s ) {
      printf(_("Null command string parsed\n"));
    } else {
      if (b_stdout) 
	outfd = stdout;
      else if (!(outfd = fopen (psz_filename, "w"))) {
	perror ("write target");
	free(psz_filename);
	return debug_readloop;
      }
      
      if (p_v) {
	fprintf(outfd, "#!%s\n", variable_expand(p_v->value));
      }
      if (!p_target->floc.filenm && p_target->cmds->fileinfo.filenm) {
	/* Fake the location based on the commands - it's better than
	   nothing...
	*/
	fprintf(outfd, "#%s/%s:%lu\n", starting_directory,
		p_target->cmds->fileinfo.filenm, 
		p_target->cmds->fileinfo.lineno-1);
      } else {
	fprintf(outfd, "#%s/%s:%lu\n", starting_directory,
		p_target->floc.filenm, p_target->floc.lineno);
      }
      
      { 
	char wd[300];
	if (getcwd (wd, sizeof(wd))) {
	  fprintf(outfd, "#cd %s\n", wd);
	}
      }
	
      initialize_file_variables (p_target, 0);
      set_file_variables (p_target);

      {
	char *line = allocated_variable_expand_for_file (s, p_target);
	fprintf (outfd, "%s\n", line);
	free(line);
      }
      
      if (!b_stdout) {
	struct stat buf;
	if (0 == fstat(fileno(outfd), &buf)) {
	  mode_t mode = buf.st_mode;
	  if (buf.st_mode & S_IRUSR) mode |= S_IXUSR;
	  if (buf.st_mode & S_IRGRP) mode |= S_IXGRP;
	  if (buf.st_mode & S_IROTH) mode |= S_IXOTH;
	  if (0 != fchmod(fileno(outfd), mode)) {
	    printf(_("Can't set execute mode on \"%s\".\n"), psz_filename);
	  }
	}
	fclose(outfd);
	printf(_("File \"%s\" written.\n"), psz_filename);
	free(psz_filename);
      }
    }
  }
  return debug_readloop;
}

/* Set a variable definition with all variable references in the value
   part of psz_string expanded. */
static debug_return_t dbg_cmd_set (char *psz_args) 
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
      dbg_cmd_show("ignore-errors");
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


/* Set a variable. Set "expand' to 1 if you want variable 
   definitions inside the value getting passed in to be expanded
   before assigment. */
static debug_return_t dbg_cmd_set_var (char *psz_args, int expand) 
{
  if (!psz_args || 0==strlen(psz_args)) {
    printf(_("You need to supply a variable name.\n"));
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
      printf(_("Variable %s now has value '%s'\n"), psz_varname,
	     psz_value);
    } else {
      try_without_dollar(psz_varname);
    }
  }
  return debug_readloop;
}

/* Set a variable definition without variable references but don't 
   expand variable references in the value part of psz_string. */
static debug_return_t dbg_cmd_set_var_noexpand (char *psz_string) 
{
  dbg_cmd_set_var(psz_string, 0);
  return debug_readloop;
}

/* Run a shell command. */
static debug_return_t dbg_cmd_shell (char *psz_varname) 
{
  int rc=system(psz_varname);
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
#ifdef HAVE_LIBREADLINE
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
  
  if (0 == i_init) {
    rl_initialize ();
    cmd_initialize();
    i_init = 1;
    using_history ();
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
    psz_target_name = p->p_target->name;
  } else if (p_target) {
    p_target_loc    = &(p_target->floc);
    psz_target_name = p_target->name;
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
	printf("Use q to quit or R to restart\n");
      } else {
	printf("\nMakefile finished at level %d. Use R to restart\n", 
	       makelevel);
	printf("the makefile at this level or 's', 'n', or 'F' to continue "
	       "in parent\n");
	in_debugger = DEBUGGER_QUIT_RC;
      }
    } else {
      printf("\n***Entering debugger because we encountered a fatal error.\n");
      printf("***Exiting the debugger will exit make with exit code %d.\n", 
	     errcode);
    }
  }

  print_debugger_location(p_target, NULL);

  /* Loop reading and executing lines until the user quits. */
  for ( debug_return = debug_readloop; 
	debug_return == debug_readloop || debug_return == debug_cmd_error; ) {
    char prompt[PROMPT_LENGTH];
    char *line;
    char *s;
    
    snprintf(prompt, PROMPT_LENGTH, "mdb%s%d%s ", 
	     open_depth, where_history(), close_depth);
    
    line = readline (prompt);

    if ( line ) {
      if ( *(s=stripwhite(line)) ) {
	add_history (s);
	debug_return=execute_line(s);
      } else {
	debug_return=dbg_cmd_step("");
      }
      free (line);
    } else 
      dbg_cmd_quit(NULL);
  }

  if (in_debugger != DEBUGGER_QUIT_RC)
    in_debugger=false;
  
#endif /* HAVE_LIBREADLINE */
  return debug_return;
}
