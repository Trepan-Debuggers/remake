/* Pattern and suffix rule internals for GNU Make.
Copyright (C) 1988, 1989, 1990, 1991, 1992, 1993 Free Software Foundation, Inc.
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

#include "make.h"
#include "commands.h"
#include "dep.h"
#include "file.h"
#include "variable.h"
#include "rule.h"

static void freerule ();

/* Chain of all pattern rules.  */

struct rule *pattern_rules;

/* Pointer to last rule in the chain, so we can add onto the end.  */

struct rule *last_pattern_rule;

/* Number of rules in the chain.  */

unsigned int num_pattern_rules;

/* Maximum number of dependencies of any pattern rule.  */

unsigned int max_pattern_deps;

/* Maximum length of the name of a dependencies of any pattern rule.  */

unsigned int max_pattern_dep_length;

/* Pointer to structure for the file .SUFFIXES
   whose dependencies are the suffixes to be searched.  */

struct file *suffix_file;

/* Maximum length of a suffix.  */

unsigned int maxsuffix;

/* Compute the maximum dependency length and maximum number of
   dependencies of all implicit rules.  Also sets the subdir
   flag for a rule when appropriate, possibly removing the rule
   completely when appropriate.  */

void
count_implicit_rule_limits ()
{
  char *name;
  unsigned int namelen;
  register struct rule *rule, *lastrule;

  num_pattern_rules = 0;
  
  name = 0;
  namelen = 0;
  rule = pattern_rules;
  lastrule = 0;
  while (rule != 0)
    {
      unsigned int ndeps = 0;
      register struct dep *dep;
      struct rule *next = rule->next;
    
      ++num_pattern_rules;
      
      for (dep = rule->deps; dep != 0; dep = dep->next)
	{
	  unsigned int len = strlen (dep->name);
	  char *p = rindex (dep->name, '/');
	  char *p2 = p != 0 ? index (dep->name, '%') : 0;

	  ndeps++;

	  if (len > max_pattern_dep_length)
	    max_pattern_dep_length = len;

	  if (p != 0 && p2 > p)
	    {
	      /* There is a slash before the % in the dep name.
		 Extract the directory name.  */
	      if (p == dep->name)
		++p;
	      if (p - dep->name > namelen)
		{
		  if (name != 0)
		    free (name);
		  namelen = p - dep->name;
		  name = (char *) xmalloc (namelen + 1);
		}
	      bcopy (dep->name, name, p - dep->name);
	      name[p - dep->name] = '\0';

	      /* In the deps of an implicit rule the `changed' flag
		 actually indicates that the dependency is in a
		 nonexistent subdirectory.  */

	      dep->changed = !dir_file_exists_p (name, "");
	      if (dep->changed && *name == '/')
		{
		  /* The name is absolute and the directory does not exist.
		     This rule can never possibly match, since this dependency
		     can never possibly exist.  So just remove the rule from
		     the list.  */
		  freerule (rule, lastrule);
		  --num_pattern_rules;
		  goto end_main_loop;
		}
	    }
	  else
	    /* This dependency does not reside in a subdirectory.  */
	    dep->changed = 0;
	}

      if (ndeps > max_pattern_deps)
	max_pattern_deps = ndeps;

      lastrule = rule;
    end_main_loop:
      rule = next;
    }
  
  if (name != 0)
    free (name);
}

/* Convert old-style suffix rules to pattern rules.
   All rules for the suffixes on the .SUFFIXES list
   are converted and added to the chain of pattern rules.  */

