/* Internals of variables for GNU Make.
Copyright (C) 1988,89,90,91,92,93,94,96,97 Free Software Foundation, Inc.
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
#include "dep.h"
#include "filedef.h"
#include "job.h"
#include "commands.h"
#include "variable.h"
#ifdef WINDOWS32
#include "pathstuff.h"
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

static struct variable *lookup_variable_in_set PARAMS ((char *name,
                          unsigned int length, struct variable_set *set));

/* Implement variables.  */

/* Define variable named NAME with value VALUE in SET.  VALUE is copied.
   LENGTH is the length of NAME, which does not need to be null-terminated.
   ORIGIN specifies the origin of the variable (makefile, command line
   or environment).
   If RECURSIVE is nonzero a flag is set in the variable saying
   that it should be recursively re-expanded.  */

struct variable *
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
	&& strneq (v->name + 1, name + 1, length - 1)
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
	  if (v->value != 0)
	    free (v->value);
	  v->value = xstrdup (value);
	  v->origin = origin;
	  v->recursive = recursive;
	}
      return v;
    }

  /* Create a new variable definition and add it to the hash table.  */

  v = (struct variable *) xmalloc (sizeof (struct variable));
  v->name = savestring (name, length);
  v->value = xstrdup (value);
  v->origin = origin;
  v->recursive = recursive;
  v->expanding = 0;
  v->per_target = 0;
  v->export = v_default;
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
	    && strneq (v->name + 1, name + 1, length - 1)
	    && v->name[length] == 0)
	  return v;
    }

  return 0;
}

/* Lookup a variable whose name is a string starting at NAME
   and with LENGTH chars in set SET.  NAME need not be null-terminated.
   Returns address of the `struct variable' containing all info
   on the variable, or nil if no such variable is defined.  */

static struct variable *
lookup_variable_in_set (name, length, set)
     char *name;
     unsigned int length;
     struct variable_set *set;
{
  register unsigned int i;
  register unsigned int hash = 0;
  register struct variable *v;

  for (i = 0; i < length; ++i)
    HASH (hash, name[i]);
  hash %= set->buckets;

  for (v = set->table[hash]; v != 0; v = v->next)
    if (*v->name == *name
        && strneq (v->name + 1, name + 1, length - 1)
        && v->name[length] == 0)
      return v;

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
	  if (v->value)
	    free (v->value);
	  free ((char *) v);
	}
    }
  free ((char *) set->table);
  free ((char *) set);
}

struct variable_set_list *
create_new_variable_set ()
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

  return setlist;
}

/* Create a new variable set and push it on the current setlist.  */

