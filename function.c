/* Variable function expansion for GNU Make.
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
the Free Software Foundation, 675 Mass Ave, Cambridge, MA 02139, USA.  */

#include "make.h"
#include "filedef.h"
#include "variable.h"
#include "dep.h"
#include "job.h"
#include "commands.h"

#ifdef _AMIGA
#include "amiga.h"
#endif
#ifdef WINDOWS32
#include <windows.h>
#include <io.h>
#include "sub_proc.h"
#endif

static char *string_glob PARAMS ((char *line));

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
  unsigned int replace_prepercent_len, replace_postpercent_len;
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
	      || strncmp (t + 1, pattern + 1, pattern_prepercent_len - 1)))
	fail = 1;

      /* Does the suffix match? */
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
    function_wordlist,
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
    { "wordlist", 8, function_wordlist },
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
#define BADARGS(func)                                                         \
  if (reading_filename != 0)                                                  \
    makefile_fatal (reading_filename, *reading_lineno_ptr,                    \
		    "insufficient arguments to function `%s'",                \
		    func);                                                    \
  else                                                                        \
    fatal ("insufficient arguments to function `%s'", func)

static char *
expand_function (o, function, text, end)
     char *o;
     enum function function;
     char *text;
     char *end;
{
  char *p, *p2, *p3;
  unsigned int i, j, len;
  int doneany = 0;
  int count;
  char endparen = *end, startparen = *end == ')' ? '(' : '{';

  switch (function)
    {
    default:
      abort ();
      break;

#ifndef VMS /* not supported for vms yet */
    case function_shell:
      {
#ifdef WINDOWS32
	SECURITY_ATTRIBUTES saAttr;
	HANDLE hIn;
	HANDLE hErr;
	HANDLE hChildOutRd;
	HANDLE hChildOutWr;
	HANDLE hProcess;
#endif
#ifdef __MSDOS__
	FILE *fpipe;
#endif
	char **argv;
	char *error_prefix;
#ifndef _AMIGA
	char **envp;
	int pipedes[2];
	int pid;
#endif

	/* Expand the command line.  */
	text = expand_argument (text, end);

#ifndef __MSDOS__
	/* Construct the argument list.  */
	argv = construct_command_argv (text,
				       (char **) NULL, (struct file *) 0);
	if (argv == 0)
	  break;
#endif

#ifndef _AMIGA
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
#endif  /* Not Amiga.  */

	/* For error messages.  */
	if (reading_filename != 0)
	  {
	    error_prefix = (char *) alloca (strlen (reading_filename) + 100);
	    sprintf (error_prefix,
		     "%s:%u: ", reading_filename, *reading_lineno_ptr);
	  }
	else
	  error_prefix = "";

#ifndef _AMIGA
# ifdef WINDOWS32
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
	  fatal("create_child_process: DuplicateHandle(In) failed (e=%d)\n",
		GetLastError());
	}
	if (DuplicateHandle(GetCurrentProcess(),
			    GetStdHandle(STD_ERROR_HANDLE),
			    GetCurrentProcess(),
			    &hErr,
			    0,
			    TRUE,
			    DUPLICATE_SAME_ACCESS) == FALSE) {
	  fatal("create_child_process: DuplicateHandle(Err) failed (e=%d)\n",
		GetLastError());
	}

	if (!CreatePipe(&hChildOutRd, &hChildOutWr, &saAttr, 0))
	  fatal("CreatePipe() failed (e=%d)\n", GetLastError());

	hProcess = process_init_fd(hIn, hChildOutWr, hErr);

	if (!hProcess)
	  fatal("expand_function: process_init_fd() failed\n");
	else
	  process_register(hProcess);

	/* make sure that CreateProcess() has Path it needs */
	sync_Path_environment();

	if (!process_begin(hProcess, argv, envp, argv[0], NULL))
		pid = (int) hProcess;
	else
		fatal("expand_function: unable to launch process (e=%d)\n",
		      process_last_err(hProcess));

	/* set up to read data from child */
	pipedes[0] = _open_osfhandle((long) hChildOutRd, O_RDONLY);

	/* this will be closed almost right away */
	pipedes[1] = _open_osfhandle((long) hChildOutWr, O_APPEND);
# else /* WINDOWS32 */
#  ifdef __MSDOS__
	{
	  /* MSDOS can't fork, but it has `popen'.
	     (Bwt, why isn't `popen' used in all the versions?) */
	  struct variable *sh = lookup_variable ("SHELL", 5);
	  int e;
	  extern int dos_command_running, dos_status;

	  /* Make sure not to bother processing an empty line.  */
	  while (isblank (*text))
	    ++text;
	  if (*text == '\0')
	    break;

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
	  fpipe = popen (text, "rt");
	  dos_command_running = 0;
	  if (!fpipe || dos_status)
	    {
	      pipedes[0] = -1;
	      pid = -1;
	      if (dos_status)
		errno = EINTR;
	      else if (errno == 0)
		errno = ENOMEM;
	      shell_function_completed = -1;
	    }
	  else
	    {
	      pipedes[0] = fileno (fpipe);
	      pid = 42;
	      errno = e;
	      shell_function_completed = 1;
	    }
	}
	if (pipedes[0] < 0)
#  else /* ! __MSDOS__ */
	if (pipe (pipedes) < 0)
#  endif /* __MSDOS__ */
	  {
	    perror_with_name (error_prefix, "pipe");
	    break;
	  }

#  ifndef  __MSDOS__
	pid = vfork ();
	if (pid < 0)
	  perror_with_name (error_prefix, "fork");
	else if (pid == 0)
	  child_execute_job (0, pipedes[1], argv, envp);
	else
#  endif /* ! __MSDOS__ */
# endif /* WINDOWS32 */
	  {
	    /* We are the parent.  */

	    char *buffer;
	    unsigned int maxlen;
	    int cc;
#if 0
	    for (i = 0; envp[i] != 0; ++i)
	      free (envp[i]);
	    free ((char *) envp);
#endif

	    /* Record the PID for reap_children.  */
	    shell_function_pid = pid;
#ifndef  __MSDOS__
	    shell_function_completed = 0;

	    /* Free the storage only the child needed.  */
	    free (argv[0]);
	    free ((char *) argv);

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
#ifdef EINTR
	    while (cc > 0 || errno == EINTR);
#else
	    while (cc > 0);
#endif

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
		      {
			if (i > 1 && buffer[i - 2] == '\r')
			  --i;
			buffer[--i] = '\0';
		      }
		    else
		      buffer[i] = '\0';

		    p = buffer;
		    for (p2=p; *p != '\0'; ++p)
		      {
			if (p[0] == '\r' && p[1] == '\n')
			  continue;
			if (*p == '\n')
			  *p2++ = ' ';
			else
			  *p2++ = *p;
		      }
		    *p2 = '\0';
		    o = variable_buffer_output (o, buffer, i);
		  }
	      }

	    free (buffer);
	  }
