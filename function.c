/* Variable function expansion for GNU Make.
Copyright (C) 1988, 1989, 1991, 1992, 1993 Free Software Foundation, Inc.
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
#include "variable.h"
#include "dep.h"
#include "commands.h"
#include "job.h"

static char *string_glob ();

/* Store into VARIABLE_BUFFER at O the result of scanning TEXT and replacing
   each occurrence of SUBST with REPLACE. TEXT is null-terminated.  SLEN is
   the length of SUBST and RLEN is the length of REPLACE.  If BY_WORD is
   nonzero, substitutions are done only on matches which are complete
   whitespace-delimited words.  If SUFFIX_ONLY is nonzero, substitutions are
   done only at the ends of whitespace-delimited words.  */
   
char *
subst_expand (o, text, subst, replace, slen, rlen, by_word, suffix_only)
     char *o;
     char *text;
     char *subst, *replace;
     unsigned int slen, rlen;
     int by_word, suffix_only;
{
  register char *t = text;
  register char *p;

  if (slen == 0 && !by_word && !suffix_only)
    {
      /* The first occurrence of "" in any string is its end.  */
      o = variable_buffer_output (o, t, strlen (t));
      if (rlen > 0)
	o = variable_buffer_output (o, replace, rlen);
      return o;
    }

  while ((p = sindex (t, 0, subst, slen)) != 0)
    {
      /* Output everything before this occurrence of the string to replace.  */
      if (p > t)
	o = variable_buffer_output (o, t, p - t);

      /* If we're substituting only by fully matched words,
	 or only at the ends of words, check that this case qualifies.  */
      if ((by_word
	   && ((p > t && !isblank (p[-1]))
	       || (p[slen] != '\0' && !isblank (p[slen]))))
	  || (suffix_only
	      && (p[slen] != '\0' && !isblank (p[slen]))))
	/* Struck out.  Output the rest of the string that is
	   no longer to be replaced.  */
	o = variable_buffer_output (o, subst, slen);
      else if (rlen > 0)
	/* Output the replacement string.  */
	o = variable_buffer_output (o, replace, rlen);

      /* Advance T past the string to be replaced.  */
      t = p + slen;
    }

  /* Output everything left on the end.  */
  if (*t != '\0')
    o = variable_buffer_output (o, t, strlen (t));

  return o;
}


/* Store into VARIABLE_BUFFER at O the result of scanning TEXT
   and replacing strings matching PATTERN with REPLACE.
   If PATTERN_PERCENT is not nil, PATTERN has already been
   run through find_percent, and PATTERN_PERCENT is the result.
   If REPLACE_PERCENT is not nil, REPLACE has already been
   run through find_percent, and REPLACE_PERCENT is the result.  */

