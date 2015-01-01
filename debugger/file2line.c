/*
Copyright (C) 2004, 2005, 2007, 2008, 2009, 2011 R. Bernstein
<rocky@gnu.org>
This file is part of GNU Make (remake variant).

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
/* Create filename to line number table used by breakpoints */
#include "../make.h"
#include <assert.h>
#include "../filedef.h"
#include "../file.h"
#include "../read.h"
#include "../rule.h"
#include "./file2line.h"

unsigned long
file2lines_hash_1 (const void *key)
{
  return_ISTRING_HASH_1 (((const lineno_array_t *) key)->hname);
}

unsigned long
file2lines_hash_2 (const void *key)
{
  return_ISTRING_HASH_2 (((const lineno_array_t *) key)->hname);
}

static int
file2lines_hash_cmp (const void *x, const void *y)
{
  return strcmp(((const lineno_array_t *) x)->hname,
		((const lineno_array_t *) y)->hname);
}

lineno_array_t *
lookup_file2lines (const char *psz_filename)
{
  assert (*psz_filename != '\0');
  return hash_find_item (&file2lines, &psz_filename);
}

/* FIXME return a status code when we fail. */
file_t *
target_for_file_and_line (const char *psz_filename, unsigned int lineno,
                          /*out*/ f2l_entry_t *entry_type)
{
  lineno_array_t **pp_linenos;
  lineno_array_t lookup_linenos;
  assert (*psz_filename != '\0');
  lookup_linenos.hname = psz_filename;
  if (0 == file2lines.ht_size) file2lines_init();
  pp_linenos = (lineno_array_t **)
    hash_find_slot(&file2lines, &lookup_linenos);

  if (NULL == *pp_linenos) return NULL;
  if (lineno > (*pp_linenos)->size) return NULL;
  *entry_type = (*pp_linenos)->type[lineno];
  return (*pp_linenos)->array[lineno];
}

void
enter_target_lineno (const char *psz_filename, unsigned int lineno,
	      file_t *p_target)
{
  lineno_array_t lookup_linenos;
  lineno_array_t **pp_linenos;
  file_t *p_file;

  lookup_linenos.hname = psz_filename;
  pp_linenos = (lineno_array_t **)
    hash_find_slot(&file2lines, &lookup_linenos);
  p_file = lookup_file(psz_filename);

  if (p_file == NULL) {
    printf("Could not find file %s\n", psz_filename);
    return;
  }
  if (p_file->nlines == 0) {
    printf("Warning: %s shows no lines\n", psz_filename);
  }

  if (HASH_VACANT(*pp_linenos)) {
    const unsigned int nlines = p_file->nlines+1;
    void **new_array = calloc (sizeof(void *), nlines);
    f2l_entry_t *new_type = calloc (sizeof(f2l_entry_t *), nlines);
    lineno_array_t *p_new_linenos = calloc (sizeof(lineno_array_t), 1);
    *pp_linenos = p_new_linenos;
    (*pp_linenos)->hname = psz_filename;
    (*pp_linenos)->type = new_type;
    (*pp_linenos)->array = new_array;
    (*pp_linenos)->size = nlines;
  }
  (*pp_linenos)->type[lineno]  = F2L_TARGET;
  (*pp_linenos)->array[lineno] = p_target;
}


static void
file2line_init (const void *item)
{
  file_t *p_target = (file_t *) item;
  const gmk_floc *p_floc = &p_target->floc;
  if (p_floc && p_floc->filenm) {
    enter_target_lineno(p_floc->filenm, p_floc->lineno, p_target);
  }
}

static void
enter_rule_lineno (rule_t *r)
{
  lineno_array_t lookup_linenos;
  lineno_array_t **pp_linenos;
  const char *psz_filename = r->floc.filenm;
  const unsigned int lineno = r->floc.lineno;
  file_t *p_file;

  if (!psz_filename) return;
  lookup_linenos.hname = psz_filename;
  pp_linenos = (lineno_array_t **)
    hash_find_slot(&file2lines, &lookup_linenos);
  p_file = lookup_file(psz_filename);

  if (p_file == NULL) {
    printf("Could not find file %s\n", psz_filename);
    return;
  }
  if (p_file->nlines == 0) {
    printf("Warning: %s shows no lines\n", psz_filename);
  }

  if (HASH_VACANT(*pp_linenos)) {
    const unsigned int nlines = p_file->nlines+1;
    void **new_array = calloc (sizeof(void *), nlines);
    f2l_entry_t *new_type = calloc (sizeof(f2l_entry_t *), nlines);
    lineno_array_t *p_new_linenos = calloc (sizeof(lineno_array_t), 1);
    *pp_linenos = p_new_linenos;
    (*pp_linenos)->hname = psz_filename;
    (*pp_linenos)->type = new_type;
    (*pp_linenos)->array = new_array;
    (*pp_linenos)->size = nlines;
  }
  (*pp_linenos)->type[lineno]  = F2L_PATTERN;
  (*pp_linenos)->array[lineno] = r;
}


/*!
  Initializes hash table file2lines. file2lines is used in breakpoints
  only. So we do this on demand.
*/
bool file2lines_init(void)
{
  if (!read_makefiles) return false;
  hash_init (&file2lines, files.ht_size, file2lines_hash_1, file2lines_hash_2,
	     file2lines_hash_cmp);
  hash_map (&files, file2line_init);

  {
    rule_t *r;
    for (r = pattern_rules; r != 0; r = r->next) {
      enter_rule_lineno(r);
    }
  }

  return true;
}

void file2lines_print_entry(const void *item)
{
    const lineno_array_t *p_linenos = (lineno_array_t *) item;
    unsigned int i;
    file_t *p_target;
    printf("%s:\n", p_linenos->hname);
    for (i=0; i<p_linenos->size; i++)
      {
	p_target = p_linenos->array[i];
        if (p_target) {
          if (p_linenos->type[i] == F2L_TARGET) {
            printf("%8lu: %s\n",
                   p_target->floc.lineno, p_target->name);
          } else  {
            rule_t *p_rule = (rule_t *) p_target;
            printf("%8lu: %s (pattern)\n",
                   p_rule->floc.lineno, p_rule->targets[0]);
          }
        }
      }
}

void file2lines_dump(void)
{
  file2lines_init();
  hash_map (&file2lines, file2lines_print_entry);
}

/*
 * Local variables:
 * eval: (c-set-style "gnu")
 * indent-tabs-mode: nil
 * End:
 */
