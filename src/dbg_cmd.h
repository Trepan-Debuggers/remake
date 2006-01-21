/* $Id: dbg_cmd.h,v 1.8 2006/01/21 13:40:21 rockyb Exp $
Copyright (C) 2005 rocky@panix.com
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

/** \file dbg_cmd.h 
 *
 *  \brief GNU Make debugger interface
 */

#ifndef DBG_CMD_H
#define DBG_CMD_H
#include "job.h"
#include "buildargv.h"

/*! 
  If 0 (or false) we are not in the debugger command read loop.
  If 1 (or true) we are in the debugger command read loop.
  If DEBUGGER_QUIT_RC we've requested to quit.
 */
extern int in_debugger;

/*!
  debugger command interface. 
*/
extern debug_return_t enter_debugger (target_stack_node_t *p, 
				      file_t *p_target, int err);

#endif /* DBG_CMD_H*/
