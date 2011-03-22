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

unsigned int i_breakpoints = 0;

/*! Add "p_target" to the list of breakpoints. Return true if 
    there were no errors
*/
bool 
add_breakpoint (file_t *p_target, unsigned int brkpt_mask) 
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
    printf(_("Note: prerequisite breakpoint already set at target %s.\n"), 
	   p_target->name);
  } 
  if (p_target->tracing & (BRK_AFTER_PREREQ & brkpt_mask)) {
    printf(_("Note: command breakpoint already set at target %s.\n"), 
	   p_target->name);
  } 
  if (p_target->tracing & (BRK_AFTER_CMD & brkpt_mask)) {
    printf(_("Note: target end breakpont set at target %s.\n"), 
	   p_target->name);
  }
  p_target->tracing = brkpt_mask;
  printf(_("Breakpoint %d on target %s"), i_breakpoints, p_target->name);
  if (p_target->floc.filenm)
    printf(": file %s, line %lu.\n", p_target->floc.filenm,
           p_target->floc.lineno);
  else
    printf(".\n");
  if (p_target->updated)
      printf("Warning: target is already updated; so it might not get stopped at again\n");
  else if (p_target->updating) {
      printf("Warning: target is in the process of being updated;\n");
      printf("so it might not get stopped at again\n");
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
    printf(_("Invalid Breakpoint number 0.\n"));
    return false;
  }
  if (i > i_breakpoints) {
    printf(_("Breakpoint number %d is too high. " 
	   "%d is the highest breakpoint number.\n"), i, i_breakpoints);
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
	printf(_("Breakpoint %d on target %s cleared\n"), 
	       i, p->p_target->name);
	free(p);
	return true;
      } else {
	printf(_("No breakpoint at target %s; nothing cleared.\n"), 
	       p->p_target->name);
	free(p);
	return false;
      }
    } else {
      printf(_("No Breakpoint number %d set.\n"), i);
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
    printf(_("No breakpoints.\n"));
    return;
  }

  printf(  "Num Type           Disp Enb target     What\n");
  for (p = p_breakpoint_top; p; p = p->p_next) {
    printf("%3d breakpoint     keep y   in %s", 
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
 