void
convert_to_pattern ()
{
  register struct dep *d, *d2, *newd;
  register struct file *f;
  register char *rulename;
  register unsigned int slen, s2len;
  register char *name, **names;

  /* Compute maximum length of all the suffixes.  */

  maxsuffix = 0;
  for (d = suffix_file->deps; d != 0; d = d->next)
    {
      register unsigned int namelen = strlen (dep_name (d));
      if (namelen > maxsuffix)
	maxsuffix = namelen;
    }

  rulename = (char *) alloca ((maxsuffix * 2) + 1);

  for (d = suffix_file->deps; d != 0; d = d->next)
    {
      /* Make a rule that is just the suffix, with no deps or commands.
	 This rule exists solely to disqualify match-anything rules.  */
      slen = strlen (dep_name (d));
      name = (char *) xmalloc (1 + slen + 1);
      name[0] = '%';
      bcopy (dep_name (d), name + 1, slen + 1);
      names = (char **) xmalloc (2 * sizeof (char *));
      names[0] = name;
      names[1] = 0;
      create_pattern_rule (names, (char **) 0, 0, (struct dep *) 0,
			   (struct commands *) 0, 0);

      f = d->file;
      if (f->cmds != 0)
	{
	  /* Record a pattern for this suffix's null-suffix rule.  */
	  newd = (struct dep *) xmalloc (sizeof (struct dep));
	  /* Construct this again rather than using the contents
	     of NAME (above), since that may have been freed by
	     create_pattern_rule.  */
	  newd->name = (char *) xmalloc (1 + slen + 1);
	  newd->name[0] = '%';
	  bcopy (dep_name (d), newd->name + 1, slen + 1);
	  newd->next = 0;
	  names = (char **) xmalloc (2 * sizeof (char *));
	  names[0] = savestring ("%", 1);
	  names[1] = 0;
	  create_pattern_rule (names, (char **) 0, 0, newd, f->cmds, 0);
	}

      /* Record a pattern for each of this suffix's two-suffix rules.  */
      bcopy (dep_name (d), rulename, slen);
      for (d2 = suffix_file->deps; d2 != 0; d2 = d2->next)
	{
	  s2len = strlen (dep_name (d2));

	  if (slen == s2len && streq (dep_name (d), dep_name (d2)))
	    continue;

	  bcopy (dep_name (d2), rulename + slen, s2len + 1);
	  f = lookup_file (rulename);
	  if (f == 0 || f->cmds == 0)
	    continue;

	  if (s2len == 2 && rulename[slen] == '.' && rulename[slen + 1] == 'a')
	    /* The suffix rule `.X.a:' is converted
	       to the pattern rule `(%.o): %.X'.  */
	    name = savestring ("(%.o)", 5);
	  else
	    {
	      /* The suffix rule `.X.Y:' is converted
		 to the pattern rule `%.Y: %.X'.  */
	      name = (char *) xmalloc (1 + s2len + 1);
	      name[0] = '%';
	      bcopy (dep_name (d2), name + 1, s2len + 1);
	    }
	  names = (char **) xmalloc (2 * sizeof (char *));
	  names[0] = name;
	  names[1] = 0;
	  newd = (struct dep *) xmalloc (sizeof (struct dep));
	  newd->next = 0;
	  /* Construct this again (see comment above).  */
	  newd->name = (char *) xmalloc (1 + slen + 1);
	  newd->name[0] = '%';
	  bcopy (dep_name (d), newd->name + 1, slen + 1);
	  create_pattern_rule (names, (char **) 0, 0, newd, f->cmds, 0);
	}
    }
}


/* Install the pattern rule RULE (whose fields have been filled in)
   at the end of the list (so that any rules previously defined
   will take precedence).  If this rule duplicates a previous one
   (identical target and dependencies), the old one is replaced
   if OVERRIDE is nonzero, otherwise this new one is thrown out.
   When an old rule is replaced, the new one is put at the end of the
   list.  Return nonzero if RULE is used; zero if not.  */