char *
patsubst_expand (o, text, pattern, replace, pattern_percent, replace_percent)
     char *o;
     char *text;
     register char *pattern, *replace;
     register char *pattern_percent, *replace_percent;
{
  register int pattern_prepercent_len, pattern_postpercent_len;
  register int replace_prepercent_len, replace_postpercent_len;
  register char *t;
  unsigned int len;
  int doneany = 0;

  /* We call find_percent on REPLACE before checking PATTERN so that REPLACE
     will be collapsed before we call subst_expand if PATTERN has no %.  */
  if (replace_percent == 0)
    replace_percent = find_percent (replace);
  if (replace_percent != 0)
    {
      /* Record the length of REPLACE before and after the % so
	 we don't have to compute these lengths more than once.  */
      replace_prepercent_len = replace_percent - replace;
      replace_postpercent_len = strlen (replace_percent + 1);
    }
  else
    /* We store the length of the replacement
       so we only need to compute it once.  */
    replace_prepercent_len = strlen (replace);

  if (pattern_percent == 0)
    pattern_percent = find_percent (pattern);
  if (pattern_percent == 0)
    /* With no % in the pattern, this is just a simple substitution.  */
    return subst_expand (o, text, pattern, replace,
			 strlen (pattern), strlen (replace), 1, 0);

  /* Record the length of PATTERN before and after the %
     so we don't have to compute it more than once.  */
  pattern_prepercent_len = pattern_percent - pattern;
  pattern_postpercent_len = strlen (pattern_percent + 1);

  while ((t = find_next_token (&text, &len)) != 0)
    {
      int fail = 0;

      /* Is it big enough to match?  */
      if (len < pattern_prepercent_len + pattern_postpercent_len)
	fail = 1;

      /* Does the prefix match?  */
      if (!fail && pattern_prepercent_len > 0
	  && (*t != *pattern
	      || t[pattern_prepercent_len - 1] != pattern_percent[-1]
	      || strncmp (t + 1, pattern + 1, pattern_prepercent_len - 1)))
	fail = 1;

      /* Does the suffix match?  */
      if (!fail && pattern_postpercent_len > 0
	  && (t[len - 1] != pattern_percent[pattern_postpercent_len]
	      || t[len - pattern_postpercent_len] != pattern_percent[1]
	      || strncmp (&t[len - pattern_postpercent_len],
			  &pattern_percent[1], pattern_postpercent_len - 1)))
	fail = 1;

      if (fail)
	/* It didn't match.  Output the string.  */
	o = variable_buffer_output (o, t, len);
      else
	{
	  /* It matched.  Output the replacement.  */

	  /* Output the part of the replacement before the %.  */
	  o = variable_buffer_output (o, replace, replace_prepercent_len);

	  if (replace_percent != 0)
	    {
	      /* Output the part of the matched string that
		 matched the % in the pattern.  */
	      o = variable_buffer_output (o, t + pattern_prepercent_len,
					  len - (pattern_prepercent_len
						 + pattern_postpercent_len));
	      /* Output the part of the replacement after the %.  */
	      o = variable_buffer_output (o, replace_percent + 1,
					  replace_postpercent_len);
	    }
	}

      /* Output a space, but not if the replacement is "".  */
      if (fail || replace_prepercent_len > 0
	  || (replace_percent != 0 && len + replace_postpercent_len > 0))
	{
	  o = variable_buffer_output (o, " ", 1);
	  doneany = 1;
	}
    }
  if (doneany)
    /* Kill the last space.  */
    --o;

  return o;
}

/* Handle variable-expansion-time functions such as $(dir foo/bar) ==> foo/  */

/* These enumeration constants distinguish the
   various expansion-time built-in functions.  */

enum function
  {
    function_subst,
    function_addsuffix,
    function_addprefix,
    function_dir,
    function_notdir,
    function_suffix,
    function_basename,
    function_wildcard,
    function_firstword,
    function_word,
    function_words,
    function_findstring,
    function_strip,
    function_join,
    function_patsubst,
    function_filter,
    function_filter_out,
    function_foreach,
    function_sort,
    function_origin,
    function_shell,
    function_invalid
  };

/* Greater than the length of any function name.  */
#define MAXFUNCTIONLEN 11

/* The function names and lengths of names, for looking them up.  */

static struct
  {
    char *name;
    unsigned int len;
    enum function function;
  } function_table[] =
  {
    { "subst", 5, function_subst },
    { "addsuffix", 9, function_addsuffix },
    { "addprefix", 9, function_addprefix },
    { "dir", 3, function_dir },
    { "notdir", 6, function_notdir },
    { "suffix", 6, function_suffix },
    { "basename", 8, function_basename },
    { "wildcard", 8, function_wildcard },
    { "firstword", 9, function_firstword },
    { "word", 4, function_word },
    { "words", 5, function_words },
    { "findstring", 10, function_findstring },
    { "strip", 5, function_strip },
    { "join", 4, function_join },
    { "patsubst", 8, function_patsubst },
    { "filter", 6, function_filter },
    { "filter-out", 10, function_filter_out },
    { "foreach", 7, function_foreach },
    { "sort", 4, function_sort },
    { "origin", 6, function_origin },
    { "shell", 5, function_shell },
    { 0, 0, function_invalid }
  };

/* Return 1 if PATTERN matches WORD, 0 if not.  */

