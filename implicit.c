/* Implicit rule searching for GNU Make.
Copyright (C) 1988, 1989, 1990, 1991 Free Software Foundation, Inc.
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
#include "rule.h"
#include "dep.h"
#include "file.h"

static int pattern_search ();

/* For a FILE which has no commands specified, try to figure out some
   from the implicit pattern rules.
   Returns 1 if a suitable implicit rule was found,
   after modifying FILE to contain the appropriate commands and deps,
   or returns 0 if no implicit rule was found.  */

int
try_implicit_rule (file, depth)
     struct file *file;
     unsigned int depth;
{
  DEBUGPR ("Looking for an implicit rule for `%s'.\n");

#ifndef	NO_ARCHIVES
  /* If this is an archive member reference, use just the
     archive member name to search for implicit rules.  */
  if (ar_name (file->name))
    {
      DEBUGPR ("Looking for archive-member implicit rule for `%s'.\n");
      if (pattern_search (file, 1, depth, 0))
	return 1;
    }
#endif

  return pattern_search (file, 0, depth, 0);
}

#define DEBUGP2(msg, a1, a2) \
  if (debug_flag) \
    { print_spaces (depth); printf (msg, a1, a2); fflush (stdout);  } else

/* Search the pattern rules for a rule with an existing dependency to make
   FILE.  If a rule is found, the appropriate commands and deps are put in FILE
   and 1 is returned.  If not, 0 is returned.

   If ARCHIVE is nonzero, FILE->name is of the form "LIB(MEMBER)".  A rule for
   "(MEMBER)" will be searched for, and "(MEMBER)" will not be chopped up into
   directory and filename parts.

   If an intermediate file is found by pattern search, the intermediate file
   is set up as a target by the recursive call and is also made a dependency
   of FILE.

   DEPTH is used for debugging messages.  */

