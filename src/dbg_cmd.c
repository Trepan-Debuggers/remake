/* 
Copyright (C) 2004, 2005 Free Software Foundation, Inc.
This file is part of GNU Make.

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

#include "print.h"
#include "dbg_cmd.h"
#include "commands.h"
#include "debug.h"
#include "expand.h"
#include "function.h"

#ifdef HAVE_LIBREADLINE
#include <stdio.h>
#include <stdlib.h>
#include <config/readline.h>

/* From readline. ?? Should this be in configure?  */
#ifndef whitespace
#define whitespace(c) (((c) == ' ') || ((c) == '\t'))
#endif

bool b_in_debugger = false;

const char *WARRANTY = 
"			    NO WARRANTY\n"
"\n"
"  11. BECAUSE THE PROGRAM IS LICENSED FREE OF CHARGE, THERE IS NO WARRANTY\n"
"FOR THE PROGRAM, TO THE EXTENT PERMITTED BY APPLICABLE LAW.  EXCEPT WHEN\n"
"OTHERWISE STATED IN WRITING THE COPYRIGHT HOLDERS AND/OR OTHER PARTIES\n"
"PROVIDE THE PROGRAM \"AS IS\" WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED\n"
"OR IMPLIED, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF\n"
"MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.  THE ENTIRE RISK AS\n"
"TO THE QUALITY AND PERFORMANCE OF THE PROGRAM IS WITH YOU.  SHOULD THE\n"
"PROGRAM PROVE DEFECTIVE, YOU ASSUME THE COST OF ALL NECESSARY SERVICING,\n"
"REPAIR OR CORRECTION.\n"
"\n"
"  12. IN NO EVENT UNLESS REQUIRED BY APPLICABLE LAW OR AGREED TO IN WRITING\n"
"WILL ANY COPYRIGHT HOLDER, OR ANY OTHER PARTY WHO MAY MODIFY AND/OR\n"
"REDISTRIBUTE THE PROGRAM AS PERMITTED ABOVE, BE LIABLE TO YOU FOR DAMAGES,\n"
"INCLUDING ANY GENERAL, SPECIAL, INCIDENTAL OR CONSEQUENTIAL DAMAGES ARISING\n"
"OUT OF THE USE OR INABILITY TO USE THE PROGRAM (INCLUDING BUT NOT LIMITED\n"
"TO LOSS OF DATA OR DATA BEING RENDERED INACCURATE OR LOSSES SUSTAINED BY\n"
"YOU OR THIRD PARTIES OR A FAILURE OF THE PROGRAM TO OPERATE WITH ANY OTHER\n"
"PROGRAMS), EVEN IF SUCH HOLDER OR OTHER PARTY HAS BEEN ADVISED OF THE\n"
"POSSIBILITY OF SUCH DAMAGES.\n";


/* Common debugger command function prototype */
typedef debug_return_t (*dbg_cmd_t) (char *);

static floc_t *p_target_loc = NULL;
static char   *psz_target_name = NULL;
static int i_stack_pos = 0;

/* We use the if when we fake a line number because
   a real one hasn't been recorded on the stack.
*/
static floc_t  fake_floc;


/* A structure which contains information on the commands this program
   can understand. */

typedef struct {
  dbg_cmd_t func;               /* Function to call to do the job. */
  const char *doc;		/* Documentation for this function.  */
  const char *use;		/* short command usage.  */
} short_cmd_t;

typedef struct {
  const char *long_name;	/* long name of command. */
  const char short_name;	/* Index into short_cmd array. */
} long_cmd_t;

static int            dbg_cmd_show_var(char *psz_arg, int expand);
static debug_return_t dbg_cmd_set_var (char *psz_arg, int expand);

static debug_return_t dbg_cmd_break            (char *psz_arg);
static debug_return_t dbg_cmd_comment          (char *psz_arg);
static debug_return_t dbg_cmd_continue         (char *psz_arg);
static debug_return_t dbg_cmd_delete           (char *psz_arg);
static debug_return_t dbg_cmd_eval             (char *psz_arg);
static debug_return_t dbg_cmd_expand           (char *psz_arg);
static debug_return_t dbg_cmd_frame            (char *psz_arg);
static debug_return_t dbg_cmd_help             (char *psz_arg);
#ifdef HISTORY_STUFF
static debug_return_t dbg_cmd_history          (char *psz_arg);
#endif
static debug_return_t dbg_cmd_info             (char *psz_arg);
static debug_return_t dbg_cmd_target           (char *psz_arg);
static debug_return_t dbg_cmd_print            (char *psz_arg);
static debug_return_t dbg_cmd_quit             (char *psz_arg);
static debug_return_t dbg_cmd_restart          (char *psz_arg);
static debug_return_t dbg_cmd_set              (char *psz_arg);
static debug_return_t dbg_cmd_set_var_noexpand (char *psz_arg);
static debug_return_t dbg_cmd_shell            (char *psz_arg);
static debug_return_t dbg_cmd_show_stack       (char *psz_arg);
static debug_return_t dbg_cmd_skip             (char *psz_arg);
static debug_return_t dbg_cmd_frame_down       (char *psz_arg);
static debug_return_t dbg_cmd_frame_up         (char *psz_arg);
static debug_return_t dbg_cmd_frame_down       (char *psz_arg);
static debug_return_t dbg_cmd_show             (char *psz_arg);
static debug_return_t dbg_cmd_step             (char *psz_arg);
static debug_return_t dbg_cmd_write_cmds       (char *psz_arg);