int
pattern_matches (pattern, percent, word)
     register char *pattern, *percent, *word;
{
  unsigned int sfxlen, wordlen;

  if (percent == 0)
    {
      unsigned int len = strlen (pattern) + 1;
      char *new = (char *) alloca (len);
      bcopy (pattern, new, len);
      pattern = new;
      percent = find_percent (pattern);
      if (percent == 0)
	return streq (pattern, word);
    }

  sfxlen = strlen (percent + 1);
  wordlen = strlen (word);

  if (wordlen < (percent - pattern) + sfxlen
      || strncmp (pattern, word, percent - pattern))
    return 0;

  return !strcmp (percent + 1, word + (wordlen - sfxlen));
}

int shell_function_pid = 0, shell_function_completed;

/* Perform the function specified by FUNCTION on the text at TEXT.
   END is points to the end of the argument text (exclusive).
   The output is written into VARIABLE_BUFFER starting at O.  */

/* Note this absorbs a semicolon and is safe to use in conditionals.  */
#define BADARGS(func)							      \
  if (reading_filename != 0)						      \
    makefile_fatal (reading_filename, *reading_lineno_ptr,		      \
		    "insufficient arguments to function `%s'", 		      \
		    func);						      \
  else									      \
    fatal ("insufficient arguments to function `%s'", func)

