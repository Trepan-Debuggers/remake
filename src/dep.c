/* Dependency routines for GNU Make.
Copyright (C) 2004 Free Software Foundation, Inc.
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
#include "file.h"
#include "variable.h"
#include "commands.h"

/*! Copy dependency chain making a new chain with the same contents
  as the old one and return that.  The return value is malloc'd. The
  caller must thus free it.
 */
dep_t *
copy_dep_chain (dep_t *p_dep)
{
  dep_t *c;
  dep_t *p_firstnew = NULL;
  dep_t *p_lastnew  = NULL;

  for ( ; p_dep ; p_dep = p_dep->next) {
    c = CALLOC(dep_t, 1);
    memmove ((char *) c, (char *) p_dep, sizeof (dep_t));
    if (c->name != 0)
      c->name = strdup (c->name);
    c->next = 0;
    if (p_firstnew == 0)
      p_firstnew = p_lastnew = c;
    else
      p_lastnew = p_lastnew->next = c;
    
  }

  return p_firstnew;
}

/*! Convert name sequence p_nameseq, into to a dependency chain
  as the old one and return that.  The return value is malloc'd. The
  caller must thus free it. p_nameseq is free'd.
 */
dep_t *
nameseq_to_dep_chain (nameseq_t *p_nameseq) 
{

  if (!p_nameseq) return NULL;
  else {
    nameseq_t *p_nameseq_next;
    dep_t *p_firstnew = NULL;
    dep_t *p_lastnew  = NULL;
    dep_t *c;
  
    for ( ; p_nameseq ; p_nameseq = p_nameseq_next) {
      p_nameseq_next = p_nameseq->next;
      c = CALLOC(dep_t, 1);

      c->name = p_nameseq->name;

      if (!p_firstnew)
	p_firstnew = p_lastnew = c;
      else
	p_lastnew = p_lastnew->next = c;
      
      free(p_nameseq);
    }
    return p_firstnew;
  }
}


/*! Free all p_namseq memory.  */
void
nameseq_free(nameseq_t *p_nameseq)
{
  nameseq_t *p_nameseq_next;

  for ( ; p_nameseq; p_nameseq = p_nameseq_next) {
    p_nameseq_next = p_nameseq->next;
    free(p_nameseq->name);
    free(p_nameseq);
  }
}
/*! Copy dependency chain making a new chain with the same contents
  as the old one and return that.  The return value is malloc'd. The
  caller must thus free it.
 */
void
dep_chain_free (dep_t *p_dep)
{
  dep_t *p_dep_next;

  for ( ; p_dep; p_dep = p_dep_next) {
    p_dep_next = p_dep->next;
    free(p_dep->name);
    free(p_dep);
  }
}

/* Set the intermediate flag.  */
static void
set_intermediate (const void *item)
{
  struct file *f = (struct file *) item;
  f->intermediate = 1;
}

/*!
  For each dependency of each file, make the `struct dep' point
  at the appropriate `struct file' (which may have to be created).
  
  Also mark the files depended on by .PRECIOUS, .PHONY, .SILENT,
  and various other special targets.  */