long_cmd_t commands[] = {
  { "break",    'b' },
  { "comment",  '#' },
  { "continue", 'c' },
  { "delete",   'd' },
  { "down",     'D' },
  { "eval" ,    'e' },
  { "examine" , 'x' },
  { "exit"    , 'q' },
  { "frame"   , 'f' },
  { "help"    , 'h' },
  { "info"    , 'i' },
  { "next"    , 'n' },
  { "print"   , 'p' },
  { "quit"    , 'q' },
  { "restart" , 'r' },
  { "run"     , 'R' },
  { "set"     , '=' },
  { "setq"    , '"' },
  { "shell"   , '!' },
  { "show"    , 'S' },
  { "skip"    , 'k' },
  { "step"    , 's' },
  { "target"  , 't' },
  { "where"   , 'T' },
  { "write"   , 'w' },
  { "up"      , 'u' },
  { (char *)NULL, ' '}
};

short_cmd_t short_command[256] = { { NULL, '\0' }, };

typedef struct {
  const char *name;	/* name of subcommand command. */
  const char *doc;	/* short description of subcommand */
} subcommand_info_t;

char *info_subcommands[] = {
  "line",
  "locals",
  "target",
  "variables",
  "warranty",
  NULL
};

subcommand_info_t show_subcommands[] = {
  { "args",     "Show argument list to give program when it is started"},
  { "basename", "Show if we are to show short or long filenames"},
  { "debug",    "GNU Make debug mask (set via --debug or -d)" },
  { "ignore-errors", "Value of GNU Make --ignore-errors (or -i) flag" },
  { "keep-going",    "Value of GNU Make --keep-going (or -k) flag"},
  { "silent",        "Value of GNU Make --silent (or -s) flags"},
  { "trace",         "Show if we are tracing execution"},
  { "version",       "Show what version of GNU Make + dbg this is"},
  { "warranty",      "Various kinds of warranty you do not have"},
  NULL
};

subcommand_info_t set_subcommands[] = {
  { "basename",      "Set short filenames (the basename) in debug output"},
  { "debug",         "Set GNU Make debug mask (--debug or -d)" },
  { "ignore-errors", "Set GNU Make --ignore-errors (or -i) flag" },
  { "keep-going",    "Set GNU Make --keep-going (or -k) flag"},
  { "silent",        "Set GNU Make --silent (or -s) flags"},
  { "trace",         "Set if we are tracing execution"},
  NULL
};

/* Pointer to top of current target call stack */
static target_stack_node_t *p_stack_top;

/* Pointer to current target call stack at the place we are currently
   focused on.
 */
static target_stack_node_t *p_stack;

static void 
on_off_toggle(const char *psz_arg, int *var) 
{
  if (strcmp (psz_arg, "on") == 0)
    *var = 1;
  else if (strcmp (psz_arg, "off") == 0)
    *var = 0;
  else if (strcmp (psz_arg, "toggle") == 0)
    *var = !*var;
  else 
    printf(_("expecting \"on\", \"off\", or \"toggle\"; got \"%s\" \n"),
	   psz_arg);
}

static char *
var_to_on_off(int var) 
{
  return var ? "on " : "off";
}

/* Find the next "word" - skip leading blanks and the "word" is the
   largest non-blank characters after that. ppsz_str is modified to
   point after the portion returned and also the region initially
   pointed to by ppsz_str is modified so that word is zero-byte
   termintated.
 */
static char *
get_word(char **ppsz_str) 
{
  char *psz_word;
  
  /* Skip leading blanks. */
  while (**ppsz_str && whitespace (**ppsz_str))
    **ppsz_str++;

  /* Word starts here at first non blank character. */
  psz_word = *ppsz_str;

  /* Find end of word - next whitespace. */
  while (**ppsz_str && !whitespace (**ppsz_str))
    (*ppsz_str)++;

  if (**ppsz_str) *((*ppsz_str)++) = '\0';

  return psz_word;
}


static bool
get_int(const char *psz_arg, int *result) 
{
  int i;
  char *endptr;
  
  if (!psz_arg || 0==strlen(psz_arg)) return 0;

  i = strtol(psz_arg, &endptr, 10);
  if (*endptr != '\0') {
    printf("expecting %s to be an integer\n", psz_arg);
    return false;
  }
  *result = i;
  return true;
}

static unsigned int
get_uint(const char *psz_arg, unsigned int *result) 
{
  unsigned int i;
  char *endptr;
  
  if (!psz_arg || 0==strlen(psz_arg)) return 0;

  i = strtol(psz_arg, &endptr, 10);
  if (*endptr != '\0') {
    printf("expecting %s to be an integer\n", psz_arg);
    return 0;
  }
  *result = i;
  return 1;
}