static char *
expand_function (o, function, text, end)
     char *o;
     enum function function;
     char *text;
     char *end;
{
  char *p, *p2, *p3;
  unsigned int i, len;
  int doneany = 0;
  int count;
  char endparen = *end, startparen = *end == ')' ? '(' : '{';

  switch (function)
    {
    default:
      abort ();
      break;
      
    case function_shell:
      {
	char **argv, **envp;
	char *error_prefix;
	int pipedes[2];
	int pid;

	/* Expand the command line.  */
	text = expand_argument (text, end);

	/* Construct the argument list.  */
	argv = construct_command_argv (text, (char *) NULL, (struct file *) 0);
	if (argv == 0)
	  break;

	/* Using a target environment for `shell' loses in cases like:
	   	export var = $(shell echo foobie) 
	   because target_environment hits a loop trying to expand $(var)
	   to put it in the environment.  This is even more confusing when
	   var was not explicitly exported, but just appeared in the
	   calling environment.  */
#if 1
	envp = environ;
#else
	/* Construct the environment.  */
	envp = target_environment ((struct file *) 0);
#endif

	/* For error messages.  */
	if (reading_filename != 0)
	  {
	    error_prefix = (char *) alloca (strlen (reading_filename) + 100);
	    sprintf (error_prefix,
		     "%s:%u: ", reading_filename, *reading_lineno_ptr);
	  }
	else
	  error_prefix = "";

	if (pipe (pipedes) < 0)
	  {
	    perror_with_name (error_prefix, "pipe");
	    break;
	  }

	pid = vfork ();
	if (pid < 0)
	  perror_with_name (error_prefix, "fork");
	else if (pid == 0)
	  child_execute_job (0, pipedes[1], argv, envp);
	else
	  {
	    /* We are the parent.  */

	    char *buffer;
	    unsigned int maxlen;
	    int cc;

	    /* Free the storage only the child needed.  */
	    free (argv[0]);
	    free ((char *) argv);
#if 0
	    for (i = 0; envp[i] != 0; ++i)
	      free (envp[i]);
	    free ((char *) envp);
#endif

	    /* Record the PID for reap_children.  */
	    shell_function_pid = pid;
	    shell_function_completed = 0;


	    /* Set up and read from the pipe.  */

	    maxlen = 200;
	    buffer = (char *) xmalloc (maxlen + 1);

	    /* Close the write side of the pipe.  */
	    (void) close (pipedes[1]);

	    /* Read from the pipe until it gets EOF.  */
	    i = 0;
	    do
	      {
		if (i == maxlen)
		  {
		    maxlen += 512;
		    buffer = (char *) xrealloc (buffer, maxlen + 1);
		  }

		errno = 0;
		cc = read (pipedes[0], &buffer[i], maxlen - i);
		if (cc > 0)
		  i += cc;
	      }
#ifdef EINTR
	    while (cc > 0 || errno == EINTR);
#else
	    while (cc > 0);
#endif

	    /* Close the read side of the pipe.  */
	    (void) close (pipedes[0]);

	    /* Loop until child_handler sets shell_function_completed
	       to the status of our child shell.  */
	    while (shell_function_completed == 0)
	      reap_children (1, 0);

	    shell_function_pid = 0;

	    /* The child_handler function will set shell_function_completed
	       to 1 when the child dies normally, or to -1 if it
	       dies with status 127, which is most likely an exec fail.  */

	    if (shell_function_completed == -1)
	      {
		/* This most likely means that the execvp failed,
		   so we should just write out the error message
		   that came in over the pipe from the child.  */
		fputs (buffer, stderr);
		fflush (stderr);
	      }
	    else
	      {
		/* The child finished normally.  Replace all
		   newlines in its output with spaces, and put
		   that in the variable output buffer.  */
		if (i > 0)
		  {
		    if (buffer[i - 1] == '\n')
		      buffer[--i] = '\0';
		    else
		      buffer[i] = '\0';
		    p = buffer;
		    while ((p = index (p, '\n')) != 0)
		      *p++ = ' ';
		    o = variable_buffer_output (o, buffer, i);
		  }
	      }

	    free (buffer);
	  }

	free (text);
	break;
      }

    case function_origin:
      /* Expand the argument.  */
      text = expand_argument (text, end);

      {
	register struct variable *v = lookup_variable (text, strlen (text));
	if (v == 0)
	  o = variable_buffer_output (o, "undefined", 9);
	else
	  switch (v->origin)
	    {
	    default:
	    case o_invalid:
	      abort ();
	      break;
	    case o_default:
	      o = variable_buffer_output (o, "default", 7);
	      break;
	    case o_env:
	      o = variable_buffer_output (o, "environment", 11);
	      break;
	    case o_file:
	      o = variable_buffer_output (o, "file", 4);
	      break;
	    case o_env_override:
	      o = variable_buffer_output (o, "environment override", 20);
	      break;
	    case o_command:
	      o = variable_buffer_output (o, "command line", 12);
	      break;
	    case o_override:
	      o = variable_buffer_output (o, "override", 8);
	      break;
	    case o_automatic:
	      o = variable_buffer_output (o, "automatic", 9);
	      break;
	    }
      }

      free (text);
      break;
      
    case function_sort:
      /* Expand the argument.  */
      text = expand_argument (text, end);

      {
	char **words = (char **) xmalloc (10 * sizeof (char *));
	unsigned int nwords = 10;
	register unsigned int wordi = 0;
	char *t;

	/* Chop TEXT into words and put them in WORDS.  */
	t = text;
	while ((p = find_next_token (&t, &len)) != 0)
	  {
	    if (wordi >= nwords - 1)
	      {
		nwords *= 2;
		words = (char **) xrealloc ((char *) words,
					    nwords * sizeof (char *));
	      }	
	    words[wordi++] = savestring (p, len);
	  }

	if (wordi > 0)
	  {
	    /* Now sort the list of words.  */
	    qsort ((char *) words, wordi, sizeof (char *), alpha_compare);

	    /* Now write the sorted list.  */
	    for (i = 0; i < wordi; ++i)
	      {
		len = strlen (words[i]);
		if (i == wordi - 1 || strlen (words[i + 1]) != len
		    || strcmp (words[i], words[i + 1]))
		  {
		    o = variable_buffer_output (o, words[i], len);
		    o = variable_buffer_output (o, " ", 1);
		  }
		free (words[i]);
	      }
	    /* Kill the last space.  */
	    --o;
	  }

	free ((char *) words);
      }

      free (text);
      break;
      
    case function_foreach:
      {
	/* Get three comma-separated arguments but
	   expand only the first two.  */
	char *var, *list;
	register struct variable *v;

	count = 0;
	for (p = text; p < end; ++p)
	  {
	    if (*p == startparen)
	      ++count;
	    else if (*p == endparen)
	      --count;
	    else if (*p == ',' && count <= 0)
	      break;
	  }
	if (p == end)
	  BADARGS ("foreach");
	var = expand_argument (text, p);

	p2 = p + 1;
	count = 0;
	for (p = p2; p < end; ++p)
	  {
	    if (*p == startparen)
	      ++count;
	    else if (*p == endparen)
	      --count;
	    else if (*p == ',' && count <= 0)
	      break;
	  }
	if (p == end)
	  BADARGS ("foreach");
	list = expand_argument (p2, p);

	++p;
	text = savestring (p, end - p);

	push_new_variable_scope ();
	v = define_variable (var, strlen (var), "", o_automatic, 0);
	p3 = list;
	while ((p = find_next_token (&p3, &len)) != 0)
	  {
	    char *result;
	    char save = p[len];
	    p[len] = '\0';
	    v->value = p;
	    result = allocated_variable_expand (text);
	    p[len] = save;

	    o = variable_buffer_output (o, result, strlen (result));
	    o = variable_buffer_output (o, " ", 1);
	    doneany = 1;
	    free (result);
	  }
	if (doneany)
	  /* Kill the last space.  */
	  --o;

	pop_variable_scope ();

	free (var);
	free (list);
	free (text);
      }
      break;

    case function_filter:
    case function_filter_out:
      {
	struct word
	  {
	    struct word *next;
	    char *word;
	    int matched;
	  } *words, *wordtail, *wp;

	/* Get two comma-separated arguments and expand each one.  */
	count = 0;
	for (p = text; p < end; ++p)
	  {
	    if (*p == startparen)
	      ++count;
	    else if (*p == endparen)
	      --count;
	    else if (*p == ',' && count <= 0)
	      break;
	  }
	if (p == end)
	  BADARGS (function == function_filter ? "filter" : "filter-out");
	p2 = expand_argument (text, p);
	
	text = expand_argument (p + 1, end);

	/* Chop TEXT up into words and then run each pattern through.  */
	words = wordtail = 0;
	p3 = text;
	while ((p = find_next_token (&p3, &len)) != 0)
	  {
	    struct word *w = (struct word *) alloca (sizeof (struct word));
	    if (words == 0)
	      words = w;
	    else
	      wordtail->next = w;
	    wordtail = w;

	    if (*p3 != '\0')
	      ++p3;
	    p[len] = '\0';
	    w->word = p;
	    w->matched = 0;
	  }

	if (words != 0)
	  {
	    wordtail->next = 0;

	    /* Run each pattern through the words, killing words.  */
	    p3 = p2;
	    while ((p = find_next_token (&p3, &len)) != 0)
	      {
		char *percent;
		char save = p[len];
		p[len] = '\0';

		percent = find_percent (p);
		for (wp = words; wp != 0; wp = wp->next)
		  wp->matched |= (percent == 0 ? streq (p, wp->word)
				  : pattern_matches (p, percent, wp->word));

		p[len] = save;
	      }

	    /* Output the words that matched (or didn't, for filter-out).  */
	    for (wp = words; wp != 0; wp = wp->next)
	      if (function == function_filter ? wp->matched : !wp->matched)
		{
		  o = variable_buffer_output (o, wp->word, strlen (wp->word));
		  o = variable_buffer_output (o, " ", 1);
		  doneany = 1;
		}
	    if (doneany)
	      /* Kill the last space.  */
	      --o;
	  }

	free (p2);
	free (text);
      }
      break;
      
    case function_patsubst:
      /* Get three comma-separated arguments and expand each one.  */
      count = 0;
      for (p = text; p < end; ++p)
	{
	  if (*p == startparen)
	    ++count;
	  else if (*p == endparen)
	    --count;
	  else if (*p == ',' && count <= 0)
	    break;
	}
      if (p == end)
	BADARGS ("patsubst");

      p2 = p;
      count = 0;
      for (++p; p < end; ++p)
	{
	  if (*p == startparen)
	    ++count;
	  else if (*p == endparen)
	    --count;
	  else if (*p == ',' && count <= 0)
	    break;
	}
      if (p == end)
	BADARGS ("patsubst");

      text = expand_argument (text, p2);
      p3 = expand_argument (p2 + 1, p);
      p2 = expand_argument (p + 1, end);
      
      o = patsubst_expand (o, p2, text, p3, (char *) 0, (char *) 0);
      
      free (text);
      free (p3);
      free (p2);
      break;

    case function_join:
      /* Get two comma-separated arguments and expand each one.  */
      count = 0;
      for (p = text; p < end; ++p)
	{
	  if (*p == startparen)
	    ++count;
	  else if (*p == endparen)
	    --count;
	  else if (*p == ',' && count <= 0)
	    break;
	}
      if (p == end)
	BADARGS ("join");
      text = expand_argument (text, p);

      p = expand_argument (p + 1, end);
      
      {
	/* Write each word of the first argument directly followed
	   by the corresponding word of the second argument.
	   If the two arguments have a different number of words,
	   the excess words are just output separated by blanks.  */
	register char *tp, *pp;
	p2 = text;
	p3 = p;
	do
	  {
	    unsigned int tlen, plen;

	    tp = find_next_token (&p2, &tlen);
	    if (tp != 0)
	      o = variable_buffer_output (o, tp, tlen);
	    
	    pp = find_next_token (&p3, &plen);
	    if (pp != 0)
	      o = variable_buffer_output (o, pp, plen);
	    
	    if (tp != 0 || pp != 0)
	      {
		o = variable_buffer_output (o, " ", 1);
		doneany = 1;
	      }
	  }
	while (tp != 0 || pp != 0);
	if (doneany)
	  /* Kill the last blank.  */
	  --o;
      }
      
      free (text);
      free (p);
      break;
      
    case function_strip:
      /* Expand the argument.  */
      text = expand_argument (text, end);

      p2 = text;
      while ((p = find_next_token (&p2, &i)) != 0)
	{
	  o = variable_buffer_output (o, p, i);
	  o = variable_buffer_output (o, " ", 1);
	  doneany = 1;
	}
      if (doneany)
	/* Kill the last space.  */
	--o;
      
      free (text);
      break;
      
    case function_wildcard:
      text = expand_argument (text, end);
      
      p = string_glob (text);
      o = variable_buffer_output (o, p, strlen (p));
      
      free (text);
      break;
      
    case function_subst:
      /* Get three comma-separated arguments and expand each one.  */
      count = 0;
      for (p = text; p < end; ++p)
	{
	  if (*p == startparen)
	    ++count;
	  else if (*p == endparen)
	    --count;
	  else if (*p == ',' && count <= 0)
	    break;
	}
      if (p == end)
	BADARGS ("subst");

      p2 = p;
      count = 0;
      for (++p; p < end; ++p)
	{
	  if (*p == startparen)
	    ++count;
	  else if (*p == endparen)
	    --count;
	  else if (*p == ',' && count <= 0)
	    break;
	}
      if (p == end)
	BADARGS ("subst");

      text = expand_argument (text, p2);
      p3 = expand_argument (p2 + 1, p);
      p2 = expand_argument (p + 1, end);
      
      o = subst_expand (o, p2, text, p3, strlen (text), strlen (p3), 0, 0);
      
      free (text);
      free (p3);
      free (p2);
      break;
      
    case function_firstword:
      /* Expand the argument.  */
      text = expand_argument (text, end);

      /* Find the first word in TEXT.  */
      p2 = text;
      p = find_next_token (&p2, &i);
      if (p != 0)
	o = variable_buffer_output (o, p, i);
      
      free (text);
      break;
      
    case function_word:
      /* Get two comma-separated arguments and expand each one.  */
      count = 0;
      for (p = text; p < end; ++p)
	{
	  if (*p == startparen)
	    ++count;
	  else if (*p == endparen)
	    --count;
	  else if (*p == ',' && count <= 0)
	    break;
	}
      if (p == end)
	BADARGS ("word");
      text = expand_argument (text, p);

      p3 = expand_argument (p + 1, end);

      /* Check the first argument.  */
      for (p2 = text; *p2 != '\0'; ++p2)
	if (*p2 < '0' || *p2 > '9')
	  {
	    if (reading_filename != 0)
	      makefile_fatal (reading_filename, *reading_lineno_ptr,
			      "non-numeric first argument to `word' function");
	    else
	      fatal ("non-numeric first argument to `word' function");
	  }

      i = (unsigned int) atoi (text);
      if (i == 0)
	{
	  if (reading_filename != 0)
	    makefile_fatal (reading_filename, *reading_lineno_ptr,
			    "the `word' function takes a one-origin \
index argument");
	  else
	    fatal ("the `word' function takes a one-origin index argument");
	}

      p2 = p3;
      while ((p = find_next_token (&p2, &len)) != 0)
	if (--i == 0)
	  break;
      if (i == 0)
	o = variable_buffer_output (o, p, len);

      free (text);
      free (p3);
      break;

    case function_words:
      /* Expand the argument.  */
      text = expand_argument (text, end);

      i = 0;
      p2 = text;
      while (find_next_token (&p2, (unsigned int *) 0) != 0)
	++i;

      {
	char buf[20];
	sprintf (buf, "%d", i);
	o = variable_buffer_output (o, buf, strlen (buf));
      }

      free (text);
      break;

    case function_findstring:
      /* Get two comma-separated arguments and expand each one.  */
      count = 0;
      for (p = text; p < end; ++p)
	{
	  if (*p == startparen)
	    ++count;
	  else if (*p == endparen)
	    --count;
	  else if (*p == ',' && count <= 0)
	    break;
	}
      if (p == end)
	BADARGS ("findstring");
      text = expand_argument (text, p);

      p = expand_argument (p + 1, end);

      /* Find the first occurrence of the first string in the second.  */
      i = strlen (text);
      if (sindex (p, 0, text, i) != 0)
	o = variable_buffer_output (o, text, i);
      
      free (p);
      free (text);
      break;
      
    case function_addsuffix:
    case function_addprefix:
      /* Get two comma-separated arguments and expand each one.  */
      count = 0;
      for (p = text; p < end; ++p)
	{
	  if (*p == startparen)
	    ++count;
	  else if (*p == endparen)
	    --count;
	  else if (*p == ',' && count <= 0)
	    break;
	}
      if (p == end)
	BADARGS (function == function_addsuffix ? "addsuffix" : "addprefix");
      text = expand_argument (text, p);
      i = strlen (text);

      p2 = expand_argument (p + 1, end);
      
      p3 = p2;
      while ((p = find_next_token (&p3, &len)) != 0)
	{
	  if (function == function_addprefix)
	    o = variable_buffer_output (o, text, i);
	  o = variable_buffer_output (o, p, len);
	  if (function == function_addsuffix)
	    o = variable_buffer_output (o, text, i);
	  o = variable_buffer_output (o, " ", 1);
	  doneany = 1;
	}
      if (doneany)
	/* Kill last space.  */
	--o;
      
      free (p2);
      free (text);
      break;
      
    case function_dir:
    case function_basename:
      /* Expand the argument.  */
      text = expand_argument (text, end);

      p3 = text;
      while ((p2 = find_next_token (&p3, &len)) != 0)
	{
	  p = p2 + len;
	  while (p >= p2 && *p != (function == function_dir ? '/' : '.'))
	    --p;
	  if (p >= p2)
	    {
	      if (function == function_dir)
		++p;
	      o = variable_buffer_output (o, p2, p - p2);
	    }
	  else if (function == function_dir)
            o = variable_buffer_output (o, "./", 2);
	  else
	    /* The entire name is the basename.  */
	    o = variable_buffer_output (o, p2, len);

	  o = variable_buffer_output (o, " ", 1);
	  doneany = 1;
	}
      if (doneany)
	/* Kill last space.  */
	--o;
      
      free (text);
      break;
      
    case function_notdir:
    case function_suffix:
      /* Expand the argument.  */
      text = expand_argument (text, end);

      p3 = text;
      while ((p2 = find_next_token (&p3, &len)) != 0)
	{
	  p = p2 + len;
	  while (p >= p2 && *p != (function == function_notdir ? '/' : '.'))
	    --p;
	  if (p >= p2)
	    {
	      if (function == function_notdir)
		++p;
	      o = variable_buffer_output (o, p, len - (p - p2));
	    }
	  else if (function == function_notdir)
	    o = variable_buffer_output (o, p2, len);

	  if (function == function_notdir || p >= p2)
	    {
	      o = variable_buffer_output (o, " ", 1);
	      doneany = 1;
	    }
	}
      if (doneany)
	/* Kill last space.  */
	--o;

      free (text);
      break;
    }

  return o;
}

