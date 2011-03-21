/* $Id: dbg_cmd.h,v 1.8 2006/01/21 13:40:21 rockyb Exp $
Copyright (C) 2005, 2008 R. Bernstein <rocky@gnu.org>
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
#include "job.h"
#include "buildargv.h"
#include "trace.h"

/*! 
  If 0 (or false) we are not in the debugger command read loop.
  If 1 (or true) we are in the debugger command read loop.
  If DEBUGGER_QUIT_RC we've requested to quit.
 */
extern int in_debugger;

typedef enum 
  {
    DEBUG_BREAKPOINT_HIT   = 1,
    DEBUG_GOAL_UPDATED_HIT = 2,
    DEBUG_READ_HIT         = 3,
    DEBUG_ERROR_HIT        = 4,
    DEBUG_STEP_HIT         = 5
  } debug_enter_reason_t;

/*!
  debugger command interface. 
*/
extern debug_return_t enter_debugger (target_stack_node_t *p, 
				      file_t *p_target, int err,
				      debug_enter_reason_t reason);

extern debug_return_t dbg_cmd_help();
extern debug_return_t dbg_cmd_info();
extern debug_return_t dbg_cmd_target();
extern debug_return_t dbg_cmd_show();
extern debug_return_t dbg_cmd_where();
extern debug_return_t dbg_cmd_show_command();

#endif /* DBG_CMD_H*/