static void 
cmd_initialize(void) 
{
  short_command['b'].func = &dbg_cmd_break;
  short_command['b'].use  = _("break *target*");
  short_command['b'].doc  = _("Set a breakpoint at a target.");

  short_command['c'].func = &dbg_cmd_continue;
  short_command['c'].use  = _("continue");
  short_command['c'].doc  = 
    _("Continue executing debugged Makefile until another breakpoint.");

  short_command['d'].func = &dbg_cmd_delete;
  short_command['d'].use  = _("delete *target*");
  short_command['d'].doc  = _("Delete target breakpoint.");

  short_command['D'].func = &dbg_cmd_frame_down;
  short_command['D'].use  = _("down [amount]");
  short_command['D'].doc  = 
    _("Select and print the target this one caused to be examined.\n" \
      "\tAn argument says how many targets down to go.");

  short_command['e'].func = &dbg_cmd_eval;
  short_command['e'].use  = _("eval *string*");
  short_command['e'].doc  = _("parse and evaluate a string.");

  short_command['f'].func = &dbg_cmd_frame;
  short_command['f'].use  = _("frame *n*");
  short_command['f'].doc  = 
    _("Move target frame to *n*; In contrast to \"up\" tor \"down\",\n" \
      "\tthis sets to an absolute postion. O is the top.");

  short_command['h'].func = &dbg_cmd_help;
  short_command['h'].use  = _("help [command]");
  short_command['h'].doc = 
    _("Display list of commands (i.e. this help text.)\n"		\
      "\twith an command name, give only the help for that command.");

#ifdef HISTORY_STUFF
  short_command['H'].func = &dbg_cmd_history;
  short_command['H'].use  = _("history [num]");
  short_command['H'].doc = 
    _("Display the history of commands\n"		\
      "\twith an argument number, list that many history entries.");
#endif

  short_command['i'].func = &dbg_cmd_info;
  short_command['i'].use = _("info [thing]");
  short_command['i'].doc = 
    _("Show the state of thing.\n" \
      "\tIf no 'thing' is specified, show everything there is to show.\n");

  short_command['k'].func = &dbg_cmd_skip;
  short_command['k'].use = _("skip");
  short_command['k'].doc = 
    _("Skip execution of next command or action.\n" );

  short_command['?'].func = &dbg_cmd_help;
  short_command['?'].use = short_command['h'].use;
  short_command['?'].doc = short_command['h'].doc;
  
  short_command['n'].func = &dbg_cmd_step;
  short_command['n'].use = _("next [amount]");
  short_command['n'].doc = _("alias for step.");

  short_command['p'].func = &dbg_cmd_print;
  short_command['p'].use = _("print {*variable* [attrs...]}");
  short_command['p'].doc = 
    _("Show a variable definition.\n" \
      "\tThe value is shown with embedded\n" \
      "\tvariable-references unexpanded. Don't include $ before a variable\n" \
      "\tname. See also \"examine\".\n\n" \
      "\tIf no variable is supplied, we try to use the\n" \
      "\tlast value given.\n"				
      );

  short_command['q'].func = &dbg_cmd_quit;
  short_command['q'].use = _("quit [exit-status]");
  short_command['q'].doc = 
    _("Exit make. If a numeric argument is given, it will be the exit\n"\
      "\tstatus this program reports back. Otherwise exit with status 0." \
      );

  short_command['Q'].func = &dbg_cmd_quit;
  short_command['Q'].use  = _("exit");
  short_command['Q'].doc  = _("alias for quit.");
  
  short_command['r'].func = &dbg_cmd_restart;
  short_command['r'].doc = _("Start program - same as restart.");
  short_command['r'].use = _("run");

  short_command['R'].func = &dbg_cmd_restart;
  short_command['R'].doc = _("Restart program. Alias: run");
  short_command['R'].use = _("restart");

  short_command['s'].func = &dbg_cmd_step;
  short_command['s'].use = _("step [amount]");
  short_command['s'].doc = 
    _("Step execution until another stopping point is reached.\n" \
      "\tArgument N means do this N times (or until there's another\n " \
      "\treason to stop.");

  short_command['S'].func = &dbg_cmd_show;
  short_command['S'].use = _("show [thing]");
  short_command['S'].doc = 
    _("Show the state of thing.\n" \
      "\tIf no 'thing' is specified, show everything there is to show.\n");

  short_command['t'].func = &dbg_cmd_target;
  short_command['t'].use = _("target");
  short_command['t'].doc = 
    _("Show information about a target.\n" \
      "\ttarget information is printed.\n" \
      "\tThe following attributes names can be given after a target name:\n" \
      "\t\t'attributes', 'commands', 'depends', 'nonorder',\n" \
      "\t\t'previous', 'state', 'time', 'variables'\n" \
      "\tIf no variable or target name is supplied, we try to use the\n" \
      "\tcurrent target name.\n"				
      );

  short_command['T'].func = &dbg_cmd_show_stack;
  short_command['T'].doc  = _("Show target stack.");
  short_command['T'].use  = _("where");

  short_command['u'].func = &dbg_cmd_frame_up;
  short_command['u'].use  = _("up [amount]");
  short_command['u'].doc  = 
    _("Select and print target that caused this one to be examined.\n" \
      "\tAn argument says how many targets up to go.");

  short_command['w'].func = &dbg_cmd_write_cmds;
  short_command['w'].use =  _("write [*target* [*filename*]]");
  short_command['w'].doc  = 
    _("writes the commands associated of a target to a file with MAKE\n" \
      "\tvariables expanded. If no target given use the current one.\n"
      "\tIf a filename is supplied it is used. If it is the string \"here\"\n"
      "\twe write the output to stdout. If no filename is given then we\n"
      "\tcreate the filename by prepending a directory name to the target\n"
      "\tname and then append \".sh\".");

  short_command['x'].func = &dbg_cmd_expand;
  short_command['x'].use =  _("examine *string*");
  short_command['x'].doc  = 
    _("Show string with internal variables references expanded. See also \n" \
      "\t\"print\".");

  short_command['#'].func = &dbg_cmd_comment;
  short_command['#'].use =  _("comment *text*");
  short_command['#'].doc  = 
    _("Ignore this line.");

  short_command['!'].func = &dbg_cmd_shell;
  short_command['!'].use =  _("shell *string*");
  short_command['!'].doc  = 
    _("Execute the rest of the line as a shell.");

  short_command['='].func = &dbg_cmd_set;
  short_command['='].use =  
    _("set {basename|debug|ignore-errors|keep-going|silent|trace|variable} *value*");
  short_command['='].doc  = 
    _("set basename {on|off|toggle} - show full name or basename?\n"
      "\tset debug debug-mask - like --debug value.\n\n"
      "\tset ignore-errors {on|off|toggle} - like --ignore-errors option\n\n"
      "\tset keep-going {on|off|toggle} - like --keep-going option\n\n"
      "\tset silent {on|off|toggle} - like --silent option\n\n"
      "\tset trace {on|off|toggle} - set tracing status\n"
      "\tset variable *var* *value*\n"
      "\tSet MAKE variable to value. Variable definitions\n"
      "\tinside VALUE are expanded before assignment occurs.\n"
      );

  short_command['"'].func = &dbg_cmd_set_var_noexpand;
  short_command['"'].use =  _("setq *variable* *value*");
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

  for (i = 0; commands[i].long_name; i++)
    if (strcmp (psz_name, commands[i].long_name) == 0)
      return (&short_command[(uint8_t) commands[i].short_name]);

  return ((short_cmd_t *)NULL);
}

