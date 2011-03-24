/* Show a variable or target definition. */
/* 
Copyright (C) 2011 R. Bernstein <rocky@gnu.org>
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
debug_return_t 
dbg_cmd_target(char *psz_args) 
{
  const char *psz_target;
  file_t *p_target = get_target(&psz_args, &psz_target);

  if (p_target) {
    print_target_mask_t i_mask = 0;
    char *psz_word;
    
    while( (psz_word = get_word(&psz_args))) {
      if (!*psz_word) {
	break;
      } else if (is_abbrev_of(psz_word, "depends", 1)) {
	i_mask |= PRINT_TARGET_DEPEND;
      } else if (is_abbrev_of(psz_word, "nonorder", 1)) {
	i_mask |= PRINT_TARGET_NONORDER;
      } else if (is_abbrev_of(psz_word, "attributes", 1)) {
	i_mask |= PRINT_TARGET_ATTRS;
      } else if (is_abbrev_of(psz_word, "state", 1)) {
	i_mask |= PRINT_TARGET_STATE;
      } else if (is_abbrev_of(psz_word, "time", 1)) {
	i_mask |= PRINT_TARGET_TIME;
      } else if (is_abbrev_of(psz_word, "variables", 1)) {
	i_mask |= PRINT_TARGET_VARS;
      } else if (is_abbrev_of(psz_word, "commands", 1)) {
	i_mask |= PRINT_TARGET_CMDS;
      } else if (is_abbrev_of(psz_word, "expand", 1)) {
	i_mask |= (PRINT_TARGET_CMDS|PRINT_TARGET_CMDS_EXP);
      } else if (is_abbrev_of(psz_word, "previous", 1)) {
	i_mask |= PRINT_TARGET_PREV;
      } else {
	printf("Don't understand attribute '%s'\n", psz_word);
	return debug_readloop;
      }
    }
    
    if (0 == i_mask) i_mask = PRINT_TARGET_ALL & (~PRINT_TARGET_VARS_HASH);

    if (i_mask & PRINT_TARGET_VARS) {
      initialize_file_variables (p_target, 0);
      set_file_variables (p_target);
    }

    print_target_props(p_target, i_mask);
  }
  return debug_readloop;
}
