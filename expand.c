/* Variable expansion functions for GNU Make.
Copyright (C) 1988, 1989, 1991 Free Software Foundation, Inc.
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
#include "file.h"
#include "variable.h"


/* Recursively expand V.  The returned string is malloc'd.  */

static char *
recursively_expand (v)
     register struct variable *v;
{
  char *value;

  if (v->expanding)
    {
      /* Expanding V causes infinite recursion.  Lose.  */
      if (reading_filename == 0)
	fatal ("Recursive variable `%s' references itself (eventually)",
	       v->name);
      else
	makefile_fatal
	  (reading_filename, *reading_lineno_ptr, 
	   "Recursive variable `%s' references itself (eventually)",
	   v->name);
    }

  v->expanding = 1;
  value = allocated_variable_expand (v->value);
  v->expanding = 0;

  return value;
}

/* Scan LINE for variable references and expansion-function calls.
   Build in `variable_buffer' the result of expanding the references and calls.
   Return the address of the resulting string, which is null-terminated
   and is valid only until the next time this function is called.  */

char *
variable_expand (line)
     register char *line;
{
  register struct variable *v;
  register char *p, *o, *p1;

  p = line;
  o = initialize_variable_output ();

  while (1)
    {
      /* Copy all following uninteresting chars all at once to the
         variable output buffer, and skip them.  Uninteresting chars end
	 at the next $ or the end of the input.  */

      p1 = index (p, '$');

      o = variable_buffer_output (o, p, p1 != 0 ? p1 - p : strlen (p) + 1);

      if (p1 == 0)
	break;
      p = p1 + 1;

      /* Dispatch on the char that follows the $.  */

      switch (*p)
	{
	case '$':
	  /* $$ seen means output one $ to the variable output buffer.  */
	  o = variable_buffer_output (o, p, 1);
	  break;

	case '(':
	case '{':
	  /* $(...) or ${...} is the general case of substitution.  */
	  {
	    char openparen = *p;
	    char closeparen = (openparen == '(') ? ')' : '}';
	    register char *beg = p + 1;
	    char *op, *begp;
	    char *end;

	    op = o;
	    begp = p;
	    if (handle_function (&op, &begp))
	      {
		o = op;
		p = begp;
		break;
	      }

	    /* Is there a variable reference inside the parens or braces?
	       If so, expand it before expanding the entire reference.  */

	    p1 = index (beg, closeparen);
	    if (p1 != 0)
	      p1 = lindex (beg, p1, '$');
	    if (p1 != 0)
	      {
		/* BEG now points past the opening paren or brace.
		   Count parens or braces until it is matched.  */
		int count = 0;
		for (p = beg; *p != '\0'; ++p)
		  {
		    if (*p == openparen)
		      ++count;
		    else if (*p == closeparen && --count < 0)
		      break;
		  }
		/* If count is >= 0, there were unmatched opening parens
		   or braces, so we go to the simple case of a variable name
		   such as `$($(a)'.  */
		if (count < 0)
		  {
		    char *name = expand_argument (beg, p);
		    static char start[3] = { '$', }, end[2];
		    start[1] = openparen;
		    end[0] = closeparen;
		    p1 = concat (start, name, end);
		    free (name);
		    name = allocated_variable_expand (p1);
		    o = variable_buffer_output (o, name, strlen (name));
		    free (name);
		    break;
		  }
	      }

	    /* This is not a reference to a built-in function and
	       it does not contain any variable references inside.
	       There are several things it could be.  */

	    p = index (beg, ':');
	    if (p != 0 && lindex (beg, p, closeparen) == 0)
	      {
		/* This is a substitution reference: $(FOO:A=B).  */
		int count;
		char *subst_beg, *replace_beg;
		unsigned int subst_len, replace_len;

		v = lookup_variable (beg, p - beg);

		subst_beg = p + 1;
		count = 0;
		for (p = subst_beg; *p != '\0'; ++p)
		  {
		    if (*p == openparen)
		      ++count;
		    else if (*p == closeparen)
		      --count;
		    else if (*p == '=' && count <= 0)
		      break;
		  }
		if (count > 0)
		  /* There were unmatched opening parens.  */
		  return initialize_variable_output ();
		subst_len = p - subst_beg;

		replace_beg = p + 1;
		count = 0;
		for (p = replace_beg; *p != '\0'; ++p)
		  {
		    if (*p == openparen)
		      ++count;
		    else if (*p == closeparen && --count < 0)
		      break;
		  }
		if (count > 0)
		  /* There were unmatched opening parens.  */
		  return initialize_variable_output ();
		end = p;
		replace_len = p - replace_beg;

		if (v != 0 && *v->value != '\0')
		  {
		    char *value = (v->recursive ? recursively_expand (v)
				   : v->value);
		    if (lindex (subst_beg, subst_beg + subst_len, '%') != 0)
		      {
			p = savestring (subst_beg, subst_len);
			p1 = savestring (replace_beg, replace_len);
			o = patsubst_expand (o, value, p, p1,
					     index (p, '%'), index (p1, '%'));
			free (p);
			free (p1);
		      }
		    else
		      o = subst_expand (o, value, subst_beg, replace_beg,
					subst_len, replace_len, 0, 1);
		    if (v->recursive)
		      free (value);
		  }
	      }

	    /* No, this must be an ordinary variable reference.  */
	    else
	      {
		/* Look up the value of the variable.  */
		end = index (beg, closeparen);
		if (end == 0)
		  return initialize_variable_output ();
		v = lookup_variable (beg, end - beg);

		if (v != 0 && *v->value != '\0')
		  {
		    char *value = (v->recursive ? recursively_expand (v)
				   : v->value);
		    o = variable_buffer_output (o, value, strlen (value));
		    if (v->recursive)
		      free (value);
		  }
	      }

	    /* Advance p past the variable reference to resume scan.  */
	    p = end;
	  }
	  break;

	case '\0':
	  break;

	default:
	  if (isblank (p[-1]))
	    break;

	  /* A $ followed by a random char is a variable reference:
	     $a is equivalent to $(a).  */
	  {
	    /* We could do the expanding here, but this way
	       avoids code repetition at a small performance cost.  */
	    char name[5];
	    name[0] = '$';
	    name[1] = '(';
	    name[2] = *p;
	    name[3] = ')';
	    name[4] = '\0';
	    p1 = allocated_variable_expand (name);
	    o = variable_buffer_output (o, p1, strlen (p1));
	    free (p1);
	  }

	  break;
	}      

      if (*p == '\0')
	break;
      else
	++p;
    }

  (void) variable_buffer_output (o, "", 1);
  return initialize_variable_output ();
}

