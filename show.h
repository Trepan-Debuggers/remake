/* $Id: show.h,v 1.1 2004/04/04 02:03:56 rockyb Exp $

Copyright (C) 2004 Free Software Foundation, Inc.
  Author: Rocky Bernstein rocky@panix.com

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

/* Header canonical routines for showing things:
   target location or variable information.
*/

#ifndef SHOW_H
#define SHOW_H

#include "variable.h"
#include "job.h"
#include "trace.h"

/*! Display a variable and its value. */
extern void show_variable PARAMS ((const variable_t *p_v));

/*! Display common file target prefix message output file target. */
extern void show_file_target_prefix PARAMS ((const file_t *p_target));

/*! Display common target prefix message. */
extern void show_target_prefix PARAMS ((const char *p_name));

/*! Display common prefix message output file target. */
extern void show_floc_prefix PARAMS ((const floc_t *p_floc));

/*! Display common prefix message output file target. */
extern void show_child_cmd PARAMS ((child_t *p_child));

/*! Display common prefix message output file target. */
extern void show_call_stack PARAMS ((target_stack_node_t *p));

#endif /*SHOW_H*/