int is_abbrev_of(const char* psz_substr, const char* psz_word) 
{
  char *psz = strstr(psz_word, psz_substr);
  return (psz && psz == psz_word);
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
      fprintf (stderr, "No such debugger command: %s.\n", psz_word);
      return debug_readloop;
    }

  /* Get argument to command, if any. */
  while (whitespace (psz_line[i]))
    i++;

  psz_word = psz_line + i;

  /* Call the function. */
  return ((*(command->func)) (psz_word));
}

/* Strip whitespace from the start and end of STRING.  Return a pointer
   into STRING. */
static char *
stripwhite (char *string)
{
  char *s, *t;

  for (s = string; whitespace (*s); s++)
    ;
    
  if (*s == 0)
    return (s);

  t = s + strlen (s) - 1;
  while (t > s && whitespace (*t))
    t--;
  *++t = '\0';

  return s;
}

/* Give some help info. */
static debug_return_t dbg_cmd_help (char *psz_arg)
{
  unsigned int i;

  if (!psz_arg || 0==strlen(psz_arg)) {
    printf ("Available commands are: \n");
    for (i = 0; commands[i].long_name; i++) {
      uint8_t s=commands[i].short_name;
      printf("  %-25s (%c):\n", 
	     short_command[s].use, commands[i].short_name);
      printf("\t%s\n\n", short_command[s].doc);
    }

    printf("Readline command line editing (emacs/vi mode) is available.\n");
    printf("For more help, type h <cmd> or consult online-documentation.\n");
    
  } else {
    short_cmd_t *p_command;

    if (1 == strlen(psz_arg)) {
      if ( NULL != short_command[(uint8_t)psz_arg[0]].func ) 
	p_command = &short_command[(uint8_t)psz_arg[0]];
      else
	p_command = NULL;
    } else {
      p_command = find_command (psz_arg);
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
	  printf("show %-15s -- %s\n", 
		 show_subcommands[i].name, show_subcommands[i].doc );
	}
      } else if ( p_command->func == &dbg_cmd_set ) {
	for (i = 0; set_subcommands[i].name; i++) {
	  printf("set %-15s -- %s\n", 
		 set_subcommands[i].name, set_subcommands[i].doc );
	}
      }
    } else {
      printf("Invalid command %s. Try help for a list of commands\n", 
	     psz_arg);
    }
  }
  
  return debug_readloop;
}

#if HISTORY_STUFF
/* Give some help info. */
static debug_return_t dbg_cmd_history (char *psz_arg)
{
  unsigned int i;

  if (!psz_arg || 0==strlen(psz_arg)) {
    ;
  }
  
}
#endif

/* Show target call stack info. */
static debug_return_t dbg_cmd_restart (char *psz_arg)
{
  printf("Changing directory to %s and restarting...\n", 
	 directory_before_chdir);
  chdir (directory_before_chdir);
  execvp (global_argv[0], global_argv);
  /* NOT USED: */
  return debug_readloop;
}

/* Show target call stack info. */
static debug_return_t dbg_cmd_show_stack (char *psz_arg)
{
  print_target_stack (p_stack_top, i_stack_pos);
  return debug_readloop;
}

/* Terminate execution. */
static debug_return_t dbg_cmd_quit (char *psz_arg)
{
  if (!psz_arg || 0==strlen(psz_arg)) {
    exit(0);
  } else {
    int rc;
    if (get_int(psz_arg, &rc)) 
      exit(rc);
  }
  /* NOT USED: */
  return debug_readloop;
}

