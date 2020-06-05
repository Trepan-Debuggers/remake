/*
Copyright (C) 2004-2005, 2007-2009, 2011,
              2014-2015, 2020 R. Bernstein
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

/** \file libdebugger/cmd.c
 *
 *  \brief Top-level command REPL.
 *
 */

#include "makeint.h"
#include "main.h"
#include "msg.h"
#include "debug.h"
#include "file.h"
#include "print.h"
#include "cmd.h"
#include "fns.h"
#include "info.h"
#include "stack.h"
#include "commands.h"
#include "debug.h"
#include "file2line.h"
#include "cmd.h"

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
#endif /* HAVE_LIBREADLINE */

#include "cmd_initialize.h"


#ifdef HAVE_READLINE_READLINE_H
#include <readline/readline.h>
#endif

#ifdef HAVE_READLINE_HISTORY_H
#include <readline/history.h>
#endif

#ifdef _GNU_SOURCE
# define ATTRIBUTE_UNUSED __attribute__((unused))
#else
# define ATTRIBUTE_UNUSED
#endif

/**
   Command-line args after the command-name part. For example in:
   break foo
   the below will be "foo".
 **/
char *psz_debugger_args;

debug_enter_reason_t last_stop_reason;

#ifdef HAVE_LIBREADLINE

short_cmd_t short_command[256] = { { NULL,
                                     (const char *) '\0',
                                     (const char *) '\0',
                                     (uint8_t) 255,
                                     false,
                                    }, };

/*!
  Look up `psz_name` as the name of a command, and return a pointer to that
  command.  Return a NULL pointer if `psz_name` isn't a command name.
*/
short_cmd_t *
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

  for (i = 0; dbg_commands[i].long_name; i++) {
    const int cmp = strcmp (psz_name, dbg_commands[i].long_name);
    if ( 0 == cmp ) {
      return &(short_command[(uint8_t) dbg_commands[i].short_name]);
    } else
      /* Words should be in alphabetic order by command name.
	 Have we gone too far? */
      if (cmp < 0) break;
  }

  return ((short_cmd_t *)NULL);
}

/*!
  Execute a command line: the "EP" part of "REPL".
*/
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

  if (command->needs_running && last_stop_reason == DEBUG_ERROR_HIT) {
      dbg_errmsg(_("command: %s is only valid when `remake` not in post-mortem debugging."), psz_word);
      return debug_readloop;
  }

  /* Get argument to command, if any. */
  while (whitespace (psz_line[i]))
    i++;

  psz_debugger_args = psz_line + i;

  /* Call the function. */
  return ((*(command->func)) (psz_debugger_args));
}

/*!
  Show command history.
*/
debug_return_t
dbg_cmd_show_command (const char
                      *psz_args ATTRIBUTE_UNUSED)
{
 /*
  if (!psz_arg || *psz_arg) {
    ;
    } */

#ifdef HAVE_READLINE_HISTORY_H
  HIST_ENTRY **hist_list = history_list();
  unsigned int i;
  UNUSED_ARGUMENT(psz_args);
  if (!hist_list) return debug_readloop;
  for (i=1; hist_list[i]; i++) {
    dbg_msg("%5u  %s", i, hist_list[i]->line);
  }
#endif /* HAVE_READLINE_HISTORY_H */
  return debug_readloop;
}

/*!
  Set a variable. Set "expand' to 1 if you want variable
  definitions inside the value getting passed in to be expanded
  before assigment.
*/
extern debug_return_t dbg_cmd_set_var (char *psz_args, int expand)
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
      dbg_msg(_("Variable %s now has value '%s'"), psz_varname,
	     psz_value);
    } else {
      p_v = try_without_dollar(psz_varname);
      if (p_v) {
        char *psz_value =  expand ? variable_expand(psz_args) : psz_args;
        define_variable_in_set(p_v->name, u_len, psz_value,
                               o_debugger, 0, NULL,
                               &(p_v->fileinfo));
        dbg_msg(_("Variable %s now has value '%s'"), psz_varname,
                psz_value);
      }
    }
  }
  return debug_readloop;
}

#endif /* HAVE_LIBREADLINE */

#define PROMPT_LENGTH 300

#include <setjmp.h>
jmp_buf debugger_loop;

/* Should be less that PROMPT_LENGTH / 2 - strlen("remake ") + log(history)
   We will make it much less that since people can't count more than
   10 or so nested <<<<>>>>'s easily.
*/
#define MAX_NEST_DEPTH 10

debug_return_t enter_debugger (target_stack_node_t *p,
			       file_t *p_target, int errcode,
			       debug_enter_reason_t reason)
{
  volatile debug_return_t debug_return = debug_readloop;
  static bool b_init = false;
  static bool b_readline_init = false;
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

#ifdef HAVE_LIBREADLINE
  if (use_readline_flag && !b_readline_init) {
      rl_initialize ();
      using_history ();
      add_history ("");
      b_readline_init = true;
  }
#endif /* HAVE_LIBREADLINE */
  if (!b_init) {
    cmd_initialize();
    file2lines.ht_size = 0;
    b_init = true;
  }


  /* Set initial frame position reporting area: 0 is bottom. */
  p_target_loc    = NULL;
  psz_target_name = (char *) "";
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
	printf("\nMakefile finished at level %u. Use R to restart\n",
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

    if (setjmp(debugger_loop))
      dbg_errmsg("Internal error jumped back to debugger loop");
    else {

#ifdef HAVE_LIBREADLINE
      if (use_readline_flag) {
        snprintf(prompt, PROMPT_LENGTH, "remake%s%d%s ",
                 open_depth, where_history(), close_depth);

        line = readline (prompt);
      } else
#endif /* HAVE_LIBREADLINE */
        {
          snprintf(prompt, PROMPT_LENGTH, "remake%s0%s ", open_depth,
                   close_depth);
          printf("%s", prompt);
          if (line == NULL) line = calloc(1, 2048);
          line = fgets(line, 2048, stdin);
          if (NULL != line) chomp(line);
        }

      if ( line ) {
        if ( *(s=stripwhite(line)) ) {
          add_history (s);
          debug_return=execute_line(s);
        } else {
          add_history ("step");
          debug_return=dbg_cmd_step((char *) "");
        }
        free (line);
      } else {
        dbg_cmd_quit(NULL);
      }
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
