/* Internals of variables for GNU Make.
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
#include "commands.h"
#include "variable.h"
#include "dep.h"
#include "file.h"

#ifdef	__GNUC__
#define	max(a, b) \
  ({ register int __a = (a), __b = (b); __a > __b ? __a : __b; })
#else
#define max(a, b) ((a) > (b) ? (a) : (b))
#endif


/* Hash table of all global variable definitions.  */

#ifndef	VARIABLE_BUCKETS
#define VARIABLE_BUCKETS		523
#endif
#ifndef	PERFILE_VARIABLE_BUCKETS
#define	PERFILE_VARIABLE_BUCKETS	23
#endif
#ifndef	SMALL_SCOPE_VARIABLE_BUCKETS
#define	SMALL_SCOPE_VARIABLE_BUCKETS	13
#endif
static struct variable *variable_table[VARIABLE_BUCKETS];
static struct variable_set global_variable_set
  = { variable_table, VARIABLE_BUCKETS };
static struct variable_set_list global_setlist
  = { 0, &global_variable_set };
struct variable_set_list *current_variable_set_list = &global_setlist;

/* The next two describe the variable output buffer.
   This buffer is used to hold the variable-expansion of a line of the
   makefile.  It is made bigger with realloc whenever it is too small.
   variable_buffer_length is the size currently allocated.
   variable_buffer is the address of the buffer.  */

static unsigned int variable_buffer_length;
static char *variable_buffer;

/* Implement variables.  */

/* Define variable named NAME with value VALUE in SET.  VALUE is copied.
   LENGTH is the length of NAME, which does not need to be null-terminated.
   ORIGIN specifies the origin of the variable (makefile, command line
   or environment).
   If RECURSIVE is nonzero a flag is set in the variable saying
   that it should be recursively re-expanded.  */

static struct variable *
define_variable_in_set (name, length, value, origin, recursive, set)
     char *name;
     unsigned int length;
     char *value;
     enum variable_origin origin;
     int recursive;
     struct variable_set *set;
{
  register unsigned int i;
  register unsigned int hashval;
  register struct variable *v;

  hashval = 0;
  for (i = 0; i < length; ++i)
    HASH (hashval, name[i]);
  hashval %= set->buckets;

  for (v = set->table[hashval]; v != 0; v = v->next)
    if (*v->name == *name
	&& !strncmp (v->name + 1, name + 1, length - 1)
	&& v->name[length] == '\0')
      break;

  if (env_overrides && origin == o_env)
    origin = o_env_override;

  if (v != 0)
    {
      if (env_overrides && v->origin == o_env)
	/* V came from in the environment.  Since it was defined
	   before the switches were parsed, it wasn't affected by -e.  */
	v->origin = o_env_override;

      /* A variable of this name is already defined.
	 If the old definition is from a stronger source
	 than this one, don't redefine it.  */
      if ((int) origin >= (int) v->origin)
	{
	  v->value = savestring (value, strlen (value));
	  v->origin = origin;
	  v->recursive = recursive;
	}
      return v;
    }

  /* Create a new variable definition and add it to the hash table.  */

  v = (struct variable *) xmalloc (sizeof (struct variable));
  v->name = savestring (name, length);
  v->value = savestring (value, strlen (value));
  v->origin = origin;
  v->recursive = recursive;
  v->expanding = 0;
  v->next = set->table[hashval];
  set->table[hashval] = v;
  return v;
}

/* Define a variable in the current variable set.  */

struct variable *
define_variable (name, length, value, origin, recursive)
     char *name;
     unsigned int length;
     char *value;
     enum variable_origin origin;
     int recursive;
{
  return define_variable_in_set (name, length, value, origin, recursive,
				 current_variable_set_list->set);
}

/* Define a variable in FILE's variable set.  */

struct variable *
define_variable_for_file (name, length, value, origin, recursive, file)
     char *name;
     unsigned int length;
     char *value;
     enum variable_origin origin;
     int recursive;
     struct file *file;
{
  return define_variable_in_set (name, length, value, origin, recursive,
				 file->variables->set);
}