struct variable_set_list *
push_new_variable_scope ()
{
  return (current_variable_set_list = create_new_variable_set());
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
#ifdef WINDOWS32
  extern char* default_shell;
#else
  extern char default_shell[];
#endif
  register struct variable *v;
  char buf[200];

  sprintf (buf, "%u", makelevel);
  (void) define_variable ("MAKELEVEL", 9, buf, o_env, 0);

  sprintf (buf, "%s%s%s",
	   version_string,
	   (remote_description == 0 || remote_description[0] == '\0')
	   ? "" : "-",
	   (remote_description == 0 || remote_description[0] == '\0')
	   ? "" : remote_description);
  (void) define_variable ("MAKE_VERSION", 12, buf, o_default, 0);

#ifdef  __MSDOS__
  /* Allow to specify a special shell just for Make,
     and use $COMSPEC as the default $SHELL when appropriate.  */
  {
    static char shell_str[] = "SHELL";
    const int shlen = sizeof (shell_str) - 1;
    struct variable *mshp = lookup_variable ("MAKESHELL", 9);
    struct variable *comp = lookup_variable ("COMSPEC", 7);

    /* Make $MAKESHELL override $SHELL even if -e is in effect.  */
    if (mshp)
      (void) define_variable (shell_str, shlen,
			      mshp->value, o_env_override, 0);
    else if (comp)
      {
	/* $COMSPEC shouldn't override $SHELL.  */
	struct variable *shp = lookup_variable (shell_str, shlen);

	if (!shp)
	  (void) define_variable (shell_str, shlen, comp->value, o_env, 0);
      }
  }
#endif

  /* This won't override any definition, but it
     will provide one if there isn't one there.  */
  v = define_variable ("SHELL", 5, default_shell, o_default, 0);
  v->export = v_export;		/* Always export SHELL.  */

  /* On MSDOS we do use SHELL from environment, since
     it isn't a standard environment variable on MSDOS,
     so whoever sets it, does that on purpose.  */
#ifndef __MSDOS__
  /* Don't let SHELL come from the environment.  */
  if (*v->value == '\0' || v->origin == o_env || v->origin == o_env_override)
    {
      free (v->value);
      v->origin = o_file;
      v->value = xstrdup (default_shell);
    }
#endif

  /* Make sure MAKEFILES gets exported if it is set.  */
  v = define_variable ("MAKEFILES", 9, "", o_default, 0);
  v->export = v_ifset;

  /* Define the magic D and F variables in terms of
     the automatic variables they are variations of.  */

  define_variable ("@D", 2, "$(patsubst %/,%,$(dir $@))", o_automatic, 1);
  define_variable ("%D", 2, "$(patsubst %/,%,$(dir $%))", o_automatic, 1);
  define_variable ("*D", 2, "$(patsubst %/,%,$(dir $*))", o_automatic, 1);
  define_variable ("<D", 2, "$(patsubst %/,%,$(dir $<))", o_automatic, 1);
  define_variable ("?D", 2, "$(patsubst %/,%,$(dir $?))", o_automatic, 1);
  define_variable ("^D", 2, "$(patsubst %/,%,$(dir $^))", o_automatic, 1);
  define_variable ("+D", 2, "$(patsubst %/,%,$(dir $+))", o_automatic, 1);
  define_variable ("@F", 2, "$(notdir $@)", o_automatic, 1);
  define_variable ("%F", 2, "$(notdir $%)", o_automatic, 1);
  define_variable ("*F", 2, "$(notdir $*)", o_automatic, 1);
  define_variable ("<F", 2, "$(notdir $<)", o_automatic, 1);
  define_variable ("?F", 2, "$(notdir $?)", o_automatic, 1);
  define_variable ("^F", 2, "$(notdir $^)", o_automatic, 1);
  define_variable ("+F", 2, "$(notdir $+)", o_automatic, 1);
}

int export_all_variables;

/* Create a new environment for FILE's commands.
   If FILE is nil, this is for the `shell' function.
   The child's MAKELEVEL variable is incremented.  */

char **
target_environment (file)
     struct file *file;
{
  struct variable_set_list *set_list;
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
  unsigned int mklev_hash;

  if (file == 0)
    set_list = current_variable_set_list;
  else
    set_list = file->variables;

  /* Find the lowest number of buckets in any set in the list.  */
  s = set_list;
  buckets = s->set->buckets;
  for (s = s->next; s != 0; s = s->next)
    if (s->set->buckets < buckets)
      buckets = s->set->buckets;

  /* Find the hash value of the bucket `MAKELEVEL' will fall into.  */
  {
    char *p = "MAKELEVEL";
    mklev_hash = 0;
    while (*p != '\0')
      HASH (mklev_hash, *p++);
  }

  /* Temporarily allocate a table with that many buckets.  */
  table = (struct variable_bucket **)
    alloca (buckets * sizeof (struct variable_bucket *));
  bzero ((char *) table, buckets * sizeof (struct variable_bucket *));

