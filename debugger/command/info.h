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
#include "../../implicit.h"
#include "../../print.h"
#include "../../rule.h"
#include "../../debug.h"
#include "../../vpath.h"
#include "../info.h"
#include "../msg.h"
#include "../stack.h"
#include "../file2line.h"

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

#include "../subcmd.h"

subcommand_var_info_t info_subcommands[] = {
  { "break",
    "Show list of target breakpoints",
    "\n\nShow list of target breakpoints.",
    NULL, false,
    1},
  { "line",
    "Show where we are currently stopped",
    "\n\nShow line and Makefile name of where we are currently stopped.",
    NULL, false,
    2},
  { "lines",
    "Show possible breakpoint lines for all targets",
    "\n\nShow possible breakpoint lines for all targets.",
    NULL, false,
    2},
  { "locals",
    "Show target local variables and their values",
    "\n\nShow target local variables and their values.",
    NULL, false,
    2},
  { "files",
    "Show read-in Makefiles",
    "\n\nShow read-in Makefiles. The last is the one initially named.",
    NULL, false,
    2},
  { "frame",
    "Show target-stack frame",
    "\n\nShow target-stack frame.",
    NULL, false,
    2},
  { "rules",
    "Show implicit or pattern rules",
    " [VERBOSE]\n\n"
"Show implicit or pattern rules. Add VERBOSE if you want more info."
    ,
    NULL, false,
    1},
  { "program",
    "Show program information and why we are stopped",
    "\n\nShow program information and why we are stopped.",
    NULL, false,
    1},
  { "target",
    "Same as 'target'",
    "\n\nShow specific target information. See 'help target'.",
    NULL, false,
    1},
  { "targets",    
    "Show a list of target names and file locations",
    " [NAMES|POSITIONS|TASKS|ALL]\n\n"
"Show the explicitly-named targets found in read Makefiles.\n"
"Suboptions are as follows:\n"
"  NAMES      -- shows target names,\n"
"  POSITIONS  -- shows the location in the Makefile\n"
"  ALL        -- shows names and location\n"
"  TASKS      -- shows target name if it has commands associated with it\n"
"\n"
"The default is ALL.",
     NULL, false,
    7},
  { "variables",
    "Show all GNU Make variables",
    "\n\nShow all GNU Make variables.",
    NULL, false,
    2},
  { "warranty",
    "Various kinds of warranty you do not have",
    "\n\nVarious kinds of warranty you do not have.",
    NULL, false,
    1},
  { NULL, NULL, NULL, NULL, false, 0}
};

/*! Show target information: location and name. */
static void 
dbg_cmd_info_target_entry (const file_t *p_target, 
                           info_target_output_mask_t output_mask) 
{
    const floc_t *p_floc = &p_target->floc;
    if (p_floc) {
      if ((p_floc->filenm) && (output_mask & INFO_TARGET_POSITION)) {
        printf("%s:%lu", p_floc->filenm, p_floc->lineno);
        if (output_mask & INFO_TARGET_NAME)
          printf(":\n");
        else
          printf("\n");
      }
      if (output_mask & INFO_TARGET_NAME) {
        printf("\t%s\n", p_target->name);
      } else if (output_mask & INFO_TARGET_TASKS \
                 && (p_target->cmds || p_target->phony) \
                 && p_floc->filenm) {
          printf("%s\n", p_target->name);
      }
    }
}

int
dbg_target_compare(const void *p1, const void *p2) 
{
    const struct file *p_target1 = *(const file_t **) p1;
    const struct file *p_target2 = *(const file_t **) p2;
    return strcmp(p_target1->name, p_target2->name);
}

