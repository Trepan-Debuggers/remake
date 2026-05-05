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

#include "callgrind_format.h"

#define CALLGRIND_FILE_PREFIX "callgrind.out."
#define CALLGRIND_FILE_TEMPLATE "%s/" CALLGRIND_FILE_PREFIX "%d"

/* + 10 is more than enough since 2**64 ~= 10**9 */
#define CALLGRIND_FILENAME_LEN (GET_PATH_MAX)

#define CALLGRIND_PREAMBLE_TEMPLATE1 "version: 1\n\
creator: %s\n"

#define CALLGRIND_PREAMBLE_TEMPLATE2 "pid: %d\n\
\n\
desc: Trigger: %s\n\
desc: Node: Targets\n\
\n\
positions: line\n\
event: Wt : Wall Time\n\
event: Rt : Recipe Time\n\
events: Wt Rt\n"

char callgrind_fname[CALLGRIND_FILENAME_LEN];
static FILE *callgrind_fd;

extern bool
callgrind_init(profile_context_t *ctx, const char *creator, const char *const *argv) {
  size_t len;
  unsigned int i;

  len = sprintf(callgrind_fname, CALLGRIND_FILE_TEMPLATE, ctx->output_dir, ctx->pid);

  if (len >= CALLGRIND_FILENAME_LEN) {
    printf("Error in generating callgrind name\n");
    return false;
  }

  callgrind_fd = fopen(callgrind_fname, "w");
  if (NULL == callgrind_fd) {
    printf("Error in opening callgrind file %s\n", callgrind_fname);
    return false;
  }

  fprintf(callgrind_fd, CALLGRIND_PREAMBLE_TEMPLATE1,
    creator);
  fprintf(callgrind_fd, "cmd:");
  for (i = 0; argv[i]; i++) {
    fprintf(callgrind_fd, " %s", argv[i]);
  }
  fprintf(callgrind_fd, "\n");

  return true;
}

static void
callgrind_profile_entry (const void *item)
{
  const profile_entry_t *p = item;
  profile_call_t *c;

  if (NULL == p->floc.filenm) {
    /* If we don't have file-location that don't bother dumping */
    return;
  }

  fprintf(callgrind_fd, "fl=%s/%s\n\n", starting_directory, p->floc.filenm);
  fprintf(callgrind_fd, "fn=%s\n", p->name);
  fprintf(callgrind_fd, "%" PRIu64 " %" PRIu64 " %" PRIu64 "\n",
          (uint64_t) p->floc.lineno,
          p->elapsed_total,
          p->elapsed_recipe);

  for (c = p->calls; c; c = c->next) {
    if (c->target->floc.filenm) {
      fprintf(callgrind_fd, "cfi=%s\n", c->target->floc.filenm);
    }

    fprintf(callgrind_fd, "cfn=%s\n", c->target->name);
    fprintf(callgrind_fd, "calls=1 %" PRIu64 "\n",
            (uint64_t) p->floc.lineno);
    fprintf(callgrind_fd, "%" PRIu64 " %" PRIu64 " %" PRIu64 "\n",
            (uint64_t) p->floc.lineno,
            c->target->elapsed_total,
            c->target->elapsed_recipe);
  }
  fprintf(callgrind_fd, "\n");
}

extern void
callgrind_close(profile_context_t *ctx, const char *program_status) {

  fprintf(callgrind_fd, CALLGRIND_PREAMBLE_TEMPLATE2, ctx->pid,
          program_status);

  fprintf(callgrind_fd, "summary: %" PRIu64 "\n\n", ctx->elapsed);

  profile_dump_entries(callgrind_profile_entry);
  fclose(callgrind_fd);

  printf("Created callgrind profiling data file: %s\n", callgrind_fname);
}
