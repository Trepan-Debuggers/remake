/*
Copyright (C) 2020 R. Bernstein <rocky@gnu.org>

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

/** \file libdebugger/cmd_initialize.h
 *
 *  \brief Debugger command initialization.
 *
 */

#include "command/help/break.h"
#include "command/help/chdir.h"
#include "command/help/comment.h"
#include "command/help/continue.h"
#include "command/help/delete.h"
#include "command/help/down.h"
#include "command/help/edit.h"
#include "command/help/expand.h"
#include "command/help/finish.h"
#include "command/help/frame.h"
#include "command/help/help.h"
#include "command/help/info.h"
#include "command/help/load.h"
#include "command/help/list.h"
#include "command/help/next.h"
#include "command/help/print.h"
#include "command/help/pwd.h"
#include "command/help/quit.h"
#include "command/help/run.h"
#include "command/help/set.h"
#include "command/help/setq.h"
#include "command/help/setqx.h"
#include "command/help/shell.h"
#include "command/help/show.h"
#include "command/help/source.h"
#include "command/help/skip.h"
#include "command/help/step.h"
#include "command/help/target.h"
#include "command/help/up.h"
#include "command/help/where.h"
#include "command/help/write.h"

/* A structure which contains information on the commands this program
   can understand. */

/* Should be in alphabetic order by command name. */
long_cmd_t dbg_commands[] = {
  { "break",    'b' },
  { "cd",       'C' },
  { "comment",  '#' },
  { "continue", 'c' },
  { "delete",   'd' },
  { "down",     'D' },
  { "edit" ,    'e' },
  { "expand" ,  'x' },
  { "finish"  , 'F' },
  { "frame"   , 'f' },
  { "help"    , 'h' },
  { "info"    , 'i' },
  { "list"    , 'l' },
  { "load"    , 'M' },
  { "next"    , 'n' },
  { "print"   , 'p' },
  { "pwd"     , 'P' },
  { "quit"    , 'q' },
  { "run"     , 'R' },
  { "set"     , '=' },
  { "setq"    , '"' },
  { "setqx"   , '`' },
  { "shell"   , '!' },
  { "show"    , 'S' },
  { "skip"    , 'k' },
  { "source"  , '<' },
  { "step"    , 's' },
  { "target"  , 't' },
  { "up"      , 'u' },
  { "where"   , 'T' },
  { "write"   , 'w' },
  { (char *)NULL, ' '}
};

/* Should be in alphabetic order by ALIAS name. */

alias_cmd_t aliases[] = {
  { "shell",    "!!" },
  { "help",     "?" },
  { "break",    "L" },
  { "where",    "backtrace" },
  { "where",    "bt" },
  { "quit",     "exit" },
  { "run",      "restart" },
  { "quit",     "return" },
  { (char *)NULL, (char *) NULL}
};


/* FIXME: folded "init" routines into the macro them. */
static void
dbg_cmd_break_init(unsigned int c)
{
  short_command[c].func = &dbg_cmd_break;
  short_command[c].use  = _("break [*target*|*linenum*] [all|run|prereq|end]*");
}

static void
dbg_cmd_chdir_init(unsigned int c)
{
  short_command[c].func = &dbg_cmd_chdir;
  short_command[c].use  = _("cd *dir*");
}

static void
dbg_cmd_comment_init(unsigned int c)
{
  short_command[c].func = &dbg_cmd_comment;
  short_command[c].use  = _("comment *text*");
}

static void
dbg_cmd_continue_init(unsigned int c)
{
  short_command[c].func = &dbg_cmd_continue;
  short_command[c].use  = _("continue [*target* [all|run|prereq|end]*]");
}

static void
dbg_cmd_delete_init(unsigned int c)
{
  short_command[c].func = &dbg_cmd_delete;
  short_command[c].use  = _("delete *breakpoint-numbers*...");
}


static void
dbg_cmd_edit_init(unsigned int c)
{
  short_command[c].func = &dbg_cmd_edit;
  short_command[c].use = _("edit");
}

static void
dbg_cmd_expand_init(unsigned int c)
{
  short_command[c].func = &dbg_cmd_expand;
  short_command[c].use  = _("expand *string*");
}

static void
dbg_cmd_down_init(unsigned int c)
{
  short_command[c].func = &dbg_cmd_down;
  short_command[c].use  = _("down [*amount*]");
}

static void
dbg_cmd_finish_init(unsigned int c)
{
  short_command[c].func = &dbg_cmd_finish;
  short_command[c].use  = _("finish [*amount*]");
}

static void
dbg_cmd_frame_init(unsigned int c)
{
  short_command[c].func = &dbg_cmd_frame;
  short_command[c].use  = _("frame *n*");
}

static void
dbg_cmd_help_init(unsigned int c)
{
  short_command[c].func = &dbg_cmd_help;
  short_command[c].use  = _("help [*command*]");
}

static void
dbg_cmd_list_init(unsigned int c)
{
  short_command[c].func = &dbg_cmd_list;
  short_command[c].use = _("list [*target*|*line-number*]");
}

static void
dbg_cmd_next_init(unsigned int c)
{

  short_command[c].func = &dbg_cmd_next;
  short_command[c].use = _("next [*amount*]");
}

static void
dbg_cmd_info_init(unsigned int c)
{
  short_command[c].func = &dbg_cmd_info;
  short_command[c].use = _("info [*subcommand*]");
}

