/* $Id: dbg_fns.h,v 1.11 2006/03/19 12:17:44 rockyb Exp $
Copyright (C) 2005, 2008 R. Bernstein <rocky@gnu.org>
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

/** \file dbg_fns.h 
 *
 *  \brief debugger helper functions.
*/

#ifndef DBG_FNS_H
#define DBG_FNS_H

#include "debug.h"
#include "filedef.h"
#include "trace.h"
#include "rule.h"

extern floc_t *p_target_loc;
extern char   *psz_target_name;

/*! We use the if when we fake a line number because
   a real one hasn't been recorded on the stack. */
extern floc_t  fake_floc;

/*! Parse psz_arg for a signed integer. The value is returned in
    *pi_result. If warn is true, then we'll give a warning if no
    integer found. The return value is true if parsing succeeded in
    any event..
 */
extern bool get_int(const char *psz_arg, /*out*/ int *pi_result,
		    bool b_warn);

/*! Parse psz_arg for a unsigned integer. The value is returned in 
    *pi_result. The retun value is true if parsing succeeded.
 */
extern bool get_uint(const char *psz_arg, /*out*/ unsigned int *pi_result);

/*! Find the next "word" - skip leading blanks and the "word" is the
   largest non-blank characters after that. ppsz_str is modified to
   point after the portion returned and also the region initially
   pointed to by ppsz_str is modified so that word is zero-byte
   termintated.
 */
extern char *get_word(char **ppsz_str);

/*! Find the target in first word of psz_args or use $@ (the current
    stack) if none.  We also allow $@ or @ explicitly as a target name
    to mean the current target on the stack. NULL is returned if a lookup 
    of the target name was not found. ppsz_target is to the name
    looked up.
 */
file_t *get_target(/*in/out*/ char **ppsz_args, 
		   /*out*/ const char **ppsz_target);

/*! Return true if psz_substr is an initial prefix (abbreviation) of
    psz_word. The empty string is not a valid abbreviation. */
extern bool is_abbrev_of(const char *psz_substr, 
			 const char *psz_word, unsigned int i_min);
/*! toggle var on or on or off depending on psz_onoff */    
extern void on_off_toggle(const char *psz_onoff, int *var) ;

/** Print where we are in the Makefile. */
extern void print_debugger_location(const file_t *p_target, 
				    debug_enter_reason_t reason,
				    const floc_stack_node_t *p_stack_floc);
    
/** Strip whitespace from the start and end of STRING.  Return a pointer
   into STRING. */
extern char *stripwhite (char *string);

/*! Show if i_bool is "on" or "off" */
extern char *var_to_on_off(int i_bool);

/*! Show a expression. Set "expand" to 1 if you want variable
   definitions inside the displayed value expanded.
*/
extern bool dbg_cmd_show_exp(char *psz_arg, bool expand);

/*! Print an interpretation of the debug level mask. */
extern void print_db_level(debug_level_mask_t e_debug_level);

/*! See if psz_varname is $varname or $(varname) */
extern void try_without_dollar(const char *psz_varname);

extern void dbg_print_invocation(void);

extern rule_t *find_rule (const char *psz_name);

#endif /* DBG_FNS_H*/
