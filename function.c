/* Builtin function expansion for GNU Make.
Copyright (C) 1988,89,91,92,93,94,95,96,97 Free Software Foundation, Inc.
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

#include "make.h"
#include "filedef.h"
#include "variable.h"
#include "dep.h"
#include "job.h"
#include "commands.h"

#ifdef _AMIGA
#include "amiga.h"
#endif


struct function_table_entry
  {
    const char *name;
    int len;
    int required_args;
    int expand_args;
    char *(*func_ptr) PARAMS((char *output, char **argv, const char*funcname));
  };


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

  do
    {
      if ((by_word | suffix_only) && slen == 0)
	/* When matching by words, the empty string should match
	   the end of each word, rather than the end of the whole text.  */
	p = end_of_token (next_token (t));
      else
	{
	  p = sindex (t, 0, subst, slen);
	  if (p == 0)
	    {
	      /* No more matches.  Output everything left on the end.  */
	      o = variable_buffer_output (o, t, strlen (t));
	      return o;
	    }
	}

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
    } while (*t != '\0');

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
  unsigned int pattern_prepercent_len, pattern_postpercent_len;
  unsigned int replace_prepercent_len, replace_postpercent_len = 0;
  char *t;
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

      /* Does the prefix match? */
      if (!fail && pattern_prepercent_len > 0
	  && (*t != *pattern
	      || t[pattern_prepercent_len - 1] != pattern_percent[-1]
	      || !strneq (t + 1, pattern + 1, pattern_prepercent_len - 1)))
	fail = 1;

      /* Does the suffix match? */
      if (!fail && pattern_postpercent_len > 0
	  && (t[len - 1] != pattern_percent[pattern_postpercent_len]
	      || t[len - pattern_postpercent_len] != pattern_percent[1]
	      || !strneq (&t[len - pattern_postpercent_len],
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


/* Look up a function by name.
   The table is currently small enough that it's not really worthwhile to use
   a fancier lookup algorithm.  If it gets larger, maybe...
*/

static const struct function_table_entry *
lookup_function (table, s)
     const struct function_table_entry *table;
     const char *s;
{
  int len = strlen(s);

  for (; table->name != NULL; ++table)
    if (table->len <= len
        && (isblank (s[table->len]) || s[table->len] == '\0')
        && strneq (s, table->name, table->len))
      return table;

  return NULL;
}


/* Return 1 if PATTERN matches STR, 0 if not.  */

int
pattern_matches (pattern, percent, str)
     register char *pattern, *percent, *str;
{
  unsigned int sfxlen, strlength;

  if (percent == 0)
    {
      unsigned int len = strlen (pattern) + 1;
      char *new_chars = (char *) alloca (len);
      bcopy (pattern, new_chars, len);
      pattern = new_chars;
      percent = find_percent (pattern);
      if (percent == 0)
	return streq (pattern, str);
    }

  sfxlen = strlen (percent + 1);
  strlength = strlen (str);

  if (strlength < (percent - pattern) + sfxlen
      || !strneq (pattern, str, percent - pattern))
    return 0;

  return !strcmp (percent + 1, str + (strlength - sfxlen));
}


/* Find the next comma or ENDPAREN (counting nested STARTPAREN and
   ENDPARENtheses), starting at PTR before END.  Return a pointer to
   next character.

   If no next argument is found, return NULL.
*/

static char *
find_next_argument (startparen, endparen, ptr, end)
     char startparen;
     char endparen;
     const char *ptr;
     const char *end;
{
  int count = 0;

  for (; ptr < end; ++ptr)
    if (*ptr == startparen)
      ++count;

    else if (*ptr == endparen)
      {
	--count;
	if (count < 0)
	  return NULL;
      }

    else if (*ptr == ',' && !count)
      return (char *)ptr;

  /* We didn't find anything.  */
  return NULL;
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
			  $(patsubst ./%.c,obj/%.o,$(wildcard ./?*.c)).  */
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

/*
  Builtin functions
 */

static char *
func_patsubst (o, argv, funcname)
     char *o;
     char **argv;
     const char *funcname;
{
  o = patsubst_expand (o, argv[2], argv[0], argv[1], (char *) 0, (char *) 0);
  return o;
}


static char *
func_join(o, argv, funcname)
     char *o;
     char **argv;
     const char *funcname;
{
  int doneany = 0;

  /* Write each word of the first argument directly followed
     by the corresponding word of the second argument.
     If the two arguments have a different number of words,
     the excess words are just output separated by blanks.  */
  register char *tp;
  register char *pp;
  char *list1_iterator = argv[0];
  char *list2_iterator = argv[1];
  do
    {
      unsigned int len1, len2;

      tp = find_next_token (&list1_iterator, &len1);
      if (tp != 0)
	o = variable_buffer_output (o, tp, len1);

      pp = find_next_token (&list2_iterator, &len2);
      if (pp != 0)
	o = variable_buffer_output (o, pp, len2);

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

  return o;
}


static char *
func_origin(o, argv, funcname)
     char *o;
     char **argv;
     const char *funcname;
{
  /* Expand the argument.  */
  register struct variable *v = lookup_variable (argv[0], strlen (argv[0]));
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

  return o;
}

#ifdef VMS
#define IS_PATHSEP(c) ((c) == ']')
#else
#if defined(__MSDOS__) || defined(WINDOWS32)
#define IS_PATHSEP(c) ((c) == '/' || (c) == '\\')
#else
#define IS_PATHSEP(c) ((c) == '/')
#endif
#endif


static char *
func_notdir_suffix(o, argv, funcname)
     char *o;
     char **argv;
     const char *funcname;
{
  /* Expand the argument.  */
  char *list_iterator = argv[0];
  char *p2 =0;
  int doneany =0;
  int len=0;

  int is_suffix = streq(funcname, "suffix");
  int is_notdir = !is_suffix;
  while ((p2 = find_next_token (&list_iterator, &len)) != 0)
    {
      char *p = p2 + len;


      while (p >= p2 && (!is_suffix || *p != '.'))
	{
	  if (IS_PATHSEP (*p))
	    break;
	  --p;
	}

      if (p >= p2)
	{
	  if (is_notdir)
	    ++p;
	  else if (*p != '.')
	    continue;
	  o = variable_buffer_output (o, p, len - (p - p2));
	}
#if defined(WINDOWS32) || defined(__MSDOS__)
      /* Handle the case of "d:foo/bar".  */
      else if (streq(funcname, "notdir") && p2[0] && p2[1] == ':')
	{
	  p = p2 + 2;
	  o = variable_buffer_output (o, p, len - (p - p2));
	}
#endif
      else if (is_notdir)
	o = variable_buffer_output (o, p2, len);

      if (is_notdir || p >= p2)
	{
	  o = variable_buffer_output (o, " ", 1);
	  doneany = 1;
	}
    }
  if (doneany)
    /* Kill last space.  */
    --o;


  return o;

}


static char *
func_basename_dir(o, argv, funcname)
     char *o;
     char **argv;
     const char *funcname;
{
  /* Expand the argument.  */
  char *p3 = argv[0];
  char *p2=0;
  int doneany=0;
  int len=0;
  char *p=0;
  int is_basename= streq(funcname, "basename");
  int is_dir= !is_basename;

  while ((p2 = find_next_token (&p3, &len)) != 0)
	{
	  p = p2 + len;
	  while (p >= p2 && (!is_basename  || *p != '.'))
	    {
	      if (IS_PATHSEP(*p))
		break;
	      	    --p;
	    }

	  if (p >= p2 && (is_dir))
	    o = variable_buffer_output (o, p2, ++p - p2);
	  else if (p >= p2 && (*p == '.'))
	    o = variable_buffer_output (o, p2, p - p2);
#if defined(WINDOWS32) || defined(__MSDOS__)
	/* Handle the "d:foobar" case */
	  else if (p2[0] && p2[1] == ':' && is_dir)
	    o = variable_buffer_output (o, p2, 2);
#endif
	  else if (is_dir)
#ifdef VMS
	    o = variable_buffer_output (o, "[]", 2);
#else
#ifndef _AMIGA
	    o = variable_buffer_output (o, "./", 2);
#else
	    ; /* Just a nop...  */
#endif /* AMIGA */
#endif /* !VMS */
	  else
	    /* The entire name is the basename.  */
	    o = variable_buffer_output (o, p2, len);

	  o = variable_buffer_output (o, " ", 1);
	  doneany = 1;
	}
      if (doneany)
	/* Kill last space.  */
	--o;


 return o;
}

static char *
func_addsuffix_addprefix(o, argv, funcname)
     char *o;
     char **argv;
     const char *funcname;
{
  int fixlen = strlen (argv[0]);
  char *list_iterator = argv[1];
  int is_addprefix = streq (funcname, "addprefix");
  int is_addsuffix = !is_addprefix;

  int doneany =0;
  char *p=0;
  int len =0;

  while ((p = find_next_token (&list_iterator, &len)) != 0)
    {
      if (is_addprefix)
	o = variable_buffer_output (o, argv[0], fixlen);
      o = variable_buffer_output (o, p, len);
      if (is_addsuffix)
	o = variable_buffer_output (o, argv[0], fixlen);
      o = variable_buffer_output (o, " ", 1);
      doneany = 1;
    }

  if (doneany)
    /* Kill last space.  */
    --o;

  return o;
}

static char *
func_subst(o, argv, funcname)
     char *o;
     char **argv;
     const char *funcname;
{
  o = subst_expand (o, argv[2], argv[0], argv[1], strlen (argv[0]),
		    strlen (argv[1]), 0, 0);

  return o;
}


static char *
func_firstword(o, argv, funcname)
     char *o;
     char **argv;
     const char *funcname;
{
  int i=0;
  char *words = argv[0];
  char *p = find_next_token (&words, &i);

  if (p != 0)
    o = variable_buffer_output (o, p, i);

  return o;
}


static char *
func_words(o, argv, funcname)
     char *o;
     char **argv;
     const char *funcname;
{
  int i = 0;
  char *word_iterator = argv[0];
  char buf[20];

  while (find_next_token (&word_iterator, (unsigned int *) 0) != 0)
    ++i;

  sprintf (buf, "%d", i);
  o = variable_buffer_output (o, buf, strlen (buf));


  return o;
}

char *
strip_whitespace (begpp, endpp)
     char **begpp;
     char **endpp;
{
  while (isspace (**begpp) && *begpp <= *endpp)
    (*begpp) ++;
  while (isspace (**endpp) && *endpp >= *begpp)
    (*endpp) --;
  return *begpp;
}

int
is_numeric (p)
     char *p;
{
  char *end = p + strlen (p) - 1;
  char *beg = p;
  strip_whitespace (&p, &end);

  while (p <= end)
    if (!ISDIGIT (*(p++)))  /* ISDIGIT only evals its arg once: see make.h.  */
      return 0;

  return (end - beg >= 0);
}

void
check_numeric (s, message)
     char *s;
     char *message;
{
  if (!is_numeric (s))
    fatal (reading_file, message);
}



static char *
func_word(o, argv, funcname)
     char *o;
     char **argv;
     const char *funcname;
{
  char *end_p=0;
  int i=0;
  char *p=0;

  /* Check the first argument.  */
  check_numeric (argv[0], _("non-numeric first argument to `word' function"));
  i =  atoi (argv[0]);

  if (i == 0)
    fatal (reading_file, _("the `word' function takes a positive index argument"));


  end_p = argv[1];
  while ((p = find_next_token (&end_p, 0)) != 0)
    if (--i == 0)
      break;

  if (i == 0)
    o = variable_buffer_output (o, p, end_p - p);

  return o;
}

static char *
func_wordlist (o, argv, funcname)
     char *o;
     char **argv;
     const char *funcname;
{
  int i=0;
  int j=0;

  /* Check the first argument.  */
  check_numeric (argv[0],
		 _("non-numeric first argument to `wordlist' function"));
  i =atoi(argv[0]);
  check_numeric (argv[1],
		 _("non-numeric second argument to `wordlist' function"));

  j = atoi(argv[1]);


  {
    char *p;
    char *end_p = argv[2];

    int start = (i < j) ? i : j;
    int count = j -i ;
    if (count < 0)
      count = - count;
    count ++;



    while (((p = find_next_token (&end_p, 0)) != 0) && --start)
      {}
    if (p)
      {
	while (--count && (find_next_token (&end_p, 0) != 0))
	  {}
	o = variable_buffer_output (o, p, end_p - p);
      }
  }
  return o;
}

static char*
func_findstring(o, argv, funcname)
     char *o;
     char **argv;
     const char *funcname;
{
  /* Find the first occurrence of the first string in the second.  */
  int i = strlen (argv[0]);
  if (sindex (argv[1], 0, argv[0], i) != 0)
    o = variable_buffer_output (o, argv[0], i);

  return o;
}

static char *
func_foreach (o, argv, funcname)
     char *o;
     char **argv;
     const char *funcname;
{
  /* expand only the first two.  */
  char *varname = expand_argument (argv[0], argv[1] - 1);
  char *list = expand_argument (argv[1], argv[2] -1);
  char *body = savestring (argv[2], argv[3] - argv[2] - 1);

  int len =0;
  char *list_iterator = list;
  char *p;
  register struct variable *var=0;
  int doneany =0;

  push_new_variable_scope ();
  var = define_variable (varname, strlen (varname), "", o_automatic, 0);

  /* loop through LIST,  put the value in VAR and expand BODY */
  while ((p = find_next_token (&list_iterator, &len)) != 0)
    {
      char *result = 0;

      {
	char save = p[len];

	p[len] = '\0';
	free (var->value);
	var->value = (char *) xstrdup ((char*) p);
	p[len] = save;
      }

      result = allocated_variable_expand (body);

      o = variable_buffer_output (o, result, strlen (result));
      o = variable_buffer_output (o, " ", 1);
      doneany = 1;
      free (result);
    }

  if (doneany)
    /* Kill the last space.  */
    --o;

  pop_variable_scope ();
  free (varname);
  free (list);
  free (body);

  return o;
}

struct a_word
{
  struct a_word *next;
  char *str;
  int matched;
};

static char *
func_filter_filterout (o, argv, funcname)
     char *o;
     char **argv;
     const char *funcname;
{
  struct  a_word   *wordhead =0;
  struct  a_word *wordtail =0;

  int is_filter = streq (funcname, "filter");
  char *patterns = argv[0];


  char *p;
  int len;
  char *word_iterator = argv[1];

  /* Chop ARGV[1] up into words and then run each pattern through.  */
  while ((p = find_next_token (&word_iterator, &len)) != 0)
    {
      struct a_word *w = (struct a_word *)alloca(sizeof(struct a_word));
      if (wordhead == 0)
	wordhead = w;
      else
	wordtail->next = w;
      wordtail = w;

      if (*word_iterator != '\0')
	++word_iterator;
      p[len] = '\0';
      w->str = p;
      w->matched = 0;
    }

  if (wordhead != 0)
    {
      struct  a_word *wp =0;
      char *pat_iterator = patterns;
      int doneany = 0;

      wordtail->next = 0;


      /* Run each pattern through the words, killing words.  */
      while ((p = find_next_token (&pat_iterator, &len)) != 0)
	{
	  char *percent;
	  char save = p[len];
	  p[len] = '\0';

	  percent = find_percent (p);
	  for (wp = wordhead; wp != 0; wp = wp->next)
	    wp->matched |= (percent == 0 ? streq (p, wp->str)
			    : pattern_matches (p, percent, wp->str));

	  p[len] = save;
	}

      /* Output the words that matched (or didn't, for filter-out).  */
      for (wp = wordhead; wp != 0; wp = wp->next)
	if (is_filter ? wp->matched : !wp->matched)
	  {
	    o = variable_buffer_output (o, wp->str, strlen (wp->str));
	    o = variable_buffer_output (o, " ", 1);
	    doneany = 1;
	  }

      if (doneany)
	/* Kill the last space.  */
	--o;
    }

  return o;
}


static char *
func_strip(o, argv, funcname)
     char *o;
     char **argv;
     const char *funcname;
{
  char *p = argv[0];
  int doneany =0;

  while (*p != '\0')
    {
      int i=0;
      char *word_start=0;

      while (isspace(*p))
	++p;
      word_start = p;
      for (i=0; *p != '\0' && !isspace(*p); ++p, ++i)
	{}
      if (!i)
	break;
      o = variable_buffer_output (o, word_start, i);
      o = variable_buffer_output (o, " ", 1);
      doneany = 1;
    }

  if (doneany)
    /* Kill the last space.  */
    --o;
  return o;
}

/*
  Print a warning or fatal message.
*/
static char *
func_error (o, argv, funcname)
     char *o;
     char **argv;
     const char *funcname;
{
  char **argvp;
  char *msg, *p;
  int len;

  /* The arguments will be broken on commas.  Rather than create yet
     another special case where function arguments aren't broken up,
     just create a format string that puts them back together.  */
  for (len=0, argvp=argv; *argvp != 0; ++argvp)
    len += strlen(*argvp) + 2;

  p = msg = alloca (len + 1);

  for (argvp=argv; argvp[1] != 0; ++argvp)
    {
      strcpy(p, *argvp);
      p += strlen(*argvp);
      *(p++) = ',';
      *(p++) = ' ';
    }
  strcpy(p, *argvp);

  if (*funcname == 'e')
    fatal (reading_file, "%s", msg);

  /* The warning function expands to the empty string.  */
  error (reading_file, "%s", msg);

  return o;
}


/*
  chop argv[0] into words, and sort them.
 */
static char *
func_sort (o, argv, funcname)
     char *o;
     char **argv;
     const char *funcname;
{
  char **words = 0;
  int nwords = 0;
  register int wordi = 0;

  /* Chop ARGV[0] into words and put them in WORDS.  */
  char *t = argv[0];
  char *p=0;
  int len;
  int i;

  while ((p = find_next_token (&t, &len)) != 0)
    {
      if (wordi >= nwords - 1)
	{
	  nwords = 2* nwords + 5;
	  words = (char **) xrealloc ((char *) words,
				      nwords * sizeof (char *));
	}
      words[wordi++] = savestring (p, len);
    }

  if (!wordi)
    return o;

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

  free (words);

  return o;
}

/*
  $(if condition,true-part[,false-part])

  CONDITION is false iff it evaluates to an empty string.  White
  space before and after condition are stripped before evaluation.

  If CONDITION is true, then TRUE-PART is evaluated, otherwise FALSE-PART is
  evaluated (if it exists).  Because only one of the two PARTs is evaluated,
  you can use $(if ...) to create side-effects (with $(shell ...), for
  example).
*/

static char *
func_if (o, argv, funcname)
     char *o;
     char **argv;
     const char *funcname;
{
  char *begp = argv[0];
  char *endp = argv[1]-1;
  int result = 0;

  /* Find the result of the condition: if we have a value, and it's not
     empty, the condition is true.  If we don't have a value, or it's the
     empty string, then it's false.  */

  strip_whitespace (&begp, &endp);

  if (begp < endp)
    {
      char *expansion = expand_argument (begp, endp);

      result = strlen (expansion);
      free (expansion);
    }

  /* If the result is true (1) we want to eval the first argument, and if
     it's false (0) we want to eval the second.  If the argument doesn't
     exist we do nothing, otherwise expand it and add to the buffer.  */

  argv += 1 + !result;

  if (argv[0] != NULL && argv[1] != NULL)
    {
      char *expansion;
      char **endp = argv+1;

      /* If we're doing the else-clause, make sure we concatenate any
         potential extra arguments into the last argument.  */
      if (!result)
        while (*endp && **endp != '\0')
          ++endp;

      expansion = expand_argument (*argv, *endp-1);

      o = variable_buffer_output (o, expansion, strlen (expansion));
      free (expansion);
    }

  return o;
}

static char *
func_wildcard(o, argv, funcname)
     char *o;
     char **argv;
     const char *funcname;
{

#ifdef _AMIGA
   o = wildcard_expansion (argv[0], o);
#else
   char *p = string_glob (argv[0]);
   o = variable_buffer_output (o, p, strlen (p));
#endif
   return o;
}

/*
  \r  is replaced on UNIX as well. Is this desirable?
 */
void
fold_newlines (buffer, length)
     char *buffer;
     int *length;
{
  char *dst = buffer;
  char *src = buffer;
  char *last_nonnl = buffer -1;
  src[*length] = 0;
  for (; *src != '\0'; ++src)
    {
      if (src[0] == '\r' && src[1] == '\n')
	continue;
      if (*src == '\n')
	{
	  *dst++ = ' ';
	}
      else
	{
	  last_nonnl = dst;
	  *dst++ = *src;
	}
    }
  *(++last_nonnl) = '\0';
  *length = last_nonnl - buffer;
}



int shell_function_pid = 0, shell_function_completed;


#ifdef WINDOWS32
/*untested*/

#include <windows.h>
#include <io.h>
#include "sub_proc.h"


void
windows32_openpipe (int *pipedes, int *pid_p, char **command_argv, char **envp)
{
  SECURITY_ATTRIBUTES saAttr;
  HANDLE hIn;
  HANDLE hErr;
  HANDLE hChildOutRd;
  HANDLE hChildOutWr;
  HANDLE hProcess;


  saAttr.nLength = sizeof(SECURITY_ATTRIBUTES);
  saAttr.bInheritHandle = TRUE;
  saAttr.lpSecurityDescriptor = NULL;

  if (DuplicateHandle(GetCurrentProcess(),
		      GetStdHandle(STD_INPUT_HANDLE),
		      GetCurrentProcess(),
		      &hIn,
		      0,
		      TRUE,
		      DUPLICATE_SAME_ACCESS) == FALSE) {
    fatal (NILF, _("create_child_process: DuplicateHandle(In) failed (e=%d)\n"),
	   GetLastError());

  }
  if (DuplicateHandle(GetCurrentProcess(),
		      GetStdHandle(STD_ERROR_HANDLE),
		      GetCurrentProcess(),
		      &hErr,
		      0,
		      TRUE,
		      DUPLICATE_SAME_ACCESS) == FALSE) {
    fatal (NILF, _("create_child_process: DuplicateHandle(Err) failed (e=%d)\n"),
	   GetLastError());
  }

  if (!CreatePipe(&hChildOutRd, &hChildOutWr, &saAttr, 0))
    fatal (NILF, _("CreatePipe() failed (e=%d)\n"), GetLastError());



  hProcess = process_init_fd(hIn, hChildOutWr, hErr);

  if (!hProcess)
    fatal (NILF, _("windows32_openpipe (): process_init_fd() failed\n"));

  else
    process_register(hProcess);

  /* make sure that CreateProcess() has Path it needs */
  sync_Path_environment();

  if (!process_begin(hProcess, command_argv, envp, command_argv[0], NULL))
    *pid_p = (int) hProcess;
  else
    fatal (NILF, _("windows32_openpipe (): unable to launch process (e=%d)\n"),
	   process_last_err(hProcess));

  /* set up to read data from child */
  pipedes[0] = _open_osfhandle((long) hChildOutRd, O_RDONLY);

  /* this will be closed almost right away */
  pipedes[1] = _open_osfhandle((long) hChildOutWr, O_APPEND);
}
#endif


#ifdef __MSDOS__
FILE *
msdos_openpipe (int* pipedes, int *pidp, char *text)
{
  FILE *fpipe=0;
  /* MSDOS can't fork, but it has `popen'.  */
  struct variable *sh = lookup_variable ("SHELL", 5);
  int e;
  extern int dos_command_running, dos_status;

  /* Make sure not to bother processing an empty line.  */
  while (isblank (*text))
    ++text;
  if (*text == '\0')
    return 0;

  if (sh)
    {
      char buf[PATH_MAX + 7];
      /* This makes sure $SHELL value is used by $(shell), even
	 though the target environment is not passed to it.  */
      sprintf (buf, "SHELL=%s", sh->value);
      putenv (buf);
    }

  e = errno;
  errno = 0;
  dos_command_running = 1;
  dos_status = 0;
  /* If dos_status becomes non-zero, it means the child process
     was interrupted by a signal, like SIGINT or SIGQUIT.  See
     fatal_error_signal in commands.c.  */
  fpipe = popen (text, "rt");
  dos_command_running = 0;
  if (!fpipe || dos_status)
    {
      pipedes[0] = -1;
      *pidp = -1;
      if (dos_status)
	errno = EINTR;
      else if (errno == 0)
	errno = ENOMEM;
      shell_function_completed = -1;
    }
  else
    {
      pipedes[0] = fileno (fpipe);
      *pidp = 42; /* Yes, the Meaning of Life, the Universe, and Everything! */
      errno = e;
      shell_function_completed = 1;
    }
  return fpipe;
}
#endif

/*
  Do shell spawning, with the naughty bits for different OSes.
 */

#ifdef VMS

/* VMS can't do $(shell ...)  */
#define func_shell 0

#else
#ifndef _AMIGA
static char *
func_shell (o, argv, funcname)
     char *o;
     char **argv;
     const char *funcname;
{
  char* batch_filename = NULL;
  int i;

#ifdef __MSDOS__
  FILE *fpipe;
#endif
  char **command_argv;
  char *error_prefix;
  char **envp;
  int pipedes[2];
  int pid;

#ifndef __MSDOS__
  /* Construct the argument list.  */
  command_argv = construct_command_argv (argv[0],
					 (char **) NULL, (struct file *) 0,
                                         &batch_filename);
  if (command_argv == 0)
    return o;
#endif

  /* Using a target environment for `shell' loses in cases like:
     export var = $(shell echo foobie)
     because target_environment hits a loop trying to expand $(var)
     to put it in the environment.  This is even more confusing when
     var was not explicitly exported, but just appeared in the
     calling environment.  */

  envp = environ;

  /* For error messages.  */
  if (reading_file != 0)
    {
      error_prefix = (char *) alloca (strlen(reading_file->filenm)+11+4);
      sprintf (error_prefix,
	       "%s:%lu: ", reading_file->filenm, reading_file->lineno);
    }
  else
    error_prefix = "";

#ifdef WINDOWS32
  windows32_openpipe (pipedes, &pid, command_argv, envp);
#else /* WINDOWS32 */

# ifdef __MSDOS__
  fpipe = msdos_openpipe (pipedes, &pid, argv[0]);
  if (pipedes[0] < 0)
    {
      perror_with_name (error_prefix, "pipe");
      return o;
    }
# else
  if (pipe (pipedes) < 0)
    {
      perror_with_name (error_prefix, "pipe");
      return o;
    }

  pid = vfork ();
  if (pid < 0)
    perror_with_name (error_prefix, "fork");
  else if (pid == 0)
    child_execute_job (0, pipedes[1], command_argv, envp);
  else
# endif /* ! __MSDOS__ */

#endif /* WINDOWS32 */
    {
      /* We are the parent.  */

      char *buffer;
      unsigned int maxlen;
      int cc;

      /* Record the PID for reap_children.  */
      shell_function_pid = pid;
#ifndef  __MSDOS__
      shell_function_completed = 0;

      /* Free the storage only the child needed.  */
      free (command_argv[0]);
      free ((char *) command_argv);

      /* Close the write side of the pipe.  */
      (void) close (pipedes[1]);
#endif

      /* Set up and read from the pipe.  */

      maxlen = 200;
      buffer = (char *) xmalloc (maxlen + 1);

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
      while (cc > 0 || EINTR_SET);

      /* Close the read side of the pipe.  */
#ifdef  __MSDOS__
      if (fpipe)
	(void) pclose (fpipe);
#else
      (void) close (pipedes[0]);
#endif

      /* Loop until child_handler sets shell_function_completed
	 to the status of our child shell.  */
      while (shell_function_completed == 0)
	reap_children (1, 0);

      if (batch_filename) {
	if (debug_flag)
	  printf(_("Cleaning up temporary batch file %s\n"), batch_filename);
	remove(batch_filename);
	free(batch_filename);
      }
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
	  fold_newlines (buffer, &i);
	  o = variable_buffer_output (o, buffer, i);
	}

      free (buffer);
    }

  return o;
}

#else	/* _AMIGA */

/* Do the Amiga version of func_shell.  */

static char *
func_shell (char *o, char **argv, const char *funcname)
{
  /* Amiga can't fork nor spawn, but I can start a program with
     redirection of my choice.  However, this means that we
     don't have an opportunity to reopen stdout to trap it.  Thus,
     we save our own stdout onto a new descriptor and dup a temp
     file's descriptor onto our stdout temporarily.  After we
     spawn the shell program, we dup our own stdout back to the
     stdout descriptor.  The buffer reading is the same as above,
     except that we're now reading from a file.  */

#include <dos/dos.h>
#include <proto/dos.h>

  BPTR child_stdout;
  char tmp_output[FILENAME_MAX];
  unsigned int maxlen = 200;
  int cc, i;
  char * buffer, * ptr;
  char ** aptr;
  int len = 0;
  char* batch_filename = NULL;

  /* Construct the argument list.  */
  command_argv = construct_command_argv (argv[0], (char **) NULL,
                                         (struct file *) 0, &batch_filename);
  if (command_argv == 0)
    return o;


  strcpy (tmp_output, "t:MakeshXXXXXXXX");
  mktemp (tmp_output);
  child_stdout = Open (tmp_output, MODE_NEWFILE);

  for (aptr=command_argv; *aptr; aptr++)
    len += strlen (*aptr) + 1;

  buffer = xmalloc (len + 1);
  ptr = buffer;

  for (aptr=command_argv; *aptr; aptr++)
    {
      strcpy (ptr, *aptr);
      ptr += strlen (ptr) + 1;
      *ptr ++ = ' ';
      *ptr = 0;
    }

  ptr[-1] = '\n';

  Execute (buffer, NULL, child_stdout);
  free (buffer);

  Close (child_stdout);

  child_stdout = Open (tmp_output, MODE_OLDFILE);

  buffer = xmalloc (maxlen);
  i = 0;
  do
    {
      if (i == maxlen)
	{
	  maxlen += 512;
	  buffer = (char *) xrealloc (buffer, maxlen + 1);
	}

      cc = Read (child_stdout, &buffer[i], maxlen - i);
      if (cc > 0)
	i += cc;
    } while (cc > 0);

  Close (child_stdout);

  fold_newlines (buffer, &i);
  o = variable_buffer_output (o, buffer, i);
  free (buffer);
  return o;
}
#endif  /* _AMIGA */
#endif  /* !VMS */

#ifdef EXPERIMENTAL

/*
  equality. Return is string-boolean, ie, the empty string is false.
 */
static char *
func_eq (char* o, char **argv, char *funcname)
{
  int result = ! strcmp (argv[0], argv[1]);
  o = variable_buffer_output (o,  result ? "1" : "", result);
  return o;
}


/*
  string-boolean not operator.
 */
static char *
func_not (char* o, char **argv, char *funcname)
{
  char * s = argv[0];
  int result = 0;
  while (isspace (*s))
    s++;
  result = ! (*s);
  o = variable_buffer_output (o,  result ? "1" : "", result);
  return o;
}
#endif


#define STRING_SIZE_TUPLE(_s) (_s), (sizeof(_s)-1)

/* Lookup table for builtin functions.

   This doesn't have to be sorted; we use a straight lookup.  We might gain
   some efficiency by moving most often used functions to the start of the
   table.

   If REQUIRED_ARGS is positive, the function takes exactly that many
   arguments.  All subsequent text is included with the last argument.  So,
   since $(sort a,b,c) takes only one argument, it will be the full string
   "a,b,c".  If the value is negative, it's the minimum number of arguments.
   A function can have more, but if it has less an error is generated.

   EXPAND_ARGS means that all arguments should be expanded before invocation.
   Functions that do namespace tricks (foreach) don't automatically expand.  */

static char *func_call PARAMS((char *o, char **argv, const char *funcname));


static struct function_table_entry function_table[] =
{
 /* Name/size */                    /* ARG EXP? Function */
  { STRING_SIZE_TUPLE("addprefix"),     2,  1,  func_addsuffix_addprefix},
  { STRING_SIZE_TUPLE("addsuffix"),     2,  1,  func_addsuffix_addprefix},
  { STRING_SIZE_TUPLE("basename"),      1,  1,  func_basename_dir},
  { STRING_SIZE_TUPLE("dir"),           1,  1,  func_basename_dir},
  { STRING_SIZE_TUPLE("notdir"),        1,  1,  func_notdir_suffix},
  { STRING_SIZE_TUPLE("subst"),         3,  1,  func_subst},
  { STRING_SIZE_TUPLE("suffix"),        1,  1,  func_notdir_suffix},
  { STRING_SIZE_TUPLE("filter"),        2,  1,  func_filter_filterout},
  { STRING_SIZE_TUPLE("filter-out"),    2,  1,  func_filter_filterout},
  { STRING_SIZE_TUPLE("findstring"),    2,  1,  func_findstring},
  { STRING_SIZE_TUPLE("firstword"),     1,  1,  func_firstword},
  { STRING_SIZE_TUPLE("join"),          2,  1,  func_join},
  { STRING_SIZE_TUPLE("patsubst"),      3,  1,  func_patsubst},
  { STRING_SIZE_TUPLE("shell"),         1,  1,  func_shell},
  { STRING_SIZE_TUPLE("sort"),          1,  1,  func_sort},
  { STRING_SIZE_TUPLE("strip"),         1,  1,  func_strip},
  { STRING_SIZE_TUPLE("wildcard"),      1,  1,  func_wildcard},
  { STRING_SIZE_TUPLE("word"),          2,  1,  func_word},
  { STRING_SIZE_TUPLE("wordlist"),      3,  1,  func_wordlist},
  { STRING_SIZE_TUPLE("words"),         1,  1,  func_words},
  { STRING_SIZE_TUPLE("origin"),        1,  1,  func_origin},
  { STRING_SIZE_TUPLE("foreach"),       3,  0,  func_foreach},
  { STRING_SIZE_TUPLE("call"),         -1,  1,  func_call},
  { STRING_SIZE_TUPLE("error"),         1,  1,  func_error},
  { STRING_SIZE_TUPLE("warning"),       1,  1,  func_error},
  { STRING_SIZE_TUPLE("if"),           -2,  0,  func_if},
#ifdef EXPERIMENTAL
  { STRING_SIZE_TUPLE("eq"),            2,  1,  func_eq},
  { STRING_SIZE_TUPLE("not"),           1,  1,  func_not},
#endif
  { 0 }
};


/* These must come after the definition of function_table[].  */

static char *
expand_builtin_function (o, argc, argv, entry_p)
     char *o;
     int argc;
     char **argv;
     struct function_table_entry *entry_p;
{
  int min = (entry_p->required_args > 0
             ? entry_p->required_args
             : -entry_p->required_args);

  if (argc < min)
    fatal (reading_file,
           _("Insufficient number of arguments (%d) to function `%s'"),
           argc, entry_p->name);

  if (!entry_p->func_ptr)
    fatal (reading_file, _("Unimplemented on this platform: function `%s'"),
           entry_p->name);

  return entry_p->func_ptr (o, argv, entry_p->name);
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
  const struct function_table_entry *entry_p;
  char openparen = (*stringp)[0];
  char closeparen = openparen == '(' ? ')' : '}';
  char *beg = *stringp + 1;
  char *endref;
  int count = 0;
  char *argbeg;
  register char *p;
  char **argv, **argvp;
  int nargs;

  entry_p = lookup_function (function_table, beg);

  if (!entry_p)
    return 0;

  /* We have found a call to a builtin function.  Find the end of the
     arguments, and do the function.  */

  endref = beg + entry_p->len;

  /* Space after function name isn't part of the args.  */
  p = next_token (endref);
  argbeg = p;

  /* Find the end of the function invocation, counting nested use of
     whichever kind of parens we use.  Since we're looking, count commas
     to get a rough estimate of how many arguments we might have.  The
     count might be high, but it'll never be low.  */

  for (nargs=1; *p != '\0'; ++p)
    if (*p == ',')
      ++nargs;
    else if (*p == openparen)
      ++count;
    else if (*p == closeparen && --count < 0)
      break;

  if (count >= 0)
    fatal (reading_file,
	   _("unterminated call to function `%s': missing `%c'"),
	   entry_p->name, closeparen);

  /* Get some memory to store the arg pointers.  */
  argvp = argv = (char **) alloca (sizeof(char *) * (nargs + 2));

  /* Chop the string into arguments, then store the end pointer and a nul.
     If REQUIRED_ARGS is positive, as soon as we hit that many assume the
     rest of the string is part of the last argument.  */
  *argvp = argbeg;
  nargs = 1;
  while (entry_p->required_args < 0 || nargs < entry_p->required_args)
    {
      char *next = find_next_argument (openparen, closeparen, *argvp, p);

      if (!next)
	break;

      *(++argvp) = next+1;
      ++nargs;
    }

  *(++argvp) = p+1;
  *(++argvp) = 0;

  /* If we should expand, do it.  */
  if (entry_p->expand_args)
    {
      for (argvp=argv; argvp[1] != 0; ++argvp)
	*argvp = expand_argument (*argvp, argvp[1]-1);

      /* end pointer doesn't make sense for expanded stuff.  */
      *argvp = 0;
    }

  /* Finally!  Run the function...  */
  *op = expand_builtin_function (*op, nargs, argv, entry_p);

  /* If we allocated memory for the expanded args, free it again.  */
  if (entry_p->expand_args)
    for (argvp=argv; *argvp != 0; ++argvp)
      free (*argvp);

  *stringp = p;

  return 1;
}


/* User-defined functions.  Expand the first argument as either a builtin
   function or a make variable, in the context of the rest of the arguments
   assigned to $1, $2, ... $N.  $0 is the name of the function.  */

static char *
func_call (o, argv, funcname)
     char *o;
     char **argv;
     const char *funcname;
{
  char *fname;
  int flen;
  char *body;
  int i;
  const struct function_table_entry *entry_p;

  /* Calling nothing is a no-op.  */
  if (*argv[0] == '\0')
    return o;

  /* There is no way to define a variable with a space in the name, so strip
     trailing whitespace as a favor to the user.  */

  flen = strlen (argv[0]);
  fname = argv[0] + flen - 1;
  while (isspace (*fname))
    --fname;
  fname[1] = '\0';

  flen = fname - argv[0] + 1;
  fname = argv[0];

  /* Are we invoking a builtin function?  */

  entry_p = lookup_function (function_table, fname);

  if (entry_p)
    {
      for (i=0; argv[i+1]; ++i)
	;

      return expand_builtin_function (o, i, argv + 1, entry_p);
    }

  /* No, so the first argument is the name of a variable to be expanded and
     interpreted as a function.  Create the variable reference.  */
  body = alloca (flen + 4);
  body[0]='$';
  body[1]='(';
  strcpy (body + 2, fname);
  body[flen+2]=')';
  body[flen+3]= '\0';

  /* Set up arguments $(1) .. $(N).  $(0) is the function name.  */

  push_new_variable_scope ();

  for (i=0; *argv; ++i, ++argv)
    {
      char num[11];

      sprintf (num, "%d", i);
      define_variable (num, strlen (num), *argv, o_automatic, 0);
    }

  /* Expand the body in the context of the arguments, adding the result to
     the variable buffer.  */

  o = variable_expand_string (o, body, flen+3);

  pop_variable_scope ();

  return o + strlen(o);
}
