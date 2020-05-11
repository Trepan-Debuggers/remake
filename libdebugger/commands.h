/*
Copyright (C) 2004-2005, 2007-2009, 2011, 2020 R. Bernstein
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

/** \file libdebugger/commands.h
 *
 *  \brief Header with all debugger command functions.
 *
*/

extern debug_return_t dbg_cmd_break (char *psz_args);
extern debug_return_t dbg_cmd_chdir (char *psz_args);
extern debug_return_t dbg_cmd_comment (char *psz_args);
extern debug_return_t dbg_cmd_continue (char *psz_args);
extern debug_return_t dbg_cmd_delete(char *psz_args);
extern debug_return_t dbg_cmd_edit(char *psz_args);
extern debug_return_t dbg_cmd_expand(char *psz_args);
extern debug_return_t dbg_cmd_eval(char *psz_args);
extern debug_return_t dbg_cmd_down(char *psz_args);
extern debug_return_t dbg_cmd_finish(char *psz_args);
extern debug_return_t dbg_cmd_frame(char *psz_args);
extern debug_return_t dbg_cmd_help(char *psz_args);
extern debug_return_t dbg_cmd_list(char *psz_args);
extern debug_return_t dbg_cmd_load(char *psz_args);
extern debug_return_t dbg_cmd_list(char *psz_args);
extern debug_return_t dbg_cmd_next(char *psz_args);
extern debug_return_t dbg_cmd_print(char *psz_args);
extern debug_return_t dbg_cmd_pwd(char *psz_args);
extern debug_return_t dbg_cmd_quit(char *psz_args);
extern debug_return_t dbg_cmd_run(char *psz_args);
extern debug_return_t dbg_cmd_setq(char *psz_args);
extern debug_return_t dbg_cmd_setqx(char *psz_args);
extern debug_return_t dbg_cmd_shell(char *psz_args);
extern debug_return_t dbg_cmd_show(char *psz_args);
extern debug_return_t dbg_cmd_skip(char *psz_args);
extern debug_return_t dbg_cmd_source(char *psz_args);
extern debug_return_t dbg_cmd_step(char *psz_args);
extern debug_return_t dbg_cmd_target(char *psz_args);
extern debug_return_t dbg_cmd_up(char *psz_args);
extern debug_return_t dbg_cmd_watch(char *psz_args);
extern debug_return_t dbg_cmd_where(char *psz_args);
extern debug_return_t dbg_cmd_write(char *psz_args);
