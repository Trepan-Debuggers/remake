/* 
Copyright (C) 2005, 2007, 2008 R. Bernstein <rocky@gnu.org>
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

/** debugger command stack routines. */

#include <assert.h>
#include "break.h"
#include "msg.h"
#include "filedef.h"
#include "print.h"

/*! Node for an item in the target call stack */
struct breakpoint_node
{
  file_t            *p_target;
  unsigned int      i_num;
  breakpoint_node_t *p_next;
};

/** Pointers to top/bottom of current breakpoint target stack */
breakpoint_node_t *p_breakpoint_top    = NULL;
breakpoint_node_t *p_breakpoint_bottom = NULL;

brkpt_mask_t i_breakpoints = BRK_NONE;

/*! Add "p_target" to the list of breakpoints. Return true if 
    there were no errors
*/
bool 
add_breakpoint (file_t *p_target, const brkpt_mask_t brkpt_mask) 
{
  breakpoint_node_t *p_new   = CALLOC (breakpoint_node_t, 1);

  if (!p_new) return false;

  /* Add breakpoint to list of breakpoints. */
  if (!p_breakpoint_top) {
    assert(!p_breakpoint_bottom);
    p_breakpoint_top            = p_breakpoint_bottom = p_new;
  } else {
    p_breakpoint_bottom->p_next = p_new;
  }
  p_breakpoint_bottom           = p_new;
  p_new->p_target               = p_target;
  p_new->i_num                  = ++i_breakpoints;


  /* Finally, note that we are tracing this target. */
  if (p_target->tracing & (BRK_BEFORE_PREREQ & brkpt_mask)) {
    dbg_msg(_("Note: prerequisite breakpoint already set at target %s."), 
            p_target->name);
  } 
  if (p_target->tracing & (BRK_AFTER_PREREQ & brkpt_mask)) {
    dbg_msg(_("Note: command breakpoint already set at target %s."), 
            p_target->name);
  } 
  if (p_target->tracing & (BRK_AFTER_CMD & brkpt_mask)) {
    dbg_msg(_("Note: target end breakpont set at target %s."), 
            p_target->name);
  }
  p_target->tracing = brkpt_mask;
  printf(_("Breakpoint %d on target %s"), i_breakpoints, p_target->name);
  if (p_target->floc.filenm)
    dbg_msg(": file %s, line %lu.", p_target->floc.filenm,
            p_target->floc.lineno);
  else
    printf(".\n");
  if (p_target->updated)
      dbg_msg("Warning: target is already updated; so it might not get stopped at again.");
  else if (p_target->updating && (brkpt_mask & (BRK_BEFORE_PREREQ | BRK_AFTER_PREREQ))) {
      dbg_msg("Warning: target is in the process of being updated;");
      dbg_msg("so it might not get stopped at again.");
  }
  return true;
  
}

/*! Remove breakpoint i from the list of breakpoints. Return true if 
    there were no errors
*/
bool 
remove_breakpoint (unsigned int i) 
{
  if (!i) {
    dbg_msg(_("Invalid Breakpoint number 0."));
    return false;
  }
  if (i > i_breakpoints) {
    dbg_msg(_("Breakpoint number %d is too high. " 
	   "%d is the highest breakpoint number."), i, i_breakpoints);
    return false;
  } else {
    /* Find breakpoint i */
    breakpoint_node_t *p_prev = NULL;
    breakpoint_node_t *p;
    for (p = p_breakpoint_top; p && p->i_num != i; p = p->p_next) {
      p_prev = p;
    }

    if (p && p->i_num == i) {
      /* Delete breakpoint */
      if (!p->p_next) p_breakpoint_bottom = p_prev;
      if ( (p == p_breakpoint_top) ) p_breakpoint_top = p->p_next;

      if (p_prev) p_prev->p_next = p->p_next;

      if (p->p_target->tracing) {
	p->p_target->tracing = BRK_NONE;
	dbg_msg(_("Breakpoint %d on target %s cleared"),
	       i, p->p_target->name);
	free(p);
	return true;
      } else {
	dbg_msg(_("No breakpoint at target %s; nothing cleared."),
	       p->p_target->name);
	free(p);
	return false;
      }
    } else {
      dbg_msg(_("No Breakpoint number %d set."), i);
      return false;
    }
  }
}

/*! List breakpoints.*/
void
list_breakpoints (void) 
{
  breakpoint_node_t *p;

  if (!p_breakpoint_top) {
    dbg_msg(_("No breakpoints."));
    return;
  }

  dbg_msg(  "Num Type           Disp Enb Target     Location");
  for (p = p_breakpoint_top; p; p = p->p_next) {
    printf("%3d breakpoint     keep y   %s", 
	   p->i_num,
	   p->p_target->name);
    if (p->p_target->floc.filenm) {
	printf(" at ");
	print_floc_prefix(&(p->p_target->floc));
    }
    printf("\n");
  }
}
/* 
 * Local variables:
 * eval: (c-set-style "gnu")
 * indent-tabs-mode: nil
 * End:
 */
 
