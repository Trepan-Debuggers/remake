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

/** \file libdebugger/command/list.c
 *
 *  \brief Debugger command to list Makefile target information
 */

#include "../../src/trace.h"
#include "../../src/print.h"
#include "../file2line.h"
#include "../cmd.h"
#include "../fns.h"
#include "../msg.h"
#include "../stack.h"

#define DEPENDS_COMMANDS " depends commands"
extern debug_return_t
dbg_cmd_list(char *psz_arg)
{
  const char *psz_target = NULL;
  char   *target_cmd = NULL;
  file_t *p_target;

  if (psz_arg && 0 == strcmp(psz_arg, "-")) {
    /* Show info for parent target. */
    if (p_stack_top) {
      /* We have a target stack  */
      target_stack_node_t *p=p_stack;

      if (!p) {
	dbg_errmsg(_("We don't seem to have a target to get parent of."));
	return debug_cmd_error;
      }

      p = p->p_parent;
      if (!p) {
	dbg_errmsg(_("We don't seem to have a parent target."));
	return debug_cmd_error;
      }
      p_target = p->p_target;
      psz_target = p_target->name;
    } else {
	dbg_errmsg(_("We don't seem to have a target stack to get parent of."));
	return debug_cmd_error;
    }
  } else {
    unsigned int u_lineno=0;
    f2l_entry_t entry_type;
    if (get_uint(psz_arg, &u_lineno, false)) {
      if (p_stack) {
        p_target = target_for_file_and_line(p_stack->p_target->floc.filenm,
                                            u_lineno, &entry_type);
        if (!p_target) {
          dbg_errmsg("Can't find target or pattern on line %s.\n"
                     "Use 'info lines' to get a list of and pattern lines.",
                     psz_arg);
          return debug_cmd_error;
        }
        psz_target = p_target->name;
      } else {
	dbg_errmsg(_("We don't seem to have a target stack to get parent of."));
	return debug_cmd_error;
      }
    } else
      p_target = get_target(&psz_arg, &psz_target);
  }

  if (!p_target) {
    dbg_errmsg(_("Trouble getting a target name for %s."), psz_target);
    return debug_cmd_error;
  }
  print_floc_prefix(&p_target->floc);
  if (p_target->description) printf("\n");
  target_cmd = CALLOC(char, strlen(psz_target) + 1 + strlen(DEPENDS_COMMANDS));
  sprintf(target_cmd, "%s%s", psz_target, DEPENDS_COMMANDS);
  return dbg_cmd_target(target_cmd);
}


/*
 * Local variables:
 * eval: (c-set-style "gnu")
 * indent-tabs-mode: nil
 * End:
 */