/* Expand an argument for an expansion function.
   The text starting at STR and ending at END is variable-expanded
   into a null-terminated string that is returned as the value.
   This is done without clobbering `variable_buffer' or the current
   variable-expansion that is in progress.  */

char *
expand_argument (str, end)
     char *str, *end;
{
  char *tmp = savestring (str, end - str);
  char *value = allocated_variable_expand (tmp);

  free (tmp);

  return value;
}

/* Expand LINE for FILE.  Error messages refer to the file and line where
   FILE's commands were found.  Expansion uses FILE's variable set list.  */

char *
variable_expand_for_file (line, file)
     char *line;
     register struct file *file;
{
  char *result;
  struct variable_set_list *save;

  if (file == 0)
    return variable_expand (line);

  save = current_variable_set_list;
  current_variable_set_list = file->variables;
  reading_filename = file->cmds->filename;
  reading_lineno_ptr = &file->cmds->lineno;
  result = variable_expand (line);
  current_variable_set_list = save;
  reading_filename = 0;
  reading_lineno_ptr = 0;

  return result;
}

/* Like variable_expand, but the returned string is malloc'd.  */
char *
allocated_variable_expand (line)
     char *line;
{
  return allocated_variable_expand_for_file (line, (struct file *) 0);
}

/* Like variable_expand_for_file, but the returned string is malloc'd.  */

char *
allocated_variable_expand_for_file (line, file)
     char *line;
     struct file *file;
{
  char *save;
  char *value;

  save = save_variable_output ();

  value = variable_expand_for_file (line, file);
  value = savestring (value, strlen (value));

  restore_variable_output (save);

  return value;
}
