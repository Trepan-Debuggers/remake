/* $Id: dbg_stack.h,v 1.2 2005/11/27 01:42:00 rockyb Exp $
Copyright (C) 2005 Free Software Foundation, Inc.
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

/** debugger command stack routines. */

#ifndef DBG_STACK_H
#define DBG_STACK_H

#include "trace.h"

extern int i_stack_pos;

/** Pointer to current target call stack at the place we are currently
   focused on.
 */
extern target_stack_node_t *p_stack;
extern floc_stack_node_t   *p_floc_stack;

/** Move reported target frame postition down by psz_amount. */
debug_return_t dbg_cmd_frame_down (char *psz_amount);

/** Move reported target frame postition to absolute position psz_frame. */
debug_return_t dbg_cmd_frame (char *psz_frame);

/** Move reported target frame postition up by psz_amount. */
debug_return_t dbg_cmd_frame_up (char *psz_amount);

#endif /* DBG_STACK_H */