#else   /* Amiga */
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
	   int cc;
	   char * buffer, * ptr;
	   char ** aptr;
	   int len = 0;

	   strcpy (tmp_output, "t:MakeshXXXXXXXX");
	   mktemp (tmp_output);
	   child_stdout = Open (tmp_output, MODE_NEWFILE);

	   for (aptr=argv; *aptr; aptr++)
	     {
	       len += strlen (*aptr) + 1;
	     }

	   buffer = xmalloc (len + 1);
	   ptr = buffer;

	   for (aptr=argv; *aptr; aptr++)
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
	   DeleteFile (tmp_output);

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
	   free (buffer);
	 }
#endif  /* Not Amiga.  */

	free (text);
	break;
      }
#endif /* !VMS */

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
      while (*p2 != '\0')
	{
	  while (isspace(*p2))
	    ++p2;
	  p = p2;
	  for (i=0; *p2 != '\0' && !isspace(*p2); ++p2, ++i)
	    {}
	  if (!i)
	    break;
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

#ifdef _AMIGA
      o = wildcard_expansion (text, o);
#else
      p = string_glob (text);
      o = variable_buffer_output (o, p, strlen (p));
#endif

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

    case function_wordlist:
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
	BADARGS ("wordlist");
      text = expand_argument (text, p);

      /* Check the first argument.  */
      for (p2 = text; *p2 != '\0'; ++p2)
	if (*p2 < '0' || *p2 > '9')
	  {
	    if (reading_filename != 0)
	      makefile_fatal (reading_filename, *reading_lineno_ptr,
			      "non-numeric first argument to `wordlist' function");
	    else
	      fatal ("non-numeric first argument to `wordlist' function");
	  }
      i = (unsigned int)atoi(text);
      free (text);

      /* Check the next argument */
      for (p2 = p + 1; isblank(*p2); ++p2)
	{}
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
	BADARGS ("wordlist");
      text = expand_argument (p2, p);

      for (p2 = text; *p2 != '\0'; ++p2)
	if (*p2 < '0' || *p2 > '9')
	  {
	    if (reading_filename != 0)
	      makefile_fatal (reading_filename, *reading_lineno_ptr,
			      "non-numeric second argument to `wordlist' function");
	    else
	      fatal ("non-numeric second argument to `wordlist' function");
	  }
      j = (unsigned int)atoi(text);
      free (text);

      if (j > i)
	j -= i;
      else
	{
	  unsigned int k;
	  k = j;
	  j = i - j;
	  i = k;
	}
      ++j;

      /* Extract the requested words */
      text = expand_argument (p + 1, end);
      p2 = text;

      while (((p = find_next_token (&p2, &len)) != 0) && --i)
	{}
      if (p)
	{
	  while (--j && (find_next_token (&p2, &len) != 0))
	    {}
	  o = variable_buffer_output (o, p, p2 - p);
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
#ifdef VMS
	  while (p >= p2 && *p != ']'
		 && (function != function_basename || *p != '.'))
#else
# ifdef __MSDOS__
	  while (p >= p2 && *p != '/' && *p != '\\'
		 && (function != function_basename || *p != '.'))
# else
	  while (p >= p2 && *p != '/'
		 && (function != function_basename || *p != '.'))
# endif
#endif
	    --p;
	  if (p >= p2 && (function == function_dir))
	    o = variable_buffer_output (o, p2, ++p - p2);
	  else if (p >= p2 && (*p == '.'))
	    o = variable_buffer_output (o, p2, p - p2);
#if defined(WINDOWS32) || defined(__MSDOS__)
	/* Handle the "d:foobar" case */
	  else if (p2[0] && p2[1] == ':' && function == function_dir)
	    o = variable_buffer_output (o, p2, 2);
#endif
	  else if (function == function_dir)
#ifdef VMS
	    o = variable_buffer_output (o, "[]", 2);
#else
#ifndef _AMIGA
	    o = variable_buffer_output (o, "./", 2);
#else
	    /* o = o */; /* Just a nop...  */
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
#ifdef VMS
	  while (p >= p2 && *p != ']'
		 && (function != function_suffix || *p != '.'))
#else
# ifdef __MSDOS__
	  while (p >= p2 && *p != '/' && *p != '\\'
		 && (function != function_suffix || *p != '.'))
# else
	  while (p >= p2 && *p != '/'
		 && (function != function_suffix || *p != '.'))
# endif
#endif
	    --p;
	  if (p >= p2)
	    {
	      if (function == function_notdir)
		++p;
	      else if (*p != '.')
		continue;
	      o = variable_buffer_output (o, p, len - (p - p2));
	    }
#if defined(WINDOWS32) || defined(__MSDOS__)
	  /* Handle the case of "d:foo/bar".  */
	  else if (function == function_notdir && p2[0] && p2[1] == ':')
	    {
	      p = p2 + 2;
	      o = variable_buffer_output (o, p, len - (p - p2));
	    }
#endif
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