/* Lookup a variable whose name is a string starting at NAME
   and with LENGTH chars.  NAME need not be null-terminated.
   Returns address of the `struct variable' containing all info
   on the variable, or nil if no such variable is defined.  */

struct variable *
lookup_variable (name, length)
     char *name;
     unsigned int length;
{
  register struct variable_set_list *setlist;

  register unsigned int i;
  register unsigned int rawhash = 0;

  for (i = 0; i < length; ++i)
    HASH (rawhash, name[i]);

  for (setlist = current_variable_set_list;
       setlist != 0; setlist = setlist->next)
    {
      register struct variable_set *set = setlist->set;
      register unsigned int hashval = rawhash % set->buckets;
      register struct variable *v;

      for (v = set->table[hashval]; v != 0; v = v->next)
	if (*v->name == *name
	    && !strncmp (v->name + 1, name + 1, length - 1)
	    && v->name[length] == 0)
	  return v;
    }

  return 0;
}

/* Initialize FILE's variable set list.  If FILE already has a variable set
   list, the topmost variable set is left intact, but the the rest of the
   chain is replaced with FILE->parent's setlist.  */

void
initialize_file_variables (file)
     struct file *file;
{
  register struct variable_set_list *l = file->variables;
  if (l == 0)
    {
      l = (struct variable_set_list *)
	xmalloc (sizeof (struct variable_set_list));
      l->set = (struct variable_set *) xmalloc (sizeof (struct variable_set));
      l->set->buckets = PERFILE_VARIABLE_BUCKETS;
      l->set->table = (struct variable **)
	xmalloc (l->set->buckets * sizeof (struct variable *));
      bzero ((char *) l->set->table,
	     l->set->buckets * sizeof (struct variable *));
      file->variables = l;
    }

  if (file->parent == 0)
    l->next = &global_setlist;
  else
    {
      if (file->parent->variables == 0)
	initialize_file_variables (file->parent);
      l->next = file->parent->variables;
    }
}

/* Pop the top set off the current variable set list,
   and free all its storage.  */

void
pop_variable_scope ()
{
  register struct variable_set_list *setlist = current_variable_set_list;
  register struct variable_set *set = setlist->set;
  register unsigned int i;

  current_variable_set_list = setlist->next;
  free ((char *) setlist);

  for (i = 0; i < set->buckets; ++i)
    {
      register struct variable *next = set->table[i];
      while (next != 0)
	{
	  register struct variable *v = next;
	  next = v->next;

	  free (v->name);
	  free ((char *) v);
	}
    }
  free ((char *) set->table);
  free ((char *) set);
}

/* Create a new variable set and push it on the current setlist.  */

void
push_new_variable_scope ()
{
  register struct variable_set_list *setlist;
  register struct variable_set *set;

  set = (struct variable_set *) xmalloc (sizeof (struct variable_set));
  set->buckets = SMALL_SCOPE_VARIABLE_BUCKETS;
  set->table = (struct variable **)
    xmalloc (set->buckets * sizeof (struct variable *));
  bzero ((char *) set->table, set->buckets * sizeof (struct variable *));

  setlist = (struct variable_set_list *)
    xmalloc (sizeof (struct variable_set_list));
  setlist->set = set;
  setlist->next = current_variable_set_list;
  current_variable_set_list = setlist;
}

/* Merge SET1 into SET0, freeing unused storage in SET1.  */

static void
merge_variable_sets (set0, set1)
     struct variable_set *set0, *set1;
{
  register unsigned int bucket1;

