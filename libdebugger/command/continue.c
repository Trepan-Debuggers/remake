/*
Copyright (C) 2011, 2015, 2020 R. Bernstein <rocky@gnu.org>
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
/* Continue running program. */

/** \file libdebugger/command/continue.c
 *
 *  \brief Debugger `continue` command.
 *
 *  Debugger command to continue running.
 */

#include "../../src/trace.h"
#include "../../src/expand.h"
#include "../../src/globals.h"
#include "../break.h"
#include "../fns.h"
#include "../stack.h"

extern debug_return_t
dbg_cmd_continue (char *psz_args)
{
  if (psz_args && *psz_args) {
    char *psz_target = get_word(&psz_args);
    file_t *p_target = NULL;
    brkpt_mask_t i_brkpt_mask;

    /** FIXME: DRY with code in break.h **/
    if (p_stack && p_stack->p_target) {
      char *psz_expanded_target =
        variable_expand_set(psz_target, p_stack->p_target->variables);
      if (*psz_expanded_target) {
        p_target = lookup_file(psz_expanded_target);
      }
    } else
      p_target = lookup_file(psz_target);

    if (!p_target) {
      printf("Can't find target %s; breakpoint not set.\n", psz_target);
	return debug_cmd_error;
    }

    /* FIXME: Combine with code in continue. */
    psz_args = get_word(&psz_args);
    if (!(psz_args && *psz_args))
      i_brkpt_mask = BRK_ALL;
    else {
      char *psz_break_type;
      i_brkpt_mask = get_brkpt_option(psz_args);
      while ((psz_break_type = get_word(&psz_args))) {
        if (!(psz_break_type && *psz_break_type)) break;
        i_brkpt_mask |= get_brkpt_option(psz_break_type) ;
      }
    }

    if (!add_breakpoint(p_target, i_brkpt_mask|BRK_TEMP))
      return debug_cmd_error;
  } else  {
    db_level = 0;
  }

  i_debugger_stepping = 0;
  i_debugger_nexting  = 0;
  debugger_flag = 0;
  define_makeflags (1, 0);
  return continue_execution;
};

/*
 * Local variables:
 * eval: (c-set-style "gnu")
 * indent-tabs-mode: nil
 * End:
 */
