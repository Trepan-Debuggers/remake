/* 
Copyright (C) 2005, 2008, 2011 R. Bernstein <rocky@gnu.org>
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

/** \file dbg_cmd.h 
 *
 *  \brief GNU Make debugger interface
 */

#ifndef DBG_CMD_H
#define DBG_CMD_H
#include "../job.h"
#include "../buildargv.h"
#include "../trace.h"

extern debug_return_t enter_debugger (target_stack_node_t *p, 
				      file_t *p_target, int errcode,
				      debug_enter_reason_t reason);

extern debug_return_t execute_line (char *psz_line);
extern debug_return_t dbg_cmd_help(char *psz_args);
extern debug_return_t dbg_cmd_info(char *psz_args);
extern debug_return_t dbg_cmd_target(char *psz_args);
extern debug_return_t dbg_cmd_show(char *psz_args);
extern debug_return_t dbg_cmd_where(char *psz_args);
extern debug_return_t dbg_cmd_show_command(char *psz_args);

#endif /* DBG_CMD_H*/
/* 
 * Local variables:
 *  c-file-style: "gnu"
 *  indent-tabs-mode: nil
 * End:
 */
