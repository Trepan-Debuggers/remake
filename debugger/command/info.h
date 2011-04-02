/*
Copyright (C) 2008, 2011 R. Bernstein rocky@gnu.org
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

#include "../../file.h"
#include "../../print.h"
#include "../../rule.h"
#include "../../debug.h"
#include "../../vpath.h"
#include "../info.h"
#include "../msg.h"
#include "../stack.h"

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


/*! Show target information: location and name. */
static void 
dbg_cmd_info_target_entry (const file_t *p_target, bool b_verbose) 
{
    const floc_t *p_floc = &p_target->floc;
    if (p_floc) {
        if (p_floc->filenm)
            printf("%s:%lu:\n", p_floc->filenm, p_floc->lineno);
        else if (!b_verbose)
            return;
        printf("\t%s\n", p_target->name);
    }
}

int
dbg_target_compare(const void *p1, const void *p2) 
{
    const struct file *p_target1 = *(const file_t **) p1;
    const struct file *p_target2 = *(const file_t **) p2;
    return strcmp(p_target1->name, p_target2->name);
}

static void
dbg_cmd_info_targets(bool b_verbose)
{
  struct file **file_slot_0 = (struct file **) hash_dump (&files, 0, 
                                                          dbg_target_compare);
  struct file **file_end = file_slot_0 + files.ht_fill;
  struct file **pp_file_slot;
  struct file *p_target;
  
  for (pp_file_slot = file_slot_0; pp_file_slot < file_end; pp_file_slot++) {
    if ((p_target = *pp_file_slot) != NULL)
      dbg_cmd_info_target_entry(p_target, b_verbose);
  }
}

/* Show line information. We want output to be compatible with gdb output.*/
void
dbg_cmd_info_line() 
{
  if (p_stack_top && p_stack_top->p_target && 
      p_stack_top->p_target->floc.filenm) {
    const floc_t *p_floc = &p_stack_top->p_target->floc;
    if (!basename_filenames && strlen(p_floc->filenm) 
        && p_floc->filenm[0] != '/') 
      dbg_msg("Line %lu of \"%s/%s\"", 
              p_floc->lineno, starting_directory,
              p_floc->filenm);
    else 
      dbg_msg("Line %lu of \"%s\"", p_floc->lineno, p_floc->filenm);
  } else {
    dbg_msg("No line number info recorded.\n");
  }
}

void 
dbg_cmd_info_program() 
{
  printf(_("Starting directory `%s'\n"), starting_directory);
  printf(_("Program invocation:\n"));
  printf("\t");
  dbg_print_invocation();
  printf(_("Recursion level: %d\n"), makelevel);
  dbg_cmd_info_line();
  switch (last_stop_reason) 
    {
    case DEBUG_BRKPT_AFTER_CMD:
      printf(_("Program is stopped after running commands.\n"));
      break;
    case DEBUG_BRKPT_BEFORE_PREREQ:
      printf(_("Program stopped at a breakpoint before prequisite checking.\n"));
	  break;
    case DEBUG_BRKPT_AFTER_PREREQ:
      printf(_("Program is stopped after prequisite checking.\n"));
      break;
    case DEBUG_GOAL_UPDATED_HIT:
      printf(_("Program stopped for updating a goal.\n"));
      printf("\n");
      break;
    case DEBUG_READ_HIT:
      printf(_("Program stopped for reading a file.\n"));
      printf("\n");
      break;
	case DEBUG_ERROR_HIT:
	  printf(_("Program stopped after an error encountered.\n"));
	  printf("\n");
	  break;
    case DEBUG_STEP_HIT:
      printf(_("Program stopped in stepping.\n"));
      printf("\n");
      break;
    case DEBUG_STEP_COMMAND:
      printf(_("Program stopped in stepping before running a command.\n"));
      printf("\n");
	  break;
    case DEBUG_NOT_GIVEN:
      printf(_("Reason not given.\n"));
      break;
    }
}

  
/* Give some info regarding the running program. */
debug_return_t 
dbg_cmd_info(char *psz_args)
{
  if (!psz_args || 0==strlen(psz_args)) {
    dbg_cmd_help("info");
  } else {
    char *psz_subcmd = get_word(&psz_args);
    if (is_abbrev_of (psz_subcmd, "line", 2)) {
      dbg_cmd_info_line();
    } else if (is_abbrev_of (psz_subcmd, "locals", 2)) {
      const char *psz_target = NULL;
      char *psz_subcmds   = NULL;
      file_t *p_target = get_target(&psz_subcmds, &psz_target);

      if (p_target) {
	if (!p_target->variables) {
	  initialize_file_variables (p_target, 0);
	  set_file_variables (p_target);
	  if (!p_target->variables) {
	    printf("Can't get variable information for target %s\n", 
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
    } else if (is_abbrev_of (psz_subcmd, "breakpoints", 1)) {
      list_breakpoints();
    } else if (is_abbrev_of (psz_subcmd, "makefiles", 1) ||
	       is_abbrev_of (psz_subcmd, "files", 2)) {
      if (0 == strlen(psz_args))
	print_read_makefiles();
      else if (0 == strcmp(psz_args, "verbose"))
	print_file_data_base ();
      else 
	dbg_cmd_target(psz_args);
    } else if (is_abbrev_of (psz_subcmd, "frame", 2)) {
      dbg_cmd_where(psz_args);
    } else if (is_abbrev_of (psz_subcmd, "program", 1)) {
      dbg_cmd_info_program();
    } else if (is_abbrev_of (psz_subcmd, "rules", 1)) {
      if (0 == strlen(psz_args))
	print_rule_data_base (false);
      else if (0 == strcmp(psz_args, "verbose"))
	print_rule_data_base (true);
      else 
	{
	  rule_t *r = find_rule(psz_args);
	  if (r) 
	    print_rule(r, true);
	  else
	    printf(_("Rule %s not found.\n"), psz_args);
	}
    } else if (is_abbrev_of (psz_subcmd, "stack", 1)) {
        print_target_stack(p_stack_top, i_stack_pos, MAX_STACK_SHOW);
    } else if (is_abbrev_of (psz_subcmd, "targets", 1)) {
        if (0 == strlen(psz_args))
            dbg_cmd_info_targets (false);
        else if (is_abbrev_of (psz_args, "verbose", 1))
            dbg_cmd_info_targets (true);
        else
            printf("Expecting verbose or nothing\n");
    } else if (is_abbrev_of (psz_subcmd, "variables", 1)) {
      print_variable_data_base();
    } else if (is_abbrev_of (psz_subcmd, "vpath", 1)) {
      print_vpath_data_base ();
    } else if (is_abbrev_of (psz_subcmd, "warranty", 1)) {
      printf("%s", WARRANTY);
    } else {
      printf(_("Undefined command \"%s\". Try \"help info\"\n"), psz_subcmd);
    }
  }
  
  return debug_readloop;
}

static void
dbg_cmd_info_init(unsigned int c) 
{
  short_command[c].func = &dbg_cmd_info;
  short_command[c].use = _("info [SUBCOMMAND]");
  short_command[c].doc = 
    _("Show program information regarding SUBCOMMAND.\n" \
      "\tIf SUBCOMMAND is not specified, give list of \"info\" subcommands.");
}

/* 
 * Local variables:
 *  c-file-style: "gnu"
 *  indent-tabs-mode: nil
 * End:
 */