  /* Run through all the variable sets in the list,
     accumulating variables in TABLE.  */
  nvariables = 0;
  for (s = set_list; s != 0; s = s->next)
    {
      register struct variable_set *set = s->set;
      for (i = 0; i < set->buckets; ++i)
	{
	  register struct variable *v;
	  for (v = set->table[i]; v != 0; v = v->next)
	    {
	      unsigned int j = i % buckets;
	      register struct variable_bucket *ov;
	      register char *p = v->name;

	      if (i == mklev_hash % set->buckets
		  && streq (v->name, "MAKELEVEL"))
		/* Don't include MAKELEVEL because it will be
		   added specially at the end.  */
		continue;

              /* If this is a per-target variable and it hasn't been touched
                 already then look up the global version and take its export
                 value.  */
              if (v->per_target && v->export == v_default)
                {
                  struct variable *gv;

                  gv = lookup_variable_in_set(v->name, strlen(v->name),
                                              &global_variable_set);
                  if (gv)
                    v->export = gv->export;
                }

	      switch (v->export)
		{
		case v_default:
		  if (v->origin == o_default || v->origin == o_automatic)
		    /* Only export default variables by explicit request.  */
		    continue;

		  if (! export_all_variables
		      && v->origin != o_command
		      && v->origin != o_env && v->origin != o_env_override)
		    continue;

		  if (*p != '_' && (*p < 'A' || *p > 'Z')
		      && (*p < 'a' || *p > 'z'))
		    continue;
		  for (++p; *p != '\0'; ++p)
		    if (*p != '_' && (*p < 'a' || *p > 'z')
			&& (*p < 'A' || *p > 'Z') && (*p < '0' || *p > '9'))
		      continue;
		  if (*p != '\0')
		    continue;
		  break;

                case v_export:
                  break;

                case v_noexport:
                  continue;

		case v_ifset:
		  if (v->origin == o_default)
		    continue;
		  break;
		}

              /* If this was from a different-sized hash table, then
                 recalculate the bucket it goes in.  */
              if (set->buckets != buckets)
                {
                  register char *np;

                  j = 0;
                  for (np = v->name; *np != '\0'; ++np)
                    HASH (j, *np);
                  j %= buckets;
                }

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

	  /* If V is recursively expanded and didn't come from the environment,
	     expand its value.  If it came from the environment, it should
	     go back into the environment unchanged.  */
	  if (v->recursive
	      && v->origin != o_env && v->origin != o_env_override)
	    {
	      char *value = recursively_expand (v);
#ifdef WINDOWS32
              if (strcmp(v->name, "Path") == 0 ||
                  strcmp(v->name, "PATH") == 0)
                convert_Path_to_windows32(value, ';');
#endif
	      result[nvariables++] = concat (v->name, "=", value);
	      free (value);
	    }
	  else
#ifdef WINDOWS32
          {
            if (strcmp(v->name, "Path") == 0 ||
                strcmp(v->name, "PATH") == 0)
              convert_Path_to_windows32(v->value, ';');
            result[nvariables++] = concat (v->name, "=", v->value);
          }
#else
	    result[nvariables++] = concat (v->name, "=", v->value);
#endif
	}
    }
  result[nvariables] = (char *) xmalloc (100);
  (void) sprintf (result[nvariables], "MAKELEVEL=%u", makelevel + 1);
  result[++nvariables] = 0;

  return result;
}

/* Try to interpret LINE (a null-terminated string) as a variable definition.

   ORIGIN may be o_file, o_override, o_env, o_env_override,
   or o_command specifying that the variable definition comes
   from a makefile, an override directive, the environment with
   or without the -e switch, or the command line.

   See the comments for parse_variable_definition().

   If LINE was recognized as a variable definition, a pointer to its `struct
   variable' is returned.  If LINE is not a variable definition, NULL is
   returned.  */

struct variable *
try_variable_definition (flocp, line, origin)
     const struct floc *flocp;
     char *line;
     enum variable_origin origin;
{
  register int c;
  register char *p = line;
  register char *beg;
  register char *end;
  enum { f_bogus,
         f_simple, f_recursive, f_append, f_conditional } flavor = f_bogus;
  char *name, *expanded_name, *value, *alloc_value=NULL;
  struct variable *v;

  while (1)
    {
      c = *p++;
      if (c == '\0' || c == '#')
	return 0;
      if (c == '=')
	{
	  end = p - 1;
	  flavor = f_recursive;
	  break;
	}
      else if (c == ':')
	if (*p == '=')
	  {
	    end = p++ - 1;
	    flavor = f_simple;
	    break;
	  }
	else
	  /* A colon other than := is a rule line, not a variable defn.  */
	  return 0;
      else if (c == '+' && *p == '=')
	{
	  end = p++ - 1;
	  flavor = f_append;
	  break;
	}
      else if (c == '?' && *p == '=')
        {
          end = p++ - 1;
          flavor = f_conditional;
          break;
        }
      else if (c == '$')
	{
	  /* This might begin a variable expansion reference.  Make sure we
	     don't misrecognize chars inside the reference as =, := or +=.  */
	  char closeparen;
	  int count;
	  c = *p++;
	  if (c == '(')
	    closeparen = ')';
	  else if (c == '{')
	    closeparen = '}';
	  else
	    continue;		/* Nope.  */

	  /* P now points past the opening paren or brace.
	     Count parens or braces until it is matched.  */
	  count = 0;
	  for (; *p != '\0'; ++p)
	    {
	      if (*p == c)
		++count;
	      else if (*p == closeparen && --count < 0)
		{
		  ++p;
		  break;
		}
	    }
	}
    }