/* Check for a function invocation in *STRINGP.  *STRINGP points at the
   opening ( or { and is not null-terminated.  If a function invocation
   is found, expand it into the buffer at *OP, updating *OP, incrementing
   *STRINGP past the reference and returning nonzero.  If not, return zero.  */

int
handle_function (op, stringp)
     char **op;
     char **stringp;

{
  register unsigned int code;
  unsigned int maxlen;
  char *beg = *stringp + 1;
  char *endref;

  endref = lindex (beg, beg + MAXFUNCTIONLEN, '\0');
  maxlen = endref != 0 ? endref - beg : MAXFUNCTIONLEN;

  for (code = 0; function_table[code].name != 0; ++code)
    {
      if (maxlen < function_table[code].len)
	continue;
      endref = beg + function_table[code].len;
      if (isblank (*endref)
	  && !strncmp (function_table[code].name, beg,
		       function_table[code].len))
	break;
    }
  if (function_table[code].name != 0)
    {
      /* We have found a call to an expansion-time function.
	 Find the end of the arguments, and do the function.  */

      char openparen = beg[-1], closeparen = openparen == '(' ? ')' : '}';
      int count = 0;
      char *argbeg;
      register char *p;

      /* Space after function name isn't part of the args.  */
      p = next_token (endref);
      argbeg = p;

      /* Count nested use of whichever kind of parens we use,
	 so that nested calls and variable refs work.  */

      for (; *p != '\0'; ++p)
	{
	  if (*p == openparen)
	    ++count;
	  else if (*p == closeparen && --count < 0)
	    break;
	}

      if (count >= 0)
	{
	  static const char errmsg[]
	    = "unterminated call to function `%s': missing `%c'";
	  if (reading_filename == 0)
	    fatal (errmsg, function_table[code].name, closeparen);
	  else
	    makefile_fatal (reading_filename, *reading_lineno_ptr, errmsg,
			    function_table[code].name, closeparen);
	}

      /* We found the end; expand the function call.  */

      *op = expand_function (*op, function_table[code].function, argbeg, p);
      *stringp = p;
      return 1;
    }

  return 0;
}

