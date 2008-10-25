/* $Id: dbg_cmd.c,v 1.82 2007/03/01 12:49:59 rockyb Exp $
Copyright (C) 2008 R. Bernstein rocky@gnu.org
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

/* debugger command interface. */

#include "file.h"
#include "print.h"
#include "dbg_cmd.h"
#include "dbg_fns.h"
#include "dbg_info.h"
#include "dbg_stack.h"
#include "debug.h"

const char *WARRANTY = 
"			    NO WARRANTY\n"
"\n"
"  11. BECAUSE THE PROGRAM IS LICENSED FREE OF CHARGE, THERE IS NO WARRANTY\n"
"FOR THE PROGRAM, TO THE EXTENT PERMITTED BY APPLICABLE LAW.  EXCEPT WHEN\n"
"OTHERWISE STATED IN WRITING THE COPYRIGHT HOLDERS AND/OR OTHER PARTIES\n"
"PROVIDE THE PROGRAM \"AS IS\" WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED\n"
"OR IMPLIED, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF\n"
"MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.  THE ENTIRE RISK AS\n"
"TO THE QUALITY AND PERFORMANCE OF THE PROGRAM IS WITH YOU.  SHOULD THE\n"
"PROGRAM PROVE DEFECTIVE, YOU ASSUME THE COST OF ALL NECESSARY SERVICING,\n"
"REPAIR OR CORRECTION.\n"
"\n"
"  12. IN NO EVENT UNLESS REQUIRED BY APPLICABLE LAW OR AGREED TO IN WRITING\n"
"WILL ANY COPYRIGHT HOLDER, OR ANY OTHER PARTY WHO MAY MODIFY AND/OR\n"
"REDISTRIBUTE THE PROGRAM AS PERMITTED ABOVE, BE LIABLE TO YOU FOR DAMAGES,\n"
"INCLUDING ANY GENERAL, SPECIAL, INCIDENTAL OR CONSEQUENTIAL DAMAGES ARISING\n"
"OUT OF THE USE OR INABILITY TO USE THE PROGRAM (INCLUDING BUT NOT LIMITED\n"
"TO LOSS OF DATA OR DATA BEING RENDERED INACCURATE OR LOSSES SUSTAINED BY\n"
"YOU OR THIRD PARTIES OR A FAILURE OF THE PROGRAM TO OPERATE WITH ANY OTHER\n"
"PROGRAMS), EVEN IF SUCH HOLDER OR OTHER PARTY HAS BEEN ADVISED OF THE\n"
"POSSIBILITY OF SUCH DAMAGES.\n";


/* Give some help info. */
debug_return_t 
dbg_cmd_info (char *psz_arg)
{
  if (!psz_arg || 0==strlen(psz_arg)) {
    dbg_cmd_help("info");
  } else {
    if (is_abbrev_of (psz_arg, "line", 2)) {
      /* We want output to be compatible with gdb output.*/
      if (p_stack_top && p_stack_top->p_target && 
	  p_stack_top->p_target->floc.filenm) {
	const floc_t *p_floc = &p_stack_top->p_target->floc;
	if (!basename_filenames && strlen(p_floc->filenm) 
	    && p_floc->filenm[0] != '/') 
	  printf("Line %lu of \"%s/%s\"\n", 
		 p_floc->lineno, starting_directory,
		 p_floc->filenm);
	else 
	  printf("Line %lu of \"%s\"\n", p_floc->lineno, p_floc->filenm);
      } else {
	printf("No line number info recorded.\n");
      }
      
    } else if (is_abbrev_of (psz_arg, "locals", 2)) {
      char *psz_target = NULL;
      char *psz_args   = NULL;
      file_t *p_target = get_target(&psz_args, &psz_target);

      if (p_target) {
	if (!p_target->variables) {
	  initialize_file_variables (p_target, 0);
	  set_file_variables (p_target);
	  if (!p_target->variables) {
	    printf("Can't get varible information for target %s\n", 
		   psz_target);
	    return debug_readloop;
	  }
	}
      } else {
	printf("No target information.\n");
	return debug_readloop;
      }
      hash_map_arg (&p_target->variables->set->table, 
		    print_variable_info, NULL);
    } else if (is_abbrev_of (psz_arg, "breakpoints", 1)) {
      list_breakpoints();
    } else if (is_abbrev_of (psz_arg, "files", 1)) {
      print_read_makefiles(true);
    } else if (is_abbrev_of (psz_arg, "makefiles", 1)) {
      print_read_makefiles(false);
    } else if (is_abbrev_of (psz_arg, "program", 1)) {
      switch (last_stop_reason) 
	{
	case DEBUG_BREAKPOINT_HIT:
	  printf("Program stopped at a breakpoint.a\n");
	  break;
	case DEBUG_GOAL_UPDATED_HIT:
	  printf("Program stopped for updating a goal.\n");
	  printf("\n");
	  break;
	case DEBUG_READ_HIT:
	  printf("Program stopped for reading a file.\n");
	  printf("\n");
	  break;
	case DEBUG_ERROR_HIT:
	  printf("Program stopped after an error encountered.\n");
	  printf("\n");
	  break;
	case DEBUG_STEP_HIT:
	  printf("Program stopped in stepping.\n");
	  printf("\n");
	  break;
	}
    } else if (is_abbrev_of (psz_arg, "rules", 1)) {
      print_rule_data_base ();
    } else if (is_abbrev_of (psz_arg, "stack", 1)) {
      print_target_stack(p_stack_top, i_stack_pos, MAX_STACK_SHOW);
    } else if (is_abbrev_of (psz_arg, "target", 1)) {
      if (p_stack_top && p_stack_top->p_target && p_stack_top->p_target->name)
	printf("target: %s\n", p_stack_top->p_target->name);
      else 
	printf("target unknown\n");
    } else if (is_abbrev_of (psz_arg, "variables", 1)) {
      print_variable_data_base();
    } else if (is_abbrev_of (psz_arg, "vpath", 1)) {
      print_vpath_data_base ();
    } else if (is_abbrev_of (psz_arg, "warranty", 1)) {
      printf(WARRANTY);
    } else {
      printf(_("Undefined command \"%s\". Try \"help info\"\n"), psz_arg);
    }
  }
  
  return debug_readloop;
}