  beg = next_token (line);
  while (end > beg && isblank (end[-1]))
    --end;
  p = next_token (p);

  /* Expand the name, so "$(foo)bar = baz" works.  */
  name = (char *) alloca (end - beg + 1);
  bcopy (beg, name, end - beg);
  name[end - beg] = '\0';
  expanded_name = allocated_variable_expand (name);

  if (expanded_name[0] == '\0')
    fatal (flocp, _("empty variable name"));

  /* Calculate the variable's new value in VALUE.  */

  switch (flavor)
    {
    case f_bogus:
      /* Should not be possible.  */
      abort ();
    case f_simple:
      /* A simple variable definition "var := value".  Expand the value.
         We have to allocate memory since otherwise it'll clobber the
	 variable buffer, and we may still need that if we're looking at a
         target-specific variable.  */
      value = alloc_value = allocated_variable_expand (p);
      break;
    case f_conditional:
      /* A conditional variable definition "var ?= value".
         The value is set IFF the variable is not defined yet. */
      v = lookup_variable(expanded_name, strlen(expanded_name));
      if (v)
        {
          free(expanded_name);
          return v;
        }
      flavor = f_recursive;
      /* FALLTHROUGH */
    case f_recursive:
      /* A recursive variable definition "var = value".
	 The value is used verbatim.  */
      value = p;
      break;
    case f_append:
      /* An appending variable definition "var += value".
	 Extract the old value and append the new one.  */
      v = lookup_variable (expanded_name, strlen (expanded_name));
      if (v == 0)
	{
	  /* There was no old value.
	     This becomes a normal recursive definition.  */
	  value = p;
	  flavor = f_recursive;
	}
      else
	{
	  /* Paste the old and new values together in VALUE.  */

	  unsigned int oldlen, newlen;

	  if (v->recursive)
	    /* The previous definition of the variable was recursive.
	       The new value comes from the unexpanded old and new values.  */
	    flavor = f_recursive;
	  else
	    /* The previous definition of the variable was simple.
	       The new value comes from the old value, which was expanded
	       when it was set; and from the expanded new value.  Allocate
               memory for the expansion as we may still need the rest of the
               buffer if we're looking at a target-specific variable.  */
	    p = alloc_value = allocated_variable_expand (p);

	  oldlen = strlen (v->value);
	  newlen = strlen (p);
	  value = (char *) alloca (oldlen + 1 + newlen + 1);
	  bcopy (v->value, value, oldlen);
	  value[oldlen] = ' ';
	  bcopy (p, &value[oldlen + 1], newlen + 1);
	}
    }

#ifdef __MSDOS__
  /* Many Unix Makefiles include a line saying "SHELL=/bin/sh", but
     non-Unix systems don't conform to this default configuration (in
     fact, most of them don't even have `/bin').  On the other hand,
     $SHELL in the environment, if set, points to the real pathname of
     the shell.
     Therefore, we generally won't let lines like "SHELL=/bin/sh" from
     the Makefile override $SHELL from the environment.  But first, we
     look for the basename of the shell in the directory where SHELL=
     points, and along the $PATH; if it is found in any of these places,
     we define $SHELL to be the actual pathname of the shell.  Thus, if
     you have bash.exe installed as d:/unix/bash.exe, and d:/unix is on
     your $PATH, then SHELL=/usr/local/bin/bash will have the effect of
     defining SHELL to be "d:/unix/bash.exe".  */
  if ((origin == o_file || origin == o_override)
      && strcmp (expanded_name, "SHELL") == 0)
    {
      char shellpath[PATH_MAX];
      extern char * __dosexec_find_on_path (const char *, char *[], char *);

      /* See if we can find "/bin/sh.exe", "/bin/sh.com", etc.  */
      if (__dosexec_find_on_path (value, (char **)0, shellpath))
	{
	  char *p;

	  for (p = shellpath; *p; p++)
	    {
	      if (*p == '\\')
		*p = '/';
	    }
	  v = define_variable (expanded_name, strlen (expanded_name),
			       shellpath, origin, flavor == f_recursive);
	}
      else
	{
	  char *shellbase, *bslash;
	  struct variable *pathv = lookup_variable ("PATH", 4);
	  char *path_string;
	  char *fake_env[2];
	  size_t pathlen = 0;

	  shellbase = rindex (value, '/');
	  bslash = rindex (value, '\\');
	  if (!shellbase || bslash > shellbase)
	    shellbase = bslash;
	  if (!shellbase && value[1] == ':')
	    shellbase = value + 1;
	  if (shellbase)
	    shellbase++;
	  else
	    shellbase = value;

	  /* Search for the basename of the shell (with standard
	     executable extensions) along the $PATH.  */
	  if (pathv)
	    pathlen = strlen (pathv->value);
	  path_string = (char *)xmalloc (5 + pathlen + 2 + 1);
	  /* On MSDOS, current directory is considered as part of $PATH.  */
	  sprintf (path_string, "PATH=.;%s", pathv ? pathv->value : "");
	  fake_env[0] = path_string;
	  fake_env[1] = (char *)0;
	  if (__dosexec_find_on_path (shellbase, fake_env, shellpath))
	    {
	      char *p;

	      for (p = shellpath; *p; p++)
		{
		  if (*p == '\\')
		    *p = '/';
		}
	      v = define_variable (expanded_name, strlen (expanded_name),
				   shellpath, origin, flavor == f_recursive);
	    }
	  else
	    v = lookup_variable (expanded_name, strlen (expanded_name));

	  free (path_string);
	}
    }
  else
#endif /* __MSDOS__ */
#ifdef WINDOWS32
  if ((origin == o_file || origin == o_override)
      && strcmp (expanded_name, "SHELL") == 0) {
    extern char* default_shell;

    /*
     * Call shell locator function. If it returns TRUE, then
	 * set no_default_sh_exe to indicate sh was found and
     * set new value for SHELL variable.
	 */
    if (find_and_set_default_shell(value)) {
       v = define_variable (expanded_name, strlen (expanded_name),
                            default_shell, origin, flavor == f_recursive);
       no_default_sh_exe = 0;
    }
  } else
#endif