/* Glob-expand LINE.  The returned pointer is
   only good until the next call to string_glob.  */

static char *
string_glob (line)
     char *line;
{
  static char *result = 0;
  static unsigned int length;
  register struct nameseq *chain;
  register unsigned int idx;

  chain = multi_glob (parse_file_seq
		      (&line, '\0', sizeof (struct nameseq),
		       /* We do not want parse_file_seq to strip `./'s.
			  That would break examples like:
			  $(patsubst ./%.c,obj/%.o,$(wildcard ./*.c)).  */
		       0),
		      sizeof (struct nameseq));

  if (result == 0)
    {
      length = 100;
      result = (char *) xmalloc (100);
    }

  idx = 0;
  while (chain != 0)
    {
      register char *name = chain->name;
      unsigned int len = strlen (name);

      struct nameseq *next = chain->next;
      free ((char *) chain);
      chain = next;

      /* multi_glob will pass names without globbing metacharacters
	 through as is, but we want only files that actually exist.  */
      if (file_exists_p (name))
	{
	  if (idx + len + 1 > length)
	    {
	      length += (len + 1) * 2;
	      result = (char *) xrealloc (result, length);
	    }
	  bcopy (name, &result[idx], len);
	  idx += len;
	  result[idx++] = ' ';
	}

      free (name);
    }

  /* Kill the last space and terminate the string.  */
  if (idx == 0)
    result[0] = '\0';
  else
    result[idx - 1] = '\0';

  return result;
}
