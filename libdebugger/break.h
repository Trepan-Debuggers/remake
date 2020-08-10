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

/** \file libdebugger/break.h
 *
 *  \brief debugger command beakpoint routines.
 */

#ifndef DBG_BREAK_H
#define DBG_BREAK_H

#include "types.h"
#include "trace.h"

/*! Opaque type definition for an item in the breakpoint list. */
typedef struct breakpoint_node breakpoint_node_t;

/** Pointers to top of current breakpoint list. */
extern breakpoint_node_t *p_breakpoint_top;

/** Pointers to bottom of current breakpoint list. */
extern breakpoint_node_t *p_breakpoint_bottom;

/** The largest breakpoint number previously given. When a new
    breakpoint is set it will be i_breakpoints+1. */
extern unsigned int i_breakpoints;

/*! Add "p_target" to the list of breakpoints. Return true if
    there were no errors
*/
extern bool add_breakpoint (file_t *p_target, unsigned int brkp_mask);

/*! Add "psz_regex" command watchpoint to the list of breakpoints.
    Return true if there were no errors.
*/
extern bool add_command_watchpoint (const char *psz_regex);

/*! Remove breakpoint i from the list of breakpoints. Return true if
    there were no errors. If silent is true, then don't warn about
    not finding breakpoint at "i".
*/
extern bool remove_breakpoint (unsigned int i, bool silent);

/*! List breakpoints.*/
extern void list_breakpoints (void);

extern void check_command_watchpoint (target_stack_node_t *p_call_stack, file_t *p_target, const char *psz_expanded_command);

#endif /* DBG_BREAK_H */