  for (bucket1 = 0; bucket1 < set1->buckets; ++bucket1)
    {
      register struct variable *v1 = set1->table[bucket1];
      while (v1 != 0)
	{
	  struct variable *next = v1->next;
	  unsigned int bucket0;
	  register struct variable *v0;

	  if (set1->buckets >= set0->buckets)
	    bucket0 = bucket1;
	  else
	    {
	      register char *n;
	      bucket0 = 0;
	      for (n = v1->name; *n != '\0'; ++n)
		HASH (bucket0, *n);
	    }
	  bucket0 %= set0->buckets;

	  for (v0 = set0->table[bucket0]; v0 != 0; v0 = v0->next)
	    if (streq (v0->name, v1->name))
	      break;

	  if (v0 == 0)
	    {
	      /* There is no variable in SET0 with the same name.  */
	      v1->next = set0->table[bucket0];
	      set0->table[bucket0] = v1;
	    }
	  else
	    {
	      /* The same variable exists in both sets.
		 SET0 takes precedence.  */
	      free (v1->value);
	      free ((char *) v1);
	    }

	  v1 = next;
	}
    }
}

/* Merge SETLIST1 into SETLIST0, freeing unused storage in SETLIST1.  */

void
merge_variable_set_lists (setlist0, setlist1)
     struct variable_set_list **setlist0, *setlist1;
{
  register struct variable_set_list *list0 = *setlist0;
  struct variable_set_list *last0 = 0;

  while (setlist1 != 0 && list0 != 0)
    {
      struct variable_set_list *next = setlist1;
      setlist1 = setlist1->next;

      merge_variable_sets (list0->set, next->set);

      free ((char *) next);

      last0 = list0;
      list0 = list0->next;
    }

  if (setlist1 != 0)
    {
      if (last0 == 0)
	*setlist0 = setlist1;
      else
	last0->next = setlist1;
    }
}

/* Define the automatic variables, and record the addresses
   of their structures so we can change their values quickly.  */

void
define_automatic_variables ()
{
  extern char default_shell[];
  register struct variable *v;
  char buf[100];

  sprintf (buf, "%u", makelevel);
  (void) define_variable ("MAKELEVEL", 9, buf, o_env, 0);

  /* This won't override any definition, but it
     will provide one if there isn't one there.  */
  v = define_variable ("SHELL", 5, default_shell, o_default, 0);

  /* Don't let SHELL come from the environment
     if MAKELEVEL is 0.  Also, SHELL must not be empty.  */
  if (*v->value == '\0' || (v->origin == o_env && makelevel == 0))
    {
      v->origin = o_file;
      v->value = savestring ("/bin/sh", 7);
    }
}

/* Subroutine of variable_expand and friends:
   The text to add is LENGTH chars starting at STRING to the variable_buffer.
   The text is added to the buffer at PTR, and the updated pointer into
   the buffer is returned as the value.  Thus, the value returned by
   each call to variable_buffer_output should be the first argument to
   the following call.  */

char *
variable_buffer_output (ptr, string, length)
     char *ptr, *string;
     unsigned int length;
{
  register unsigned int newlen = length + (ptr - variable_buffer);

  if (newlen > variable_buffer_length)
    {
      unsigned int offset = ptr - variable_buffer;
      variable_buffer_length = max (2 * variable_buffer_length, newlen + 100);
      variable_buffer = (char *) xrealloc (variable_buffer,
					   variable_buffer_length);
      ptr = variable_buffer + offset;
    }

  bcopy (string, ptr, length);
  return ptr + length;
}

/* Return a pointer to the beginning of the variable buffer.  */

char *
initialize_variable_output ()
{
  /* If we don't have a variable output buffer yet, get one.  */

  if (variable_buffer == 0)
    {
      variable_buffer_length = 200;
      variable_buffer = (char *) xmalloc (variable_buffer_length);
    }

  return variable_buffer;
}

/* Create a new environment for FILE's commands.
   The child's MAKELEVEL variable is incremented.  */

char **
target_environment (file)
     struct file *file;
{
  register struct variable_set_list *s;
  struct variable_bucket
    {
      struct variable_bucket *next;
      struct variable *variable;
    };
  struct variable_bucket **table;
  unsigned int buckets;
  register unsigned int i;
  register unsigned nvariables;
  char **result;

  int noexport = enter_file (".NOEXPORT")->is_target;