static void
dbg_cmd_load_init(unsigned int c)
{
  short_command[c].func = &dbg_cmd_load;
  short_command[c].use = _("load *file-glob*");
}

static void
dbg_cmd_pwd_init(unsigned int c)
{
  short_command[c].func = &dbg_cmd_pwd;
  short_command[c].use = _("pwd");
}

static void
dbg_cmd_print_init(unsigned int c)
{
  short_command[c].func = &dbg_cmd_print;
  short_command[c].use = _("print {*variable* [*attrs*...]}");
}

static void
dbg_cmd_quit_init(unsigned int c)
{
  short_command[c].func = &dbg_cmd_quit;
  short_command[c].use = _("quit [*exit-status*]");
}

static void
dbg_cmd_run_init(unsigned int c)
{
  short_command[c].func = &dbg_cmd_run;
  short_command[c].use = _("run [*args*]");
  short_command[c].doc =
    _("Run Makefile from the beginning.\n"
      "You may specify arguments to give it.\n"
      "With no arguments, uses arguments last specified (with \"run\")");
}

static void
dbg_cmd_set_init(unsigned int c)
{

  short_command[c].func = &dbg_cmd_set;
  short_command[c].use =
    _("set *option* {on|off|toggle}");
}

static void
dbg_cmd_setq_init(unsigned int c)
{
  short_command[c].func = &dbg_cmd_setq;
  short_command[c].use  = _("setq *variable* *value*");
}

static void
dbg_cmd_setqx_init(unsigned int c)
{
  short_command[c].func = &dbg_cmd_setqx;
  short_command[c].use  = _("setqx *variable* *value");
}

static void
dbg_cmd_shell_init(unsigned int c)
{
  short_command[c].func = &dbg_cmd_shell;
  short_command[c].use =  _("shell *string*");
}

static void
dbg_cmd_show_init(unsigned int c)
{
  short_command[c].func = &dbg_cmd_show;
  short_command[c].use = _("show [*subcommand]");
}

static void
dbg_cmd_skip_init(unsigned int c)
{
  short_command[c].func = &dbg_cmd_skip;
  short_command[c].use = _("skip");
}

static void
dbg_cmd_source_init(unsigned int c)
{
  short_command[c].func = &dbg_cmd_source;
  short_command[c].use = _("source *file-glob*");
}

static void
dbg_cmd_step_init(unsigned int c)
{
  short_command[c].func = &dbg_cmd_step;
  short_command[c].use = _("step [*amount*]");
}

static void
dbg_cmd_target_init(unsigned int c)
{
  short_command[c].func = &dbg_cmd_target;
  short_command[c].use =  _("target [*target-name] [info1 [info2...]]");
}

static void
dbg_cmd_up_init(unsigned int c)
{
  short_command[c].func = &dbg_cmd_up;
  short_command[c].use  = _("up [*amount*]");
}

static void
dbg_cmd_where_init(unsigned int c)
{
  short_command[c].func = &dbg_cmd_where;
  short_command[c].use =  _("where");
}

static void
dbg_cmd_write_init(unsigned int c)
{
  short_command[c].func = &dbg_cmd_write;
  short_command[c].use =  _("write [*target* [*filename*]]");
}

#define DBG_CMD_INIT(CMD, LETTER, NEEDS_RUNNING)        \
  dbg_cmd_ ## CMD ## _init(LETTER);                     \
  short_command[LETTER].doc = _(CMD ## _HELP_TEXT);     \
  short_command[LETTER].needs_running = NEEDS_RUNNING;  \
  short_command[LETTER].id = id++

static void
cmd_initialize(void)
{
  int id=0;
  DBG_CMD_INIT(break, 'b', false);
  DBG_CMD_INIT(chdir, 'C', false);
  DBG_CMD_INIT(comment, '#', false);
  DBG_CMD_INIT(continue, 'c', true);
  DBG_CMD_INIT(delete, 'd', false);
  DBG_CMD_INIT(down, 'D', false);
  DBG_CMD_INIT(edit, 'e', false);
  DBG_CMD_INIT(expand, 'x', false);
  DBG_CMD_INIT(finish, 'F', true);
  DBG_CMD_INIT(frame, 'f', false);
  DBG_CMD_INIT(help, 'h', false);
  DBG_CMD_INIT(info, 'i', false);
  DBG_CMD_INIT(list, 'l', false);
  DBG_CMD_INIT(load, 'M', false);
  DBG_CMD_INIT(next, 'n', true);
  DBG_CMD_INIT(print, 'p', false);
  DBG_CMD_INIT(pwd, 'P', false);
  DBG_CMD_INIT(quit, 'q', false);
  DBG_CMD_INIT(run, 'R', false);
  DBG_CMD_INIT(set, '=', false);
  DBG_CMD_INIT(setq, '"', false);
  DBG_CMD_INIT(setqx, '`', false);
  DBG_CMD_INIT(shell, '!', false);
  DBG_CMD_INIT(show, 'S', false);
  DBG_CMD_INIT(skip, 'k', true);
  DBG_CMD_INIT(source, '<', false);
  DBG_CMD_INIT(step, 's', true);
  DBG_CMD_INIT(target, 't', false);
  DBG_CMD_INIT(up, 'u', false);
  DBG_CMD_INIT(where, 'T', false);
  DBG_CMD_INIT(write, 'w', false);
}