/* Set a breakpoint. */
static debug_return_t dbg_cmd_break (char *psz_target)
{
  file_t *p_target;

  if (!psz_target || 0==strlen(psz_target))
    return debug_readloop;
  
  p_target = lookup_file (psz_target);
  if (!p_target) {
    printf("Can't find target %s; breakpoint not set.\n", psz_target);
    return debug_readloop;
  }

  if (p_target->tracing) {
    printf("Breakpoint already set at target %s; nothing done.\n", psz_target);
  } else {
    p_target->tracing = 1;
    printf("Breakpoint on target %s set.\n", psz_target);
  }

  return debug_readloop;
}

/* Delete a breakpoint. */
static debug_return_t dbg_cmd_delete (char *psz_target)
{
  file_t *p_target;

  if (!psz_target || 0==strlen(psz_target))
    return debug_readloop;
  
  p_target = lookup_file (psz_target);
  if (!p_target) {
    printf("Can't find target %s; breakpoint not cleared.\n", psz_target);
    return debug_readloop;
  }

  if (p_target->tracing) {
    p_target->tracing = 0;
    printf("Breakpoint on target %s cleared\n", psz_target);
  } else {
    printf("No breakpoint at target %s; nothing cleared.\n", psz_target);
  }

  return debug_readloop;
}

/* Parse and evaluate buffer and return the results. */
static debug_return_t dbg_cmd_eval (char *psz_evalstring)
{
  func_eval(NULL, &psz_evalstring, NULL);
  reading_file = 0;
  return debug_readloop;
}

/* Comment line - ingore text on line. */
static debug_return_t dbg_cmd_comment (char *psz_arg)
{
  return debug_readloop;
}

/* Continue running program. */
static debug_return_t dbg_cmd_continue (char *psz_arg)
{
  debugger_stepping = 0;
  return continue_execution;
}

/* Give some help info. */
static debug_return_t dbg_cmd_show (char *psz_arg)
{
  if (!psz_arg || 0==strlen(psz_arg)) {
    unsigned int i;
    for (i=0; show_subcommands[i].name; i++) {
      if ( 0 != strcmp(show_subcommands[i].name, "warranty") )
	dbg_cmd_show((char *) show_subcommands[i].name);
    }
  } else {
    if (is_abbrev_of (psz_arg, "args")) {
      unsigned int i;
      printf("args:");
      for (i = 0; global_argv[i]; i++) {
	printf(" %s", global_argv[i]);
      }
      printf("\n");
    } else if (is_abbrev_of (psz_arg, "basename")) {
      printf("basename is %s.\n", var_to_on_off(basename_filenames));
    } else if (is_abbrev_of (psz_arg, "debug")) {
      printf("debug is %d.\n", db_level);
    } else if (is_abbrev_of (psz_arg, "ignore-errors")) {
      printf("ignore-errors is %s.\n", var_to_on_off(ignore_errors_flag));
    } else if (is_abbrev_of (psz_arg, "keep-going")) {
      printf("keep-going is %s.\n", var_to_on_off(keep_going_flag));
    } else if (is_abbrev_of (psz_arg, "silent")) {
      printf("silent is %s.\n", var_to_on_off(silent_flag));
    } else if (is_abbrev_of (psz_arg, "trace")) {
      printf("trace is %s.\n", var_to_on_off(tracing));
    } else if (is_abbrev_of (psz_arg, "version")) {
      printf("version: ");
      print_version();
    } else if (is_abbrev_of (psz_arg, "warranty")) {
      printf("warranty: ");
      printf(WARRANTY);
    } else {
      printf("Undefined show command \"%s\". Try \"help show\"\n", psz_arg);
    }
  }
  
  return debug_readloop;
}

/* Give some help info. */
static debug_return_t dbg_cmd_info (char *psz_arg)
{
  if (!psz_arg || 0==strlen(psz_arg)) {
    dbg_cmd_help("info");
  } else {
    if (is_abbrev_of (psz_arg, "line")) {
      /* We want output to be compatible with gdb output.*/
      if (p_stack_top && p_stack_top->p_target && 
	  p_stack_top->p_target->floc.filenm) {
	const floc_t *p_floc = &p_stack_top->p_target->floc;
	if (!basename_filenames && strlen(p_floc->filenm) 
	    && p_floc->filenm[0] != '/') 
	  printf("Line %d of \"%s/%s\"", p_floc->lineno, starting_directory,
		 p_floc->filenm);
	else 
	  printf("Line %d of \"%s\"", p_floc->lineno, p_floc->filenm);
      } else {
	printf("No line number info recorded.\n");
      }
      
    } else if (is_abbrev_of (psz_arg, "locals")) {
      if (p_stack_top && p_stack_top->p_target) {
	file_t *p_target = p_stack_top->p_target;
	if (!p_target->variables) {
	  char *psz_target = p_target->name;
	  p_target = lookup_file (psz_target);
	  if (!p_target->variables) return debug_readloop;
	}
	hash_map_arg (&p_target->variables->set->table, 
		      print_variable_info, NULL);
      }
    } else if (is_abbrev_of (psz_arg, "stack")) {
      print_target_stack(p_stack_top, i_stack_pos);
    } else if (is_abbrev_of (psz_arg, "target")) {
      if (p_stack_top && p_stack_top->p_target && p_stack_top->p_target->name)
	printf("target: %s\n", p_stack_top->p_target->name);
      else 
	printf("target unknown\n");
    } else if (is_abbrev_of (psz_arg, "variables")) {
      print_variable_data_base();
    } else if (is_abbrev_of (psz_arg, "warranty")) {
      printf(WARRANTY);
    } else {
      printf("Undefined info command \"%s\". Try \"help info\"\n", psz_arg);
    }
  }
  
  return debug_readloop;
}