  v = define_variable (expanded_name, strlen (expanded_name),
		       value, origin, flavor == f_recursive);

  if (alloc_value)
    free (alloc_value);
  free (expanded_name);

  return v;
}

/* Print information for variable V, prefixing it with PREFIX.  */

static void
print_variable (v, prefix)
     register struct variable *v;
     char *prefix;
{
  const char *origin;

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

void
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
    puts (_("# No variables."));
  else
    {
      printf (_("# %u variables in %u hash buckets.\n"),
	      nvariables, set->buckets);
#ifndef	NO_FLOAT
      printf (_("# average of %.1f variables per bucket, \
max %u in one bucket.\n"),
	      (double) nvariables / (double) set->buckets,
	      per_bucket);
#else
      {
	int f = (nvariables * 1000 + 5) / set->buckets;
	printf (_("# average of %d.%d variables per bucket, \
max %u in one bucket.\n"),
	      f/10, f%10,
	      per_bucket);
      }
#endif
    }
}


/* Print the data base of variables.  */

void
print_variable_data_base ()
{
  puts (_("\n# Variables\n"));

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

#ifdef WINDOWS32
void
sync_Path_environment(void)
{
    char* path = allocated_variable_expand("$(Path)");
    static char* environ_path = NULL;

    if (!path)
        return;

    /*
     * If done this before, don't leak memory unnecessarily.
     * Free the previous entry before allocating new one.
     */
    if (environ_path)
        free(environ_path);

    /*
     * Create something WINDOWS32 world can grok
     */
    convert_Path_to_windows32(path, ';');
    environ_path = concat("Path", "=", path);
    putenv(environ_path);
    free(path);
}
#endif
