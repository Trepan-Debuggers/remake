/* Copyright (C) 1988, 1989, 1991 Free Software Foundation, Inc.
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
the Free Software Foundation, 675 Mass Ave, Cambridge, MA 02139, USA.  */

/* Structure used for pattern rules.  */

struct rule
  {
    struct rule *next;
    char **targets;		/* Targets of the rule.  */
    unsigned int *lens;		/* Lengths of each target.  */
    char **suffixes;		/* Suffixes (after `%') of each target.  */
    struct dep *deps;		/* Dependencies of the rule.  */
    struct commands *cmds;	/* Commands to execute.  */
    char terminal;		/* If terminal (double-colon).  */
    char subdir;		/* If references nonexistent subdirectory.  */
    char in_use;		/* If in use by a parent pattern_search.  */
  };

/* For calling install_pattern_rule.  */
struct pspec
  {
    char *target, *dep, *commands;
  };


extern struct rule *pattern_rules;
extern struct rule *last_pattern_rule;
extern unsigned int num_pattern_rules;

extern unsigned int max_pattern_deps;
extern unsigned int max_pattern_dep_length;

extern struct file *suffix_file;
extern unsigned int maxsuffix;


extern void install_pattern_rule ();