void
dbg_cmd_info_targets(info_target_output_mask_t output_mask)
{
  struct file **file_slot_0 = (struct file **) hash_dump (&files, 0, 
                                                          dbg_target_compare);
  struct file **file_end = file_slot_0 + files.ht_fill;
  struct file **pp_file_slot;
  struct file *p_target;
  
  for (pp_file_slot = file_slot_0; pp_file_slot < file_end; pp_file_slot++) {
    if ((p_target = *pp_file_slot) != NULL)
      dbg_cmd_info_target_entry(p_target, output_mask);
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
      printf(_("Program is stopped after running rule command(s).\n"));
      break;
    case DEBUG_BRKPT_BEFORE_PREREQ:
      printf(_("Program stopped before rule-prequisite checking.\n"));
	  break;
    case DEBUG_BRKPT_AFTER_PREREQ:
      printf(_("Program is stopped after rule-prequisite checking.\n"));
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
      printf(_("Program stopped in stepping before running rule command(s).\n"));
      printf("\n");
	  break;
    case DEBUG_EXPLICIT_CALL:
      printf(_("Program stopped from explicit debugger function call.\n"));
      printf("\n");
      break;
    case DEBUG_NOT_GIVEN:
      printf(_("Reason not given.\n"));
      break;
    case DEBUG_STACK_CHANGING:
      /* Should not happen? */
      break;
    }
}

  
/* Give some info regarding the running program. */
debug_return_t 
dbg_cmd_info(char *psz_args)
{
  if (!psz_args || 0==strlen(psz_args)) {
    unsigned int i;
    for (i = 0; info_subcommands[i].name; i++) {
      dbg_help_subcmd_entry("info", "%-10s -- %s", 
                            &(info_subcommands[i]), false);
    }
    return debug_readloop;
  } else {
    char *psz_subcmd = get_word(&psz_args);
    if (0 == strcmp(psz_subcmd, "lines")) {
      file2lines_dump();
    } else if (is_abbrev_of (psz_subcmd, "line", 2)) {
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
	dbg_errmsg("No target information for %s.", psz_target);
	return debug_cmd_error;
      }
      hash_map_arg (&p_target->variables->set->table, 
		    print_variable_info, NULL);
    } else if (is_abbrev_of (psz_subcmd, "breakpoints", 1)) {
      list_breakpoints();
    } else if (is_abbrev_of (psz_subcmd, "makefiles", 1) ||
	       is_abbrev_of (psz_subcmd, "files", 2)) {
      if (0 == strlen(psz_args))
	print_read_makefiles(NULL);
      else {
        if (!print_read_makefiles(psz_args))
          dbg_errmsg("File %s not in list of read-in files.", psz_args);
      }
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
	    dbg_errmsg(_("Rule %s not found.\n"), psz_args);
	}
    } else if (is_abbrev_of (psz_subcmd, "stack", 1)) {
        print_target_stack(p_stack_top, i_stack_pos, MAX_STACK_SHOW);
    } else if (0 == strcmp(psz_subcmd, "targets")) {
      /* Note: "targets" has to come before "target" */
      info_target_output_mask_t output_type;
      if (0 == strlen(psz_args))
        output_type = INFO_TARGET_POSITION_AND_NAME;
      else if (is_abbrev_of (psz_args, "all", 1))
        output_type = INFO_TARGET_POSITION_AND_NAME;
      else if (is_abbrev_of (psz_args, "positions", 1))
        output_type = INFO_TARGET_POSITION;
      else if (is_abbrev_of (psz_args, "names", 1))
        output_type = INFO_TARGET_NAME;
      else if (is_abbrev_of (psz_args, "tasks", 1))
        output_type = INFO_TARGET_TASKS;
      else {
        printf("Expecting 'all', 'positions', 'names', 'tasks', or nothing; got %s.\n",
               psz_args);
        return debug_cmd_error;
      }
      dbg_cmd_info_targets(output_type);

    } else if (is_abbrev_of (psz_subcmd, "target", 1)) {
      if (0 == strlen(psz_args)) {
        if (p_stack_top && p_stack_top->p_target && 
            p_stack_top->p_target->name)
          printf("target: %s\n", p_stack_top->p_target->name);
        else 
          {
            printf("target unknown\n");
          }
      } 
      else
        dbg_cmd_target(psz_args);
      
    } else if (is_abbrev_of (psz_subcmd, "variables", 1)) {
      print_variable_data_base();
    } else if (is_abbrev_of (psz_subcmd, "vpath", 1)) {
      print_vpath_data_base ();
    } else if (is_abbrev_of (psz_subcmd, "warranty", 1)) {
      printf("%s", WARRANTY);
    } else {
      dbg_errmsg(_("Undefined command \"%s\". Try \"help info\"."), 
                 psz_subcmd);
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
    _("Show program information regarding SUBCOMMAND.\n"
      "If SUBCOMMAND is not specified, give list of \"info\" subcommands.");
}

/* 
 * Local variables:
 *  c-file-style: "gnu"
 *  indent-tabs-mode: nil
 * End:
 */