void
snap_deps (hash_table_t *p_files)
{
  file_t *f;
  file_t *f2;
  dep_t *d;
  file_t **file_slot_0;
  file_t **file_slot;
  file_t **file_end;

  /* Enter each dependency name as a file.  */
  /* We must use hash_dump (), because within this loop
     we might add new files to the table, possibly causing
     an in-situ table expansion.  */
  file_slot_0 = (file_t **) hash_dump (p_files, 0, 0);
  file_end = file_slot_0 + p_files->ht_fill;
  for (file_slot = file_slot_0; file_slot < file_end; file_slot++)
    for (f2 = *file_slot; f2 != 0; f2 = f2->prev)
      for (d = f2->deps; d != 0; d = d->next)
	if (d->name != 0)
	  {
	    d->file = lookup_file (d->name);
	    if (d->file == 0)
	      d->file = enter_file (d->name, NILF);
	    else
	      free (d->name);
	    d->name = 0;
	  }
  free (file_slot_0);

  for (f = lookup_file (".PRECIOUS"); f != 0; f = f->prev)
    for (d = f->deps; d != 0; d = d->next)
      for (f2 = d->file; f2 != 0; f2 = f2->prev)
	f2->precious = 1;

  for (f = lookup_file (".LOW_RESOLUTION_TIME"); f != 0; f = f->prev)
    for (d = f->deps; d != 0; d = d->next)
      for (f2 = d->file; f2 != 0; f2 = f2->prev)
	f2->low_resolution_time = 1;

  for (f = lookup_file (".PHONY"); f != 0; f = f->prev)
    for (d = f->deps; d != 0; d = d->next)
      for (f2 = d->file; f2 != 0; f2 = f2->prev)
	{
	  /* Mark this file as phony and nonexistent.  */
	  f2->phony = 1;
	  f2->last_mtime = NONEXISTENT_MTIME;
	  f2->mtime_before_update = NONEXISTENT_MTIME;
	}

  for (f = lookup_file (".INTERMEDIATE"); f != 0; f = f->prev)
    {
      /* .INTERMEDIATE with deps listed
	 marks those deps as intermediate files.  */
      for (d = f->deps; d != 0; d = d->next)
	for (f2 = d->file; f2 != 0; f2 = f2->prev)
	  f2->intermediate = 1;
      /* .INTERMEDIATE with no deps does nothing.
	 Marking all files as intermediates is useless
	 since the goal targets would be deleted after they are built.  */
    }

  for (f = lookup_file (".SECONDARY"); f != 0; f = f->prev)
    {
      /* .SECONDARY with deps listed
	 marks those deps as intermediate files
	 in that they don't get rebuilt if not actually needed;
	 but unlike real intermediate files,
	 these are not deleted after make finishes.  */
      if (f->deps)
        for (d = f->deps; d != 0; d = d->next)
          for (f2 = d->file; f2 != 0; f2 = f2->prev)
            f2->intermediate = f2->secondary = 1;
      /* .SECONDARY with no deps listed marks *all* files that way.  */
      else
        {
          all_secondary = 1;
          hash_map (p_files, set_intermediate);
        }
    }

  f = lookup_file (".EXPORT_ALL_VARIABLES");
  if (f != 0 && f->is_target)
    export_all_variables = 1;

  f = lookup_file (".IGNORE");
  if (f != 0 && f->is_target)
    {
      if (f->deps == 0)
	ignore_errors_flag = 1;
      else
	for (d = f->deps; d != 0; d = d->next)
	  for (f2 = d->file; f2 != 0; f2 = f2->prev)
	    f2->command_flags |= COMMANDS_NOERROR;
    }

  f = lookup_file (".SILENT");
  if (f != 0 && f->is_target)
    {
      if (f->deps == 0)
	silent_flag = 1;
      else
	for (d = f->deps; d != 0; d = d->next)
	  for (f2 = d->file; f2 != 0; f2 = f2->prev)
	    f2->command_flags |= COMMANDS_SILENT;
    }

  f = lookup_file (".POSIX");
  if (f != 0 && f->is_target)
    posix_pedantic = 1;

  f = lookup_file (".NOTPARALLEL");
  if (f != 0 && f->is_target)
    not_parallel = 1;
}

static unsigned long
dep_hash_1 (const void *key)
{
  return_STRING_HASH_1 (dep_name ((dep_t const *) key));
}

static unsigned long
dep_hash_2 (const void *key)
{
  return_STRING_HASH_2 (dep_name ((dep_t const *) key));
}

static int
dep_hash_cmp (const void *x, const void *y)
{
  dep_t *dx = (dep_t *) x;
  dep_t *dy = (dep_t *) y;
  int cmp = strcmp (dep_name (dx), dep_name (dy));

  /* If the names are the same but ignore_mtimes are not equal, one of these
     is an order-only prerequisite and one isn't.  That means that we should
     remove the one that isn't and keep the one that is.  */

  if (!cmp && dx->ignore_mtime != dy->ignore_mtime)
    dx->ignore_mtime = dy->ignore_mtime = false;

  return cmp;
}

/*! Remove duplicate dependencies in CHAIN.  */
void
uniquize_deps (dep_t *chain)
{
  hash_table_t deps;
  dep_t **depp;

  hash_init (&deps, 500, dep_hash_1, dep_hash_2, dep_hash_cmp);

  /* Make sure that no dependencies are repeated.  This does not
     really matter for the purpose of updating targets, but it
     might make some names be listed twice for $^ and $?.  */

  depp = &chain;
  while (*depp)
    {
      dep_t *dep = *depp;
      dep_t **dep_slot = (struct dep **) hash_find_slot (&deps, dep);
      if (HASH_VACANT (*dep_slot))
	{
	  hash_insert_at (&deps, dep, dep_slot);
	  depp = &dep->next;
	}
      else
	{
	  /* Don't bother freeing duplicates.
	     It's dangerous and little benefit accrues.  */
	  *depp = dep->next;
	}
    }

  hash_free (&deps, 0);
}