/* Skip over next comand or action. */
static debug_return_t dbg_cmd_skip (char *psz_arg)
{
  return skip_execution;
}

/* Step next command. */
static debug_return_t dbg_cmd_step (char *psz_arg)
{

  if (!psz_arg || 0==strlen(psz_arg)) {
    debugger_stepping = 1;
    return continue_execution;
  } 
  if (get_uint(psz_arg, &debugger_stepping)) 
    return continue_execution;
  else 
    return continue_execution;
}

/* Show a variable definition. */
static debug_return_t dbg_cmd_print (char *psz_args) 
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
  
  if (dbg_cmd_show_var(psz_name, 0)) {
    if (psz_last_name) free(psz_last_name);
    psz_last_name = strdup(psz_name);
  }

  return debug_readloop;
}

/* Show a variable or target definition. */
static debug_return_t dbg_cmd_target (char *psz_args) 
{
  file_t *p_target;
  char   *psz_target;

  if (!psz_args || 0==strlen(psz_args)) {
    /* Use current target */
    if (p_stack && p_stack->p_target && p_stack->p_target->name) {
      psz_args = p_stack->p_target->name;
    } else {
      printf("No current target - must supply something to print\n");
      return debug_readloop;
    }
  }

  psz_target = get_word(&psz_args);
  
  p_target = lookup_file (psz_target);
  if (p_target) {
    print_target_mask_t i_mask = 0;
    char *psz_word;
    
    while( (psz_word = get_word(&psz_args))) {
      if (0 == strlen(psz_word)) {
	break;
      } else if (is_abbrev_of(psz_word, "depends")) {
	i_mask |= PRINT_TARGET_DEPEND;
      } else if (is_abbrev_of(psz_word, "nonorder")) {
	i_mask |= PRINT_TARGET_NONORDER;
      } else if (is_abbrev_of(psz_word, "attributes")) {
	i_mask |= PRINT_TARGET_ATTRS;
      } else if (is_abbrev_of(psz_word, "state")) {
	i_mask |= PRINT_TARGET_STATE;
      } else if (is_abbrev_of(psz_word, "time")) {
	i_mask |= PRINT_TARGET_TIME;
      } else if (is_abbrev_of(psz_word, "variables")) {
	i_mask |= PRINT_TARGET_VARS;
      } else if (is_abbrev_of(psz_word, "commands")) {
	i_mask |= PRINT_TARGET_CMDS;
      } else if (is_abbrev_of(psz_word, "previous")) {
	i_mask |= PRINT_TARGET_PREV;
      } else {
	printf("Don't understand attribute '%s'\n", psz_word);
	return debug_readloop;
      }
    }
    
    if (0 == i_mask) i_mask = PRINT_TARGET_ALL;
    print_target_props(p_target, i_mask);
  } else {
    printf("Couldn't find target '%s'\n", psz_target);
  }
  return debug_readloop;
}