int
new_pattern_rule (rule, override)
     register struct rule *rule;
     int override;
{
  register struct rule *r, *lastrule;
  register unsigned int i, j;

  rule->in_use = 0;
  rule->terminal = 0;

  rule->next = 0;

  /* Search for an identical rule.  */
  lastrule = 0;
  for (r = pattern_rules; r != 0; lastrule = r, r = r->next)
    for (i = 0; rule->targets[i] != 0; ++i)
      {
	for (j = 0; r->targets[j] != 0; ++j)
	  if (!streq (rule->targets[i], r->targets[j]))
	    break;
	if (r->targets[j] == 0)
	  /* All the targets matched.  */
	  {
	    register struct dep *d, *d2;
	    for (d = rule->deps, d2 = r->deps;
		 d != 0 && d2 != 0; d = d->next, d2 = d2->next)
	      if (!streq (dep_name (d), dep_name (d2)))
		break;
	    if (d == 0 && d2 == 0)
	      /* All the dependencies matched.  */
	      if (override)
		{
		  /* Remove the old rule.  */
		  freerule (r, lastrule);
		  /* Install the new one.  */
		  if (pattern_rules == 0)
		    pattern_rules = rule;
		  else
		    last_pattern_rule->next = rule;
		  last_pattern_rule = rule;
		  
		  /* We got one.  Stop looking.  */
		  goto matched;
		}
	      else
		{
		  /* The old rule stays intact.  Destroy the new one.  */
		  freerule (rule, (struct rule *) 0);
		  return 0;
		}
	  }
      }

 matched:;

  if (r == 0)
    {
      /* There was no rule to replace.  */
      if (pattern_rules == 0)
	pattern_rules = rule;
      else
	last_pattern_rule->next = rule;
      last_pattern_rule = rule;
    }

  return 1;
}


/* Install an implicit pattern rule based on the three text strings
   in the structure P points to.  These strings come from one of
   the arrays of default implicit pattern rules.
   TERMINAL specifies what the `terminal' field of the rule should be.  */

void
install_pattern_rule (p, terminal)
     struct pspec *p;
     int terminal;
{
  register struct rule *r;
  char *ptr;

  r = (struct rule *) xmalloc (sizeof (struct rule));

  r->targets = (char **) xmalloc (2 * sizeof (char *));
  r->suffixes = (char **) xmalloc (2 * sizeof (char *));
  r->lens = (unsigned int *) xmalloc (2 * sizeof (unsigned int));

  r->targets[1] = 0;
  r->suffixes[1] = 0;
  r->lens[1] = 0;

  r->lens[0] = strlen (p->target);
  /* These will all be string literals, but we malloc space for
     them anyway because somebody might want to free them later on.  */
  r->targets[0] = savestring (p->target, r->lens[0]);
  r->suffixes[0] = find_percent (r->targets[0]);
  if (r->suffixes[0] == 0)
    /* Programmer-out-to-lunch error.  */
    abort ();
  else
    ++r->suffixes[0];

  ptr = p->dep;
  r->deps = (struct dep *) multi_glob (parse_file_seq (&ptr, '\0',
                                                       sizeof (struct dep)),
				       sizeof (struct dep),
				       1);

  if (new_pattern_rule (r, 0))
    {
      r->terminal = terminal;
      r->cmds = (struct commands *) xmalloc (sizeof (struct commands));
      r->cmds->filename = 0;
      r->cmds->lineno = 0;
      /* These will all be string literals, but we malloc space for them
	 anyway because somebody might want to free them later.  */
      r->cmds->commands = savestring (p->commands, strlen (p->commands));
      r->cmds->command_lines = 0;
    }
}


/* Free all the storage used in RULE and take it out of the
   pattern_rules chain.  LASTRULE is the rule whose next pointer
   points to RULE.  */

static void
freerule (rule, lastrule)
     register struct rule *rule, *lastrule;
{
  struct rule *next = rule->next;
  register unsigned int i;

  for (i = 0; rule->targets[i] != 0; ++i)
    free (rule->targets[i]);

  free ((char *) rule->targets);
  free ((char *) rule->suffixes);
  free ((char *) rule->lens);

  /* We can't free the storage for the commands because there
     are ways that they could be in more than one place:
       * If the commands came from a suffix rule, they could also be in
       the `struct file's for other suffix rules or plain targets given
       on the same makefile line.
       * If two suffixes that together make a two-suffix rule were each
       given twice in the .SUFFIXES list, and in the proper order, two
       identical pattern rules would be created and the second one would
       be discarded here, but both would contain the same `struct commands'
       pointer from the `struct file' for the suffix rule.  */

  free ((char *) rule);

  if (pattern_rules == rule)
    if (lastrule != 0)
      abort ();
    else
      pattern_rules = next;
  else if (lastrule != 0)
    lastrule->next = next;
  if (last_pattern_rule == rule)
    last_pattern_rule = lastrule;
}