  /* Find the lowest number of buckets in any set in the list.  */
  s = file->variables;
  buckets = s->set->buckets;
  for (s = s->next; s != 0; s = s->next)
    if (s->set->buckets < buckets)
      buckets = s->set->buckets;

  /* Temporarily allocate a table with that many buckets.  */
  table = (struct variable_bucket **)
    alloca (buckets * sizeof (struct variable_bucket *));
  bzero ((char *) table, buckets * sizeof (struct variable_bucket *));

  /* Run through all the variable sets in the list,
     accumulating variables in TABLE.  */
  nvariables = 0;
  for (s = file->variables; s != 0; s = s->next)
    {
      register struct variable_set *set = s->set;
      for (i = 0; i < set->buckets; ++i)
	{
	  register struct variable *v;
	  for (v = set->table[i]; v != 0; v = v->next)
	    {
	      extern char *getenv ();
	      unsigned int j = i % buckets;
	      register struct variable_bucket *ov;
	      register char *p = v->name;

	      /* If `.NOEXPORT' was specified, only export command-line and
		 environment variables.  This is a temporary (very ugly) hack
		 until I fix this problem the right way in version 4.  Ick.  */
	      if (noexport
		  && (v->origin != o_command
		      && v->origin != o_env && v->origin != o_env_override
		      && !(v->origin == o_file && getenv (p) != 0)))
		continue;

	      if (v->origin == o_default
		  || streq (p, "MAKELEVEL"))
		continue;

	      if (*p != '_' && (*p < 'A' || *p > 'Z')
		  && (*p < 'a' || *p > 'z'))
		continue;
	      for (++p; *p != '\0'; ++p)
		if (*p != '_' && (*p < 'a' || *p > 'z')
		    && (*p < 'A' || *p > 'Z') && (*p < '0' || *p > '9'))
		  break;
	      if (*p != '\0')
		continue;

	      for (ov = table[j]; ov != 0; ov = ov->next)
		if (streq (v->name, ov->variable->name))
		  break;
	      if (ov == 0)
		{
		  register struct variable_bucket *entry;
		  entry = (struct variable_bucket *)
		    alloca (sizeof (struct variable_bucket));
		  entry->next = table[j];
		  entry->variable = v;
		  table[j] = entry;
		  ++nvariables;
		}
	    }
	}
    }

  result = (char **) xmalloc ((nvariables + 2) * sizeof (char *));
  nvariables = 0;
  for (i = 0; i < buckets; ++i)
    {
      register struct variable_bucket *b;
      for (b = table[i]; b != 0; b = b->next)
	{
	  register struct variable *v = b->variable;
	  result[nvariables++] = concat (v->name, "=", v->value);
	}
    }
  result[nvariables] = (char *) xmalloc (100);
  (void) sprintf (result[nvariables], "MAKELEVEL=%u", makelevel + 1);
  result[++nvariables] = 0;

  return result;
}

/* Try to interpret LINE (a null-terminated string)
   as a variable definition.  If it is one, define the
   variable and return 1.  Otherwise return 0.

   ORIGIN may be o_file, o_override, o_env, o_env_override,
   or o_command specifying that the variable definition comes
   from a makefile, an override directive, the environment with
   or without the -e switch, or the command line.

   A variable definition has the form "name = value" or "name := value".
   Any whitespace around the "=" or ":=" is removed.  The first form
   defines a variable that is recursively re-evaluated.  The second form
   defines a variable whose value is variable-expanded at the time of
   definition and then is evaluated only once at the time of expansion.  */

int
try_variable_definition (line, origin)
     char *line;
     enum variable_origin origin;
{
  register int c;
  register char *p = line;
  register char *beg;
  register char *end;
  register int recursive;

  if (*p == '\t')
    return 0;
  while (1)
    {
      c = *p++;
      if (c == '\0' || c == '#')
	return 0;
      if (c == '=')
	{
	  recursive = 1;
	  break;
	}
      else if (c == ':')
	if (*p == '=')
	  {
	    ++p;
	    recursive = 0;
	    break;
	  }
	else
	  return 0;
    }