static int
pattern_search (file, archive, depth, recursions)
     struct file *file;
     int archive;
     unsigned int depth;
     unsigned int recursions;
{
  /* Filename we are searching for a rule for.  */
  char *filename = archive ? index (file->name, '(') : file->name;

  /* Length of FILENAME.  */
  unsigned int namelen = strlen (filename);

  /* The last slash in FILENAME (or nil if there is none).  */
  char *lastslash;

  /* This is a file-object used as an argument in
     recursive calls.  It never contains any data
     except during a recursive call.  */
  struct file *intermediate_file = 0;

  /* List of dependencies found recursively.  */
  struct file **intermediate_files
    = (struct file **) alloca (max_pattern_deps * sizeof (struct file *));

  /* List of the patterns used to find intermediate files.  */
  char **intermediate_patterns
    = (char **) alloca (max_pattern_deps * sizeof (char *));

  /* This buffer records all the dependencies actually found for a rule.  */
  char **found_files = (char **) alloca (max_pattern_deps * sizeof (char *));
  /* Number of dep names now in FOUND_FILES.  */
  unsigned int deps_found;

  /* Names of possible dependencies are constructed in this buffer.  */
  register char *depname = (char *) alloca (namelen + max_pattern_dep_length);

  /* The start and length of the stem of FILENAME for the current rule.  */
  register char *stem;
  register unsigned int stemlen;

  /* Buffer in which we store all the rules that are possibly applicable.  */
  struct rule **tryrules
    = (struct rule **) alloca (num_pattern_rules * sizeof (struct rule *));

  /* Number of valid elements in TRYRULES.  */
  unsigned int nrules;

  /* The numbers of the rule targets of each rule
     in TRYRULES that matched the target file.  */
  unsigned int *matches
    = (unsigned int *) alloca (num_pattern_rules * sizeof (unsigned int));

  /* Each element is nonzero if LASTSLASH was used in
     matching the corresponding element of TRYRULES.  */
  char *checked_lastslash
    = (char *) alloca (num_pattern_rules * sizeof (char));

  /* The index in TRYRULES of the rule we found.  */
  unsigned int foundrule;

  /* Nonzero if should consider intermediate files as dependencies.  */
  int intermed_ok;

  /* Nonzero if we have matched a pattern-rule target
     that is not just `%'.  */
  int specific_rule_matched = 0;

  register unsigned int i;
  register struct rule *rule;
  register struct dep *dep;

  char *p;

#ifndef	NO_ARCHIVES
  if (archive || ar_name (filename))
    lastslash = 0;
  else
#endif
    {
      /* Set LASTSLASH to point at the last slash in FILENAME
	 but not counting any slash at the end.  (foo/bar/ counts as
	 bar/ in directory foo/, not empty in directory foo/bar/.)  */
      lastslash = rindex (filename, '/');
      if (lastslash != 0 && lastslash[1] == '\0')
	lastslash = 0;
    }

  /* First see which pattern rules match this target
     and may be considered.  Put them in TRYRULES.  */

  nrules = 0;
  for (rule = pattern_rules; rule != 0; rule = rule->next)
    {
      int specific_rule_may_have_matched = 0;
      int check_lastslash;

      /* If the pattern rule has deps but no commands, ignore it.
	 Users cancel built-in rules by redefining them without commands.  */
      if (rule->deps != 0 && rule->cmds == 0)
	continue;

      /* If this rule is in use by a parent pattern_search,
	 don't use it here.  */
      if (rule->in_use)
	{
	  DEBUGP2 ("Avoiding implicit rule recursion.\n", 0, 0);
	  continue;
	}

      for (i = 0; rule->targets[i] != 0; ++i)
	{
	  char *target = rule->targets[i];
	  char *suffix = rule->suffixes[i];

	  /* Rules that can match any filename and are not terminal
	     are ignored if we're recursing, so that they cannot be
	     intermediate files.  */
	  if (recursions > 0 && target[1] == '\0' && !rule->terminal)
	    continue;

	  if (rule->lens[i] > namelen)
	    /* It can't possibly match.  */
	    continue;

	  /* From the lengths of the filename and the pattern parts,
	     find the stem: the part of the filename that matches the %.  */
	  stem = filename + (suffix - target - 1);
	  stemlen = namelen - rule->lens[i] + 1;

	  /* Set CHECK_LASTSLASH if FILENAME contains a directory
	     prefix and the target pattern does not contain a slash.  */

	  check_lastslash = lastslash != 0 && index (target, '/') == 0;
	  if (check_lastslash)
	    {
	      /* In that case, don't include the
		 directory prefix in STEM here.  */
	      unsigned int difference = lastslash - filename + 1;
	      if (difference > stemlen)
		continue;
	      stemlen -= difference;
	      stem += difference;
	    }

	  /* Check that the rule pattern matches the text before the stem.  */
	  if (check_lastslash)
	    {
	      if (stem > (lastslash + 1)
		  && strncmp (target, lastslash + 1, stem - lastslash - 1))
		continue;
	    }
	  else if (stem > filename
		   && strncmp (target, filename, stem - filename))
	    continue;

	  /* Check that the rule pattern matches the text after the stem.
	     We could test simply use streq, but this way we compare the
	     first two characters immediately.  This saves time in the very
	     common case where the first character matches because it is a
	     period.  */
	  if (*suffix != stem[stemlen]
	      || (*suffix != '\0' && !streq (&suffix[1], &stem[stemlen + 1])))
	    continue;

	  /* Record if we match a rule that not all filenames will match.  */
	  if (target[1] != '\0')
	    specific_rule_may_have_matched = 1;

	  /* We have a matching target.  Don't search for any more.  */
	  break;
	}

      /* None of the targets matched.  */
      if (rule->targets[i] == 0)
	continue;

      specific_rule_matched |= specific_rule_may_have_matched;

      /* A rule with no dependencies and no commands exists solely to set
	 specific_rule_matched when it matches.  Don't try to use it.  */
      if (rule->deps == 0 && rule->cmds == 0)
	continue;

      /* Record this rule in TRYRULES and the index
	 of the (first) matching target in MATCHES.  */
      tryrules[nrules] = rule;
      matches[nrules] = i;
      checked_lastslash[nrules] = check_lastslash;
      ++nrules;
    }

  /* If we have found a matching rule that won't match all filenames,
     retroactively reject any "terminal" rules that do always match.  */
  if (specific_rule_matched)
    for (i = 0; i < nrules; ++i)
      if (!tryrules[i]->terminal)
	{
	  register unsigned int j;
	  for (j = 0; tryrules[i]->targets[j] != 0; ++j)
	    if (tryrules[i]->targets[j][1] == '\0')
	      break;
	  if (tryrules[i]->targets[j] != 0)
	    tryrules[i] = 0;
	}

  /* Try each rule once without intermediate files, then once with them.  */
  for (intermed_ok = 0; intermed_ok == !!intermed_ok; ++intermed_ok)
    {
      /* Try each pattern rule till we find one that applies.
	 If it does, copy the names of its dependencies (as substituted)
	 and store them in FOUND_FILES.  DEPS_FOUND is the number of them.  */

      for (i = 0; i < nrules; i++)
	{
	  int check_lastslash;

	  rule = tryrules[i];

	  /* RULE is nil when we discover that a rule,
	     already placed in TRYRULES, should not be applied.  */
	  if (rule == 0)
	    continue;

	  /* Reject any terminal rules if we're
	     looking to make intermediate files.  */
	  if (intermed_ok && rule->terminal)
	    continue;

	  /* Mark this rule as in use so a recursive
	     pattern_search won't try to use it.  */
	  rule->in_use = 1;

	  /* From the lengths of the filename and the matching pattern parts,
	     find the stem: the part of the filename that matches the %.  */
	  stem = filename
	    + (rule->suffixes[matches[i]] - rule->targets[matches[i]]) - 1;
	  stemlen = namelen - rule->lens[matches[i]] + 1;
	  check_lastslash = (lastslash != 0
			     && index (rule->targets[matches[i]], '/') == 0);
	  if (check_lastslash)
	    {
	      stem += lastslash - filename + 1;
	      stemlen -= (lastslash - filename) + 1;
	    }

	  DEBUGP2 ("Trying pattern rule with stem `%.*s'.\n",
		   stemlen, stem);

	  /* Try each dependency; see if it "exists".  */

	  deps_found = 0;
	  for (dep = rule->deps; dep != 0; dep = dep->next)
	    {
	      /* If the dependency name has a %, substitute the stem.  */
	      p = index (dep_name (dep), '%');
	      if (p != 0)
		{
		  register unsigned int i;
		  if (check_lastslash)
		    {
		      i = lastslash - filename + 1;
		      bcopy (filename, depname, i);
		    }
		  else
		    i = 0;
		  bcopy (dep_name (dep), depname + i, p - dep_name (dep));
		  i += p - dep_name (dep);
		  bcopy (stem, depname + i, stemlen);
		  i += stemlen;
		  strcpy (depname + i, p + 1);
		  p = depname;
		}
	      else
		p = dep_name (dep);

	      /* P is now the actual dependency name as substituted.  */

	      if (file_impossible_p (p))
		{
		  /* If this dependency has already been ruled
		     "impossible", then the rule fails and don't
		     bother trying it on the second pass either
		     since we know that will fail too.  */
		  DEBUGP2 ("Rejecting impossible %s dependent `%s'.\n",
			   p == depname ? "implicit" : "rule", p);
		  tryrules[i] = 0;
		  break;
		}

	      intermediate_files[deps_found] = 0;

	      DEBUGP2 ("Trying %s dependency `%s'.\n",
		       p == depname ? "implicit" : "rule", p);
	      if (!rule->subdir && lookup_file (p) != 0 || file_exists_p (p))
		{
		  found_files[deps_found++] = savestring (p, strlen (p));
		  continue;
		}
	      /* This code, given FILENAME = "lib/foo.o", dependency name
		 "lib/foo.c", and VPATH=src, searches for "src/lib/foo.c".  */
	      if (vpath_search (&p))
		{
		  DEBUGP2 ("Found dependent as `%s'.\n", p, 0);
		  found_files[deps_found++] = p;
		  continue;
		}

	      /* We could not find the file in any place we should look.
		 Try to make this dependency as an intermediate file,
		 but only on the second pass.  */

	      if (intermed_ok)
		{
		  if (intermediate_file == 0)
		    intermediate_file
		      = (struct file *) alloca (sizeof (struct file));

		  DEBUGP2 ("Looking for a rule with intermediate file `%s'.\n",
			   p, 0);

		  bzero ((char *) intermediate_file, sizeof (struct file));
		  intermediate_file->name = p;
		  if (pattern_search (intermediate_file, 0, depth + 1,
				      recursions + 1))
		    {
		      p = savestring (p, strlen (p));
		      intermediate_patterns[deps_found]
			= intermediate_file->name;
		      found_files[deps_found] = p;
		      intermediate_file->name = p;
		      intermediate_files[deps_found] = intermediate_file;
		      intermediate_file = 0;
		      ++deps_found;
		      continue;
		    }

		  /* If we have tried to find P as an intermediate
		     file and failed, mark that name as impossible
		     so we won't go through the search again later.  */
		  file_impossible (p);
		}

	      /* A dependency of this rule does not exist.
		 Therefore, this rule fails.  */
	      break;
	    }

	  /* This rule is no longer `in use' for recursive searches.  */
	  rule->in_use = 0;

	  if (dep != 0)
	    {
	      /* This pattern rule does not apply.
		 If some of its dependencies succeeded,
		 free the data structure describing them.  */
	      while (deps_found-- > 0)
		{
		  register struct file *f = intermediate_files[deps_found];
		  free (found_files[deps_found]);
		  if (f != 0
		      && (f->stem < f->name
			  || f->stem > f->name + strlen (f->name)))
		    free (f->stem);
		}
	    }
	  else
	    /* This pattern rule does apply.  Stop looking for one.  */
	    break;
	}

      /* If we found an applicable rule without
	 intermediate files, don't try with them.  */
      if (i < nrules)
	break;

      rule = 0;
    }

  /* RULE is nil if the loop went all the way
     through the list and everything failed.  */
  if (rule == 0)
    return 0;

  foundrule = i;

  /* If we are recursing, store the pattern that matched
     FILENAME in FILE->name for use in upper levels.  */

  if (recursions > 0)
    /* Kludge-o-matic */
    file->name = rule->targets[matches[foundrule]];

  /* FOUND_FILES lists the dependencies for the rule we found.
     This includes the intermediate files, if any.
     Convert them into entries on the deps-chain of FILE.  */

  while (deps_found-- > 0)
    {
      register char *s;

      if (intermediate_files[deps_found] != 0)
	{
	  /* If we need to use an intermediate file,
	     make sure it is entered as a target, with the info that was
	     found for it in the recursive pattern_search call.
	     We know that the intermediate file did not already exist as
	     a target; therefore we can assume that the deps and cmds
	     of F below are null before we change them.  */

	  struct file *imf = intermediate_files[deps_found];
	  register struct file *f = enter_file (imf->name);
	  f->deps = imf->deps;
	  f->cmds = imf->cmds;
	  f->stem = imf->stem;
	  imf = lookup_file (intermediate_patterns[deps_found]);
	  if (imf != 0 && imf->precious)
	    f->precious = 1;
	  f->intermediate = 1;
	  f->tried_implicit = 1;
	  for (dep = f->deps; dep != 0; dep = dep->next)
	    {
	      dep->file = enter_file (dep->name);
	      dep->name = 0;
	      dep->file->tried_implicit |= dep->changed;
	    }
	  num_intermediates++;
	}

      dep = (struct dep *) xmalloc (sizeof (struct dep));
      s = found_files[deps_found];
      if (recursions == 0)
	{
	  dep->name = 0;
	  dep->file = enter_file (s);	
	}
      else
	{
	  dep->name = s;
	  dep->file = 0;
	  dep->changed = 0;
	}
      if (intermediate_files[deps_found] == 0 && tryrules[foundrule]->terminal)
	{
	  /* If the file actually existed (was not an intermediate file),
	     and the rule that found it was a terminal one, then we want
	     to mark the found file so that it will not have implicit rule
	     search done for it.  If we are not entering a `struct file' for
	     it now, we indicate this with the `changed' flag.  */
	  if (dep->file == 0)
	    dep->changed = 1;
	  else
	    dep->file->tried_implicit = 1;
	}
      dep->next = file->deps;
      file->deps = dep;
    }
 
  uniquize_deps (file->deps);

  if (!checked_lastslash[foundrule])
    file->stem = stem[stemlen] == '\0' ? stem : savestring (stem, stemlen);
  else
    {
      file->stem = (char *) xmalloc (((lastslash + 1) - filename)
				     + stemlen + 1);
      bcopy (filename, file->stem, (lastslash + 1) - filename);
      bcopy (stem, file->stem + ((lastslash + 1) - filename), stemlen);
      file->stem[((lastslash + 1) - filename) + stemlen] = '\0';
    }

  file->cmds = rule->cmds;

  /* Put the targets other than the one that
     matched into FILE's `also_make' member.  */

  /* If there was only one target, there is nothing to do.  */
  if (rule->targets[1] != 0)
    for (i = 0; rule->targets[i] != 0; ++i)
      if (i != matches[foundrule])
	{
	  struct dep *new = (struct dep *) xmalloc (sizeof (struct dep));
	  new->name = p = (char *) xmalloc (rule->lens[i] + stemlen + 1);
	  bcopy (rule->targets[i], p,
		 rule->suffixes[i] - rule->targets[i] - 1);
	  p += rule->suffixes[i] - rule->targets[i] - 1;
	  bcopy (stem, p, stemlen);
	  p += stemlen;
	  bcopy (rule->suffixes[i], p,
		 rule->lens[i]
		 - (rule->suffixes[i] - rule->targets[i] - 1) + 1);
	  new->file = enter_file (new->name);
	  new->next = file->also_make;
	  file->also_make = new;
	}


  return 1;
}
