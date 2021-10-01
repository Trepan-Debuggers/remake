/*
Copyright (C) 2015 R. Bernstein <rocky@gnu.org>
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

#include <stdio.h>
#include <sys/types.h>
#include <config.h>

/* _LARGE_FILES can get set in <config.h>. However this messes up
AIX #include headers which presumably it was there for.
There is *nothing* in this file that needs or wants large-file access.
So we'll disable it, independent of what config.h says.
*/
#undef _LARGE_FILES

#include <unistd.h>
#include <errno.h>

#include "globals.h"
#include "profile.h"
#include "hash.h"
#include "dep.h"

#include "callgrind_format.h"
#include "json_format.h"

/* #define DEBUG_PROFILE */

static struct hash_table profile_table;

static unsigned long
profile_table_entry_hash_1 (const void *key)
{
  return_ISTRING_HASH_1 (((profile_entry_t *) key)->name);
}

static unsigned long
profile_table_entry_hash_2 (const void *key)
{
  return_ISTRING_HASH_2 (((profile_entry_t *) key)->name);
}

static int
profile_hash_cmp (const void *x, const void *y)
{
  return_ISTRING_COMPARE (((profile_entry_t *) x)->name,
                          ((profile_entry_t *) y)->name);
}

static profile_entry_t *
add_profile_entry (const file_t *target)
{
  /* Look up the string in the hash.  If it's there, return it.  */

  profile_entry_t *new;
  profile_entry_t **slot;
  profile_entry_t *profile_entry;

  new = xcalloc (sizeof (profile_entry_t));
  new->name = target->name;

  slot = (profile_entry_t **) hash_find_slot (&profile_table, new);
  profile_entry = *slot;
  if (!HASH_VACANT (profile_entry)) {
    free(new);
    return profile_entry;
  }

  /* Not there yet. So finish initializing data and add it into the
     profile hash table.  */
  new->calls = NULL;
  memcpy(&(new->floc), &(target->floc), sizeof(gmk_floc));

  hash_insert_at (&profile_table, new, slot);
  return new;
}

static profile_context_t ctx;

extern bool
profile_init(const char *creator, const char *const *argv, int jobs) {

  ctx.jobs = jobs;
  ctx.pid = getpid();
  ctx.output_dir = profile_directory;

  ctx.resolution = 0;
  ctx.start = file_timestamp_now(&ctx.resolution);

#ifdef PROFILER_MIN_RES
  /* Require at least microsecond resolution */
  if (ctx.resolution > PROFILER_MIN_RES) {
    printf("Resolution is only %d ns. Profiling not supported\n", ctx.resolution);
    return false;
  }
#endif

  switch (profile_flag) {
    case PROFILE_CALLGRIND:
      if (!callgrind_init(&ctx, creator, argv)) {
        return false;
      }
      break;
    case PROFILE_JSON:
      if (!json_init(&ctx, creator, argv)) {
        return false;
      }
      break;
    default:
      break;
  }

  hash_init(&profile_table, 1000, profile_table_entry_hash_1,
      profile_table_entry_hash_2, profile_hash_cmp);

  return true;
}

static void add_timestamp(profile_entry_t *t, bool updated, enum cmd_state state)
{
  FILE_TIMESTAMP *ts = NULL;
  int resolution = 0;
  if (state == cs_not_started && t->start == 0) {
    /* This is the first time this target is checked */
    ts = &t->start;
  } else if (state == cs_deps_running && t->deps == 0) {
    ts = &t->deps;
  } else if (state == cs_running && t->recipe == 0) {
    ts = &t->recipe;
  } else if ((state == cs_finished || updated) && t->end == 0) {
    ts = &t->end;
  } else {
    /* No updates */
    return;
  }
  *ts = file_timestamp_now(&resolution);

  if (t->start > 0 && t->end > 0) {
    t->elapsed_total = (t->end - t->start)/1000;
  }

  if (t->recipe > 0 && t->end > 0) {
    t->elapsed_recipe = (t->end - t->recipe)/1000;
  }
}

static void
add_dependency(profile_entry_t *target, profile_entry_t *parent)
{
  profile_call_t *c = CALLOC(profile_call_t, 1);
  c->target = target;
  c->next = parent->calls;
  parent->calls = c;
}

extern void
profile_add_dependency(file_t *target, file_t *parent)
{
  profile_entry_t *p = NULL;
  profile_entry_t *t = add_profile_entry(target);

  add_timestamp(t, (target->updated == 1), target->command_state);
  if (parent) {
    p = add_profile_entry(parent);
    add_dependency(t, p);
  }
}

extern void
profile_update_target(file_t *target)
{
  profile_entry_t *t = add_profile_entry(target);
  add_timestamp(t, (target->updated == 1), target->command_state);
  memcpy(&(t->floc), &(target->floc), sizeof(gmk_floc));
}

extern void
profile_add_timestamp(file_t *target)
{
  profile_entry_t *t = add_profile_entry(target);
  add_timestamp(t, (target->updated == 1), target->command_state);
}

#ifdef DEBUG_PROFILE
/* Print profile entry */
static void
print_profile_entry (const void *item)
{
  const profile_entry_t *p = item;
  profile_call_t *c;
  printf("name: %s, file: %s, line: %" PRIu64 ", time: %" PRIu64 "\n",
         p->name, p->floc.filenm, p->floc.lineno, p->elapsed_total);
  for (c = p->calls; c; c = c->p_next) {
    printf("calls: %s\n", c->p_target->name);
  }
}
#endif

extern void
profile_dump_entries(profile_dump_entry_fn fn) {
  hash_map(&profile_table, fn);
}

extern void
profile_close(const char *program_status, const struct goaldep *goal, bool jobserver) {
  int resolution = 0;
  ctx.end = file_timestamp_now(&resolution);

  ctx.elapsed = (ctx.end - ctx.start)/1000;
  ctx.entry = goal;
  ctx.jobserver = jobserver;

#ifdef DEBUG_PROFILE
  profile_dump_entries(print_profile_entry);
#endif

  switch (profile_flag) {
    case PROFILE_CALLGRIND:
      callgrind_close(&ctx, program_status);
      break;
    case PROFILE_JSON:
      json_close(&ctx, program_status);
      break;
    default:
      break;
  }
}