  beg = next_token (line);
  end = p - 1;
  if (!recursive)
    --end;
  while (isblank (end[-1]))
    --end;
  p = next_token (p);

  (void) define_variable (beg, end - beg, recursive ? p : variable_expand (p),
			  origin, recursive);

  return 1;
}

/* Print information for variable V, prefixing it with PREFIX.  */

static void
print_variable (v, prefix)
     register struct variable *v;
     char *prefix;
{
  char *origin;

  switch (v->origin)
    {
    case o_default:
      origin = "default";
      break;
    case o_env:
      origin = "environment";
      break;
    case o_file:
      origin = "makefile";
      break;
    case o_env_override:
      origin = "environment under -e";
      break;
    case o_command:
      origin = "command line";
      break;
    case o_override:
      origin = "`override' directive";
      break;
    case o_automatic:
      origin = "automatic";
      break;
    case o_invalid:
    default:
      abort ();
      break;
    }
  printf ("# %s\n", origin);

  fputs (prefix, stdout);

  /* Is this a `define'?  */
  if (v->recursive && index (v->value, '\n') != 0)
    printf ("define %s\n%s\nendef\n", v->name, v->value);
  else
    {
      register char *p;

      printf ("%s %s= ", v->name, v->recursive ? "" : ":");

      /* Check if the value is just whitespace.  */
      p = next_token (v->value);
      if (p != v->value && *p == '\0')
	/* All whitespace.  */
	printf ("$(subst ,,%s)", v->value);
      else if (v->recursive)
	fputs (v->value, stdout);
      else
	/* Double up dollar signs.  */
	for (p = v->value; *p != '\0'; ++p)
	  {
	    if (*p == '$')
	      putchar ('$');
	    putchar (*p);
	  }
      putchar ('\n');
    }
}


/* Print all the variables in SET.  PREFIX is printed before
   the actual variable definitions (everything else is comments).  */

static void
print_variable_set (set, prefix)
     register struct variable_set *set;
     char *prefix;
{
  register unsigned int i, nvariables, per_bucket;
  register struct variable *v;

  per_bucket = nvariables = 0;
  for (i = 0; i < set->buckets; ++i)
    {
      register unsigned int this_bucket = 0;

      for (v = set->table[i]; v != 0; v = v->next)
	{
	  ++this_bucket;
	  print_variable (v, prefix);
	}

      nvariables += this_bucket;
      if (this_bucket > per_bucket)
	per_bucket = this_bucket;
    }

  if (nvariables == 0)
    puts ("# No variables.");
  else
    {
      printf ("# %u variables in %u hash buckets.\n",
	      nvariables, set->buckets);
#ifndef	NO_FLOAT
      printf ("# average of %.1f variables per bucket, \
max %u in one bucket.\n",
	      ((double) nvariables) * 100.0 / (double) set->buckets,
	      per_bucket);
#endif
    }
}


/* Print the data base of variables.  */

void
print_variable_data_base ()
{
  puts ("\n# Variables\n");

  print_variable_set (&global_variable_set, "");
}


/* Print all the local variables of FILE.  */

void
print_file_variables (file)
     struct file *file;
{
  if (file->variables != 0)
    print_variable_set (file->variables->set, "# ");
}

struct output_state
  {
    char *buffer;
    unsigned int length;
  };

/* Save the current variable output state and return a pointer
   to storage describing it.  Then reset the output state.  */

char *
save_variable_output ()
{
  struct output_state *state;

  state = (struct output_state *) xmalloc (sizeof (struct output_state));
  state->buffer = variable_buffer;
  state->length = variable_buffer_length;

  variable_buffer = 0;
  variable_buffer_length = 0;

  return (char *) state;
}

/* Restore the variable output state saved in SAVE.  */

void
restore_variable_output (save)
     char *save;
{
  register struct output_state *state = (struct output_state *) save;

  if (variable_buffer != 0)
    free (variable_buffer);

  variable_buffer = state->buffer;
  variable_buffer_length = state->length;

  free ((char *) state);
}
