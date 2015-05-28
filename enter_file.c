/* Target file management for GNU Make.
Copyright (C) 1988-2014 Free Software Foundation, Inc.
This file is part of GNU Make.

GNU Make is free software; you can redistribute it and/or modify it under the
terms of the GNU General Public License as published by the Free Software
Foundation; either version 3 of the License, or (at your option) any later
version.

GNU Make is distributed in the hope that it will be useful, but WITHOUT ANY
WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR
A PARTICULAR PURPOSE.  See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License along with
this program.  If not, see <http://www.gnu.org/licenses/>.  */

#include "makeint.h"

#include <assert.h>

#include "filedef.h"
#include "file.h"
#include "dep.h"
#include "job.h"
#include "commands.h"
#include "variable.h"
#include "debug.h"
#include "hash.h"

struct hash_table files;

static unsigned long
file_hash_1 (const void *key)
{
  return_ISTRING_HASH_1 (((struct file const *) key)->hname);
}

static unsigned long
file_hash_2 (const void *key)
{
  return_ISTRING_HASH_2 (((struct file const *) key)->hname);
}

int
file_hash_cmp (const void *x, const void *y)
{
  return_ISTRING_COMPARE (((struct file const *) x)->hname,
                          ((struct file const *) y)->hname);
}

#ifndef FILE_BUCKETS
#define FILE_BUCKETS    1007
#endif

struct file *
enter_file (const char *name)
{
  struct file *f;
  struct file *new;
  struct file **file_slot;
  struct file file_key;

  assert (*name != '\0');
  assert (! verify_flag || strcache_iscached (name));

#if defined(VMS) && !defined(WANT_CASE_SENSITIVE_TARGETS)
  if (*name != '.')
    {
      const char *n;
      char *lname, *ln;
      lname = xstrdup (name);
      for (n = name, ln = lname; *n != '\0'; ++n, ++ln)
        if (isupper ((unsigned char)*n))
          *ln = tolower ((unsigned char)*n);
        else
          *ln = *n;

      *ln = '\0';
      name = strcache_add (lname);
      free (lname);
    }
#endif

  file_key.hname = name;
  file_slot = (struct file **) hash_find_slot (&files, &file_key);
  f = *file_slot;
  if (! HASH_VACANT (f) && !f->double_colon)
    {
      f->builtin = 0;
      return f;
    }

  new = xcalloc (sizeof (struct file));
  new->name = new->hname = name;
  new->update_status = us_none;

  if (HASH_VACANT (f))
    {
      new->last = new;
      hash_insert_at (&files, new, file_slot);
    }
  else
    {
      /* There is already a double-colon entry for this file.  */
      new->double_colon = f;
      f->last->prev = new;
      f->last = new;
    }

  return new;
}

void
init_hash_files (void)
{
  hash_init (&files, 1000, file_hash_1, file_hash_2, file_hash_cmp);
}