/* Create a new pattern rule with the targets in the nil-terminated
   array TARGETS.  If TARGET_PERCENTS is not nil, it is an array of
   pointers into the elements of TARGETS, where the `%'s are.
   The new rule has dependencies DEPS and commands from COMMANDS.
   It is a terminal rule if TERMINAL is nonzero.  This rule overrides
   identical rules with different commands if OVERRIDE is nonzero.

   The storage for TARGETS and its elements is used and must not be freed
   until the rule is destroyed.  The storage for TARGET_PERCENTS is not used;
   it may be freed.  */

void
create_pattern_rule (targets, target_percents,
		     terminal, deps, commands, override)
     char **targets, **target_percents;
     int terminal;
     struct dep *deps;
     struct commands *commands;
     int override;
{
  register struct rule *r = (struct rule *) xmalloc (sizeof (struct rule));
  register unsigned int max_targets, i;

  r->cmds = commands;
  r->deps = deps;
  r->targets = targets;

  max_targets = 2;
  r->lens = (unsigned int *) xmalloc (2 * sizeof (unsigned int));
  r->suffixes = (char **) xmalloc (2 * sizeof (char *));
  for (i = 0; targets[i] != 0; ++i)
    {
      if (i == max_targets - 1)
	{
	  max_targets += 5;
	  r->lens = (unsigned int *)
	    xrealloc ((char *) r->lens, max_targets * sizeof (unsigned int));
	  r->suffixes = (char **)
	    xrealloc ((char *) r->suffixes, max_targets * sizeof (char *));
	}
      r->lens[i] = strlen (targets[i]);
      r->suffixes[i] = (target_percents == 0 ? find_percent (targets[i])
			: target_percents[i]) + 1;
      if (r->suffixes[i] == 0)
	abort ();
    }

  if (i < max_targets - 1)
    {
      r->lens = (unsigned int *) xrealloc ((char *) r->lens,
					   (i + 1) * sizeof (unsigned int));
      r->suffixes = (char **) xrealloc ((char *) r->suffixes,
					(i + 1) * sizeof (char *));
    }

  if (new_pattern_rule (r, override))
    r->terminal = terminal;
}

/* Print the data base of rules.  */

void
print_rule_data_base ()
{
  register unsigned int rules, terminal;
  register struct rule *r;
  register struct dep *d;
  register unsigned int i;

  puts ("\n# Implicit Rules");

  rules = terminal = 0;
  for (r = pattern_rules; r != 0; r = r->next)
    {
      ++rules;

      putchar ('\n');
      for (i = 0; r->targets[i] != 0; ++i)
	{
	  fputs (r->targets[i], stdout);
	  if (r->targets[i + 1] != 0)
	    putchar (' ');
	  else
	    putchar (':');
	}
      if (r->terminal)
	{
	  ++terminal;
	  putchar (':');
	}

      for (d = r->deps; d != 0; d = d->next)
	printf (" %s", dep_name (d));
      putchar ('\n');

      if (r->cmds != 0)
	print_commands (r->cmds);
    }

  if (rules == 0)
    puts ("\n# No implicit rules.");
  else
    {
      printf ("\n# %u implicit rules, %u", rules, terminal);
#ifndef	NO_FLOAT
      printf (" (%.1f%%)", (double) terminal / (double) rules * 100.0);
#endif
      puts (" terminal.");
    }

  if (num_pattern_rules != rules)
    fatal ("BUG: num_pattern_rules wrong!  %u != %u",
	   num_pattern_rules, rules);
}
