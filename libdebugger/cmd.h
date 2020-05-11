/*
Copyright (C) 2005, 2008, 2011, 2020 R. Bernstein <rocky@gnu.org>
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

/** \file libdebugger/cmd.h
 *
 *  \brief Debugger interface header
 */

#ifndef REMAKE_DBG_CMD_H
#define REMAKE_DBG_CMD_H
#include "../src/job.h"
#include "../src/buildargv.h"
#include "../src/trace.h"
#include "debugger.h"
#include "subcmd.h"

/*!
   Command-line args after the command-name part. For example in:
   break foo
   the below will be "foo".
 **/
extern char *psz_debugger_args;

/**
   Think of the below not as an enumeration but as C-preprocessor
   defines done in a way that we'll be able to use the value in a gdb.
 **/
enum {
  MAX_FILE_LENGTH   = 1000,
} debugger_enum1;


typedef struct {
  const char *command;	        /* real command name. */
  const char *alias;	        /* alias for command. */
} alias_cmd_t;

extern alias_cmd_t aliases[];
extern long_cmd_t dbg_commands[];
extern short_cmd_t short_command[256];

extern subcommand_var_info_t info_subcommands[];
extern subcommand_var_info_t set_subcommands[];
extern subcommand_var_info_t show_subcommands[];


extern debug_return_t enter_debugger (target_stack_node_t *p,
				      file_t *p_target, int errcode,
				      debug_enter_reason_t reason);

extern debug_return_t execute_line (char *psz_line);
extern debug_return_t dbg_cmd_help(char *psz_args);
extern debug_return_t dbg_cmd_info(char *psz_args);
extern debug_return_t dbg_cmd_target(char *psz_args);
extern debug_return_t dbg_cmd_show(char *psz_args);
extern debug_return_t dbg_cmd_watch(char *psz_regex);
extern debug_return_t dbg_cmd_where(char *psz_args);
extern debug_return_t dbg_cmd_set(char *psz_args);
extern debug_return_t dbg_cmd_set_var (char *psz_arg, int expand);
extern debug_return_t dbg_cmd_show_command(const char *psz_args);
/*! Show just a list of targets */
extern void dbg_cmd_info_targets(info_target_output_mask_t output_mask);
/*! Show just a list of tasks */
extern void dbg_cmd_info_tasks();


/*! Look up NAME as the name of a command, and return a pointer to that
  command.  Return a NULL pointer if NAME isn't a command name. */
short_cmd_t * find_command (const char *psz_name);



#endif /* DBG_CMD_H*/
/*
 * Local variables:
 *  c-file-style: "gnu"
 *  indent-tabs-mode: nil
 * End:
 */
