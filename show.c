/* $Id: show.c,v 1.1 2004/04/04 02:03:56 rockyb Exp $

Copyright (C) 2004 Free Software Foundation, Inc.
  Author: Rocky Bernstein rocky@panix.com

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

/* Routines for showing things:
   target location or variable information.
*/

#include "make.h"
#include "show.h"
#include "debug.h"

/*! Display a variable and its value. */
void 
show_variable (const variable_t *p_v)
{
  printf("%s:%d %s = %s\n", 
	 p_v->fileinfo.filenm, p_v->fileinfo.lineno,
	 p_v->name, p_v->value);
}

/*! Show a command before executing it. */
extern void 
show_target_prefix (const char *p_name) 
{
  printf(" %s", p_name);
  if (makelevel != 0) {
    printf ("[%u]", makelevel);
  }
}

/*! Show a command before executing it. */
extern void 
show_file_target_prefix (const file_t *p_target) 
{
  show_floc_prefix(&(p_target->floc));
  show_target_prefix(p_target->name);
}

/*! Show a command before executing it. */
extern void 
show_floc_prefix (const floc_t *p_floc) 
{
  printf("%s:%d", p_floc->filenm, p_floc->lineno);
}

/*! Show a command before executing it. */
extern void 
show_child_cmd (child_t *p_child)
{
  if (!p_child) return;
  
  /* +++ HERE IS WHERE DEBUG WOULD STOP ++++ */
  if (tracing || p_child->tracing) {
    p_child->fileinfo.lineno += (p_child->command_line-1);
    show_floc_prefix(&(p_child->fileinfo));
    show_target_prefix(p_child->file->name);
    printf("\n\t");
    p_child->fileinfo.lineno -= (p_child->command_line-1);
  }
}

/*! Display common prefix message output file target. */
extern void 
show_call_stack (target_stack_node_t *p)
{
  fprintf(stderr, "*** target dependency stack:\n");
  for ( ; p ; p = p->p_parent ) {
    const floc_t *p_floc = &(p->p_target->floc);
    if (p_floc->filenm)
    fprintf (stderr, "%s:%lu: %s\n", 
	     p_floc->filenm, p_floc->lineno, p->p_target->name);
  }
}

