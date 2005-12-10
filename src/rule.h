/* $Id: rule.h,v 1.5 2005/12/10 02:50:32 rockyb Exp $
Copyright (C) 1988, 1989, 1991, 1992, 1993, 2005 Free Software Foundation, Inc.
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

/** \file rule.h
 *
 *  \brief Definitions for using pattern rules in GNU Make.
 */

#include "types.h"

/** \brief Structure used for pattern rules.  */
typedef struct rule
  {
    struct rule *next;
    char **targets;		/**< Targets of the rule.  */
    unsigned int *lens;		/**< Lengths of each target.  */
    char **suffixes;		/**< Suffixes (after `%') of each target.  */
    dep_t *deps;		/**< Dependencies of the rule.  */
    commands_t *cmds;	        /**< Commands to execute.  */
    char terminal;		/**< If terminal (double-colon).  */
    char in_use;		/**< If in use by a parent pattern_search.  */
  } rule_t;

/*! For calling install_pattern_rule.  */
struct pspec {
  char *target, *dep, *commands;
};


/*! Chain of all pattern rules.  */
extern rule_t *pattern_rules;

/*! Pointer to last rule in the chain, so we can add onto the end.  */
extern rule_t *last_pattern_rule;

/*! Number of rules in the chain.  */
extern unsigned int num_pattern_rules;

/*! Maximum number of dependencies of any pattern rule.  */
extern unsigned int max_pattern_deps;

extern unsigned int max_pattern_targets;

/*! Maximum length of the name of a dependencies of any pattern rule.  */
extern unsigned int max_pattern_dep_length;

/*! Pointer to structure for the file .SUFFIXES
   whose dependencies are the suffixes to be searched.  */
extern file_t *suffix_file;

/*! Maximum length of a suffix.  */
extern unsigned int maxsuffix;


/*! Install an implicit pattern rule based on the three text strings
   in the structure P points to.  These strings come from one of
   the arrays of default implicit pattern rules.
   TERMINAL specifies what the `terminal' field of the rule should be.  */
void install_pattern_rule (pspec_t *p, int terminal);

/*! Install the pattern rule P_RULE (whose fields have been filled in)
   at the end of the list (so that any rules previously defined
   will take precedence).  If this rule duplicates a previous one
   (identical target and dependencies), the old one is replaced
   if OVERRIDE is nonzero, otherwise this new one is thrown out.
   When an old rule is replaced, the new one is put at the end of the
   list.  Return nonzero if P_RULE is used; zero if not. 
 */
int new_pattern_rule (rule_t *p_rule, int override);

/*! Compute the maximum dependency length and maximum number of
   dependencies of all implicit rules.  Also sets the subdir
   flag for a rule when appropriate, possibly removing the rule
   completely when appropriate.
*/
void count_implicit_rule_limits (void);

/*!
 Convert old-style suffix rules to pattern rules.
 All rules for the suffixes on the .SUFFIXES list
 are converted and added to the chain of pattern rules.  
*/
void convert_to_pattern (void);

/*! Create a new pattern rule with the targets in the nil-terminated
   array TARGETS.  If TARGET_PERCENTS is not nil, it is an array of
   pointers into the elements of TARGETS, where the `%'s are.
   The new rule has dependencies DEPS and commands from COMMANDS.
   It is a terminal rule if TERMINAL is nonzero.  This rule overrides
   identical rules with different commands if OVERRIDE is nonzero.

   The storage for TARGETS and its elements is used and must not be freed
   until the rule is destroyed.  The storage for TARGET_PERCENTS is not used;
   it may be freed.  
*/
void create_pattern_rule (char **targets, char **target_percents, int terminal,
			  dep_t *deps, commands_t *commands, 
			  int override);


/*! Free all pattern rules */
void free_pattern_rules (void);