#define MAX_FILE_LENGTH 1000
/* Write commands associated with a given target. */
static debug_return_t dbg_cmd_write_cmds (char *psz_args) 
{
  file_t *p_target;
  char *psz_target;
  int b_stdout = 0;

  if (!psz_args || 0==strlen(psz_args)) {
    /* Use current target */
    if (p_stack && p_stack->p_target && p_stack->p_target->name)
      psz_target = p_stack->p_target->name;
    else {
      printf("No current target - supply a target name.\n");
      return debug_readloop;
    }
  } else {
    psz_target = get_word(&psz_args);
  }

  /* As a special case, we'll allow $@ for the current target. */
  if ( 0 == strcmp("$@", psz_target) ) {
    if (p_stack && p_stack->p_target && p_stack->p_target->name)
      psz_target = p_stack->p_target->name;
    else {
      printf(_("No current target found for $@ - supply a target name.\n"));
      return debug_readloop;
    }
  }
  
  p_target = lookup_file (psz_target);
  if (!p_target) {
    printf(_("Target \"%s\" doesn't appear to be a target name.\n"), 
	   psz_target);
  } else {
    variable_t *p_v = 
      lookup_variable ("SHELL", strlen ("SHELL"));
    char filename[MAX_FILE_LENGTH];
    FILE *outfd;
    char *s;
    
    
    if (! p_target->cmds || ! p_target->cmds->commands) {
      printf(_("Target \"%s\" doesn't have commands associated with it.\n"), 
	     psz_target);
      return debug_readloop;
    }
    
    s = p_target->cmds->commands;
    
    /* FIXME: should get directory from a variable, e.g. TMPDIR */

    if (psz_args) {
      if (strcmp (psz_args, "here") == 0)
	b_stdout = 1;
      else 
	strncpy(filename, psz_args, MAX_FILE_LENGTH);
    } else {
      snprintf(filename, MAX_FILE_LENGTH, "/tmp/%s.sh", psz_target);
    }
    
    /* Skip leading space, MAKE's comamnd prefixes:
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
      else if (!(outfd = fopen (filename, "w"))) {
	perror ("write target");
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
      
      initialize_file_variables (p_target, 0);
      set_file_variables (p_target);

      {
	char *line = allocated_variable_expand_for_file (s, p_target);
	fprintf (outfd, "%s\n", line);
	free(line);
      }
      
      if (!b_stdout) {
	fclose(outfd);
	printf(_("File \"%s\" written.\n"), filename);
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
      *psz_args++;

    if (is_abbrev_of (psz_varname, "variable")) {
      return dbg_cmd_set_var(psz_args, 1);
    } else if (is_abbrev_of (psz_varname, "basename")) {
      if (!psz_args || 0==strlen(psz_args))
	on_off_toggle("toggle", &basename_filenames);
      else
	on_off_toggle(psz_args, &basename_filenames);
      dbg_cmd_show("basename");
    } else if (is_abbrev_of (psz_varname, "debug")) {
      int dbg_mask;
      if (get_int(psz_args, &dbg_mask)) {
	db_level = dbg_mask;
      }
    } else if (is_abbrev_of (psz_varname, "ignore-errors")) {
      if (!psz_args || 0==strlen(psz_args))
	on_off_toggle("toggle", &ignore_errors_flag);
      else
	on_off_toggle(psz_args, &ignore_errors_flag);
      dbg_cmd_show("ignore-errors");
    } else if (is_abbrev_of (psz_varname, "keep-going")) {
      if (!psz_args || 0==strlen(psz_args))
	on_off_toggle("toggle", &keep_going_flag);
      else
	on_off_toggle(psz_args, &keep_going_flag);
      dbg_cmd_show("keep-going");
    } else if (is_abbrev_of (psz_varname, "silent")) {
      if (!psz_args || 0==strlen(psz_args))
	on_off_toggle("toggle", &silent_flag);
      else
	on_off_toggle(psz_args, &silent_flag);
      dbg_cmd_show("silent");
    } else if (is_abbrev_of (psz_varname, "trace")) {
      if (!psz_args || 0==strlen(psz_args))
	on_off_toggle("toggle", &tracing);
      else
	on_off_toggle(psz_args, &tracing);
      dbg_cmd_show("trace");
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
      *psz_args++;

    p_v = lookup_variable (psz_varname, u_len);

    if (p_v) {
      variable_origin_t e_origin = p_v->origin;
      char *psz_value =  expand ? variable_expand(psz_args) : psz_args;
      
      define_variable_in_set(p_v->name, u_len, psz_value,
			     o_debugger, 0, NULL,
			     &(p_v->fileinfo));
      printf(_("Variable %s now has value '%s'\n"), psz_varname,
	     psz_value);
    } else {
      printf(_("Can't find variable %s\n"), psz_varname);
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
  system(psz_varname);
  return debug_readloop;
}


/* Show a variable definition. Set "expand" to 1 if you want variable
   definitions inside the displayed value expanded.
*/
static int dbg_cmd_show_var (char *psz_varname, int expand) 
{
  if (!psz_varname || 0==strlen(psz_varname)) {
    printf(_("You need to supply a variable name.\n"));
    return 0;
  } else {
    variable_t *p_v;
    variable_set_t *p_set = NULL;
    if (p_stack) {
      initialize_file_variables (p_stack->p_target, 0);
      set_file_variables (p_stack->p_target);
      if (p_stack->p_target->variables) 
	p_set = p_stack->p_target->variables->set;
      else {
	char *psz_target = p_stack->p_target->name;
	file_t *p_target = lookup_file (psz_target);
	p_set = p_target->variables->set;
      }
    }
    p_v = lookup_variable (psz_varname, strlen (psz_varname));
    if (!p_v && p_set) {
      p_v = lookup_variable_in_set(psz_varname, strlen(psz_varname), p_set);
    }
    if (p_v) {
      if (expand) {
	print_variable_expand(p_v);
      } else
	print_variable(p_v);
    } else {
      if (expand)
	printf("%s\n", variable_expand(psz_varname));
      else {
	printf("Can't find variable %s\n", psz_varname);
	return 0;
      }
    }
  }
  return 1;
}

/* Show a variable definition;'t expand any variable references
   in the displayed value. */
static debug_return_t dbg_cmd_expand (char *psz_string) 
{
  static char *psz_last_string = NULL;

  if (!psz_string || 0==strlen(psz_string)) {
    /* Use last target value */
    if (psz_last_string)
      psz_string = psz_last_string;
    else {
      printf("No current expand string - must supply something to print\n");
      return debug_readloop;
    }
  }
  
  if (dbg_cmd_show_var(psz_string, 1)) {
    if (psz_last_string) free(psz_last_string);
    psz_last_string = strdup(psz_string);
  }
  return debug_readloop;
}

/* Move reported target frame postition up one. */
static debug_return_t dbg_cmd_frame_down (char *psz_amount) 
{
  unsigned int i=0;
  int i_amount = 1;

  if (!psz_amount || 0==strlen(psz_amount)) {
    i_amount = 1;
  } else {
    if (!get_int(psz_amount, &i_amount))
      return debug_readloop;
  }

  if (i_stack_pos - i_amount < 0) {
    printf(_("Move down by %d would be below bottom-most frame position.\n"),
	   i_amount);
    return debug_readloop;
  }
  
  i_stack_pos -= i_amount;
  
  for ( p_stack=p_stack_top; p_stack ; p_stack = p_stack->p_parent ) {
    if (i_stack_pos == i)
      break;
    i++;
  }

  p_target_loc    = &(p_stack->p_target->floc);
  psz_target_name = p_stack->p_target->name;
  
  return debug_readloop;
}

/* Move reported target frame postition up one. */
static debug_return_t dbg_cmd_frame (char *psz_frame) 
{
  int i, i_frame;

  if (!psz_frame || 0==strlen(psz_frame)) {
    return debug_readloop;
  } else {
    if (!get_int(psz_frame, &i_frame))
      return debug_readloop;
  }

  i = i_frame + 1;
  
  for ( p_stack=p_stack_top; p_stack ; p_stack = p_stack->p_parent ) {
    i--;
    if (0 == i)
      break;
  }

  if (0 != i) {
    printf(_("Can't set frame to position %d; %d is the highest position."),
	   i_frame, i_frame - i);
    return debug_readloop;
  }

  i_stack_pos     = i_frame;
  p_target_loc    = &(p_stack->p_target->floc);
  psz_target_name = p_stack->p_target->name;
  
  return debug_readloop;
}

/* Move reported target frame postition up one. */
static debug_return_t dbg_cmd_frame_up (char *psz_amount) 
{
  unsigned int i_amount=1;
  unsigned int i = 0;
  target_stack_node_t *p=p_stack;

  if (!psz_amount || 0==strlen(psz_amount)) {
    i_amount = 1;
  } else {
    if (!get_uint(psz_amount, &i_amount))
      return debug_readloop;
  }

  for ( ; p ; p = p->p_parent, i++ ) {
    if (i_amount == i) break;
  }

  if (p) {
    i_stack_pos    += i_amount;
    p_stack         = p;
    psz_target_name = p->p_target->name;

    p_target_loc    = &(p->p_target->floc);
    if (!p->p_target->floc.filenm && p->p_target->cmds->fileinfo.filenm) {
      /* Fake the location based on the commands - it's better than
	 nothing...
       */
      memcpy(&fake_floc, &(p->p_target->cmds->fileinfo),
	     sizeof(floc_t));
      /* HACK: is it okay to assume that the target is on the line
	 before the first command? Or should we list the line
	 that the command starts on - so we know we've faked the location?
       */
      fake_floc.lineno--;
      p_target_loc = &fake_floc;
    }
  } else {
    printf("Can't move up %d - would be beyond top-most frame position.\n",
	   i_amount);
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

debug_return_t
enter_debugger (target_stack_node_t *p, file_t *p_target, int err)
{
  debug_return_t debug_return = debug_readloop;
#ifdef HAVE_LIBREADLINE
  static int i_init = 0;
  char open_depth[MAX_NEST_DEPTH];
  char close_depth[MAX_NEST_DEPTH];
  unsigned int i = 0;

  if ( debugger_stepping > 1 ) {
    /* Don't stop unless we are here from a breakpoint. But
       do decrement the step count. */
    debugger_stepping--;
    if (!p_target->tracing) return continue_execution;
  } else if (!debugger_on_error && !debugger_stepping 
	     && !p_target->tracing && -2 != err) 
    return continue_execution;
  
  if (0 == i_init) {
    rl_initialize ();
    cmd_initialize();
    i_init = 1;
    /* where_history() returns 0 when no commands are initialized and
       0 when the first one is set. Thus, if we don't add a dummy
       initial command as we do below, we'll get history item 0 listed
       twice.  I don't know a better way to fix. */
    add_history (""); 
  }

  /* Set initial frame position reporting area: 0 is bottom. */
  p_target_loc    = NULL;
  psz_target_name = "";
  i_stack_pos      = 0;

  p_stack = p_stack_top = p;

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

  if (err) {
    if (-1 == err) {
      printf("\n***Entering debugger because we encountered an error.\n");
    } else if (-2 == err) {
      printf("\nDebugged Makefile terminated.  "
	     "Use q to quit or R to restart\n");
    } else {
      printf("\n***Entering debugger because we encountered a fatal error.\n");
      printf("***Exiting the debugger will exit make with exit code %d.\n", 
	     err);
    }
  }

  b_in_debugger = true;

  /* Loop reading and executing lines until the user quits. */
  for ( debug_return = debug_readloop; debug_return == debug_readloop; ) {
    char prompt[PROMPT_LENGTH];
    char *line;
    char *s;
    
    if (p_target_loc) {
      printf ("\n");
      if ( !p_target_loc->filenm && !p_target_loc->lineno 
	   && p_target->name ) {
	/* We don't have file location info in the target floc, but we
	   do have it as part of the name, so use that. This happens for
	   example with we've stopped before reading a Makefile.
	 */
	printf("(%s:0)\n", p_target->name);
      } else {
	printf("(", p_target->name);
	print_floc_prefix(p_target_loc);
	printf ("): %s\n", psz_target_name);
      }
    }
    
    snprintf(prompt, PROMPT_LENGTH, "makedb%s%d%s ", 
	     open_depth, where_history(), close_depth);
    
    if ( (line = readline (prompt)) ) {
      if ( *(s=stripwhite(line)) ) {
	add_history (s);
	debug_return=execute_line(s);
      } else {
	debug_return=dbg_cmd_step("");
      }
      free (line);
    }
  }
  b_in_debugger=false;
#endif /* HAVE_LIBREADLINE */
  return debug_return;
}
