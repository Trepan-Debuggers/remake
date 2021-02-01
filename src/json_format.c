/* Module for outputting json formatted profiling information
Copyright (C) 2021 Google LLC

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

/** \file json_format.c
 *
 *  \brief Module for outputting profiling information in json
 *
 *  JSON output dumps target information in an easily parsable format
 *  for post-processing by external tools. The file is named build.XYZ.json
 *  where XYZ is the pid of the remake process and which is also captured in the
 *  file.
 */

#include <stdio.h>
#include <sys/types.h>
#include <inttypes.h>
#include <config.h>

#include "json_format.h"
#include "dep.h"

/* Uncomment to remove whitespace indents */
//#define JSON_COMPRESS

#define JSON_FILE_PREFIX "build."
#define JSON_FILE_EXT    ".json"
#define JSON_FILE_TEMPLATE JSON_FILE_PREFIX "%d" JSON_FILE_EXT

#define JSON_FILENAME_LEN (sizeof(JSON_FILE_PREFIX) + 20 + sizeof(JSON_FILE_EXT))

#define JSON_FILE_VER   "1.0.0"

#ifdef JSON_COMPRESS
#define LVL1    ""
#else
/*! Indentation string for pretty printing */
#define LVL1    "\t"
#endif

#define LVL2    LVL1 LVL1
#define LVL3    LVL2 LVL1
#define LVL4    LVL3 LVL1
#define LVL5    LVL4 LVL1

static void dump_preamble(FILE *fd, profile_context_t *ctx);
static void dump_creator(FILE *fd, const char *creator);
static void dump_argv(FILE *fd, const char *const *argv);
static void dump_timestamps(FILE *fd, profile_context_t *ctx);
static void dump_targets(FILE *fd);
static void dump_target(const void *item);
static void dump_target_timestamps(FILE *fd, const profile_entry_t *p);
static void dump_depends(FILE *fd, const profile_call_t *c);

static char json_fname[JSON_FILENAME_LEN];
static FILE *json_fd;
static bool first_entry;

static void
escape_print(FILE *fd, const char *str)
{
  for (const char *c = str; *c; c++) {
    switch (*c) {
      /* Replace with a space */
      case '\b':
      case '\t':
      case '\n':
      case '\r':
        fputc(' ', fd);
        break;

      /* Add an escape backslash */
      case '\"':
      case '\\':
        fputc('\\', fd);
        __attribute__((fallthrough));

      default:
        fputc(*c, fd);
    }
  }
}

static void
format_timestamp(FILE *fd, FILE_TIMESTAMP ts)
{
  if (ts == 0) {
    fprintf(fd, "null");
  } else {
    uintmax_t s = FILE_TIMESTAMP_S(ts);
    uintmax_t ns = FILE_TIMESTAMP_NS(ts);
    fprintf(fd, "%" PRIuMAX ".%09" PRIuMAX, s, ns);
  }
}

static void
dump_preamble(FILE *fd, profile_context_t *ctx)
{
  fprintf(fd, "{\n");
  fprintf(fd, LVL1 "\"version\":\"%s\",\n", JSON_FILE_VER);
  fprintf(fd, LVL1 "\"pid\":%d,\n", ctx->pid);
  if (makeparent_pid != 0) {
    fprintf(fd, LVL1 "\"parent\":{\n");
    fprintf(fd, LVL2 "\"pid\":%d,\n", makeparent_pid);
    fprintf(fd, LVL2 "\"target\":\"");
    escape_print(fd, makeparent_target);
    fprintf(fd, "\"\n");
    fprintf(fd, LVL1 "},\n");
  }
}

static void
dump_creator(FILE *fd, const char *creator)
{
  fprintf(fd, LVL1 "\"creator\":\"%s\",\n", creator);
}

static void
dump_argv(FILE *fd, const char *const *argv)
{
  fprintf(fd, LVL1 "\"argv\": [");
  for (size_t i = 0; argv[i]; i++) {
    if (i != 0) {
      fprintf(fd, ",");
    }

    fprintf(fd, "\n" LVL2 "\"");
    escape_print(fd, argv[i]);
    fprintf(fd, "\"");
  }
  fprintf(fd, "\n" LVL1 "],\n");
}

static void
dump_timestamps(FILE *fd, profile_context_t *ctx)
{
  fprintf(fd, LVL1 "\"start\":");
  format_timestamp(fd, ctx->start);
  fprintf(fd, ",\n");

  fprintf(fd, LVL1 "\"end\":");
  format_timestamp(fd, ctx->end);
  fprintf(fd, ",\n");
}

static void
dump_jobserver(FILE *fd, profile_context_t *ctx)
{
  fprintf(fd, LVL1 "\"jobs\":%d,\n", ctx->jobs);
  fprintf(fd, LVL1 "\"server\":%d,\n", ctx->jobserver);

}

static void
dump_status(FILE *fd, const char *status)
{
  fprintf(fd, LVL1 "\"status\":\"%s\",\n", status);
}

static void
dump_entry(FILE *fd, const struct goaldep *entry)
{
  fprintf(fd, LVL1 "\"directory\":\"%s\",\n", starting_directory);
  fprintf(fd, LVL1 "\"entry\":[\n");
  for (const struct goaldep *e = entry; e; e=e->next) {
    fprintf(fd, LVL2 "\"");
    escape_print(fd, e->file->name);
    if (e->next) {
      fprintf(fd, "\",\n");
    } else {
      fprintf(fd, "\"\n");
    }
    fprintf(fd, LVL1 "],\n");
  }
}

static void
dump_targets(FILE *fd)
{
  first_entry = true;
  fprintf(fd, LVL1 "\"targets\":[");
  profile_dump_entries(dump_target);
  fprintf(fd, "\n" LVL1 "]\n");
}

static void
dump_target(const void *item)
{
  const profile_entry_t *p = item;

  if (first_entry) {
    first_entry = false;
  } else {
    fprintf(json_fd, ",");
  }
  fprintf(json_fd, "\n");

  fprintf(json_fd, LVL2 "{\n");
  fprintf(json_fd, LVL3 "\"name\":\"");
  escape_print(json_fd, p->name);
  fprintf(json_fd, "\",\n");
  if (p->floc.filenm) {
    fprintf(json_fd, LVL3 "\"file\":\"%s\",\n", p->floc.filenm);
  } else {
    fprintf(json_fd, LVL3 "\"file\":null,\n");
  }
  fprintf(json_fd, LVL3 "\"line\":%u,\n", (uint32_t)p->floc.lineno);
  dump_target_timestamps(json_fd, p);
  dump_depends(json_fd, p->calls);
  fprintf(json_fd, LVL2 "}");
  fflush(json_fd);
}

static void
dump_target_timestamps(FILE *fd, const profile_entry_t *p)
{
  fprintf(fd, LVL3 "\"start\":");
  format_timestamp(fd, p->start);
  fprintf(fd, ",\n");

  fprintf(fd, LVL3 "\"deps\":");
  format_timestamp(fd, p->deps);
  fprintf(fd, ",\n");

  fprintf(fd, LVL3 "\"recipe\":");
  format_timestamp(fd, p->recipe);
  fprintf(fd, ",\n");

  fprintf(fd, LVL3 "\"end\":");
  format_timestamp(fd, p->end);
  fprintf(fd, ",\n");
}

static void
dump_depends(FILE *fd, const profile_call_t *c)
{
  fprintf(fd, LVL3 "\"depends\":[\n");
  for (; c; c=c->next) {
    fprintf(fd, LVL4 "\"");
    escape_print(fd, c->target->name);
    if (c->next) {
      fprintf(fd, "\",\n");
    } else {
      fprintf(fd, "\"\n");
    }
  }
  fprintf(fd, LVL3 "]\n");
}

extern bool
json_init(profile_context_t *ctx, const char *creator, const char *const *argv)
{
  size_t len;

  len = sprintf(json_fname, JSON_FILE_TEMPLATE, ctx->pid);

  if (len >= JSON_FILENAME_LEN) {
    printf("Error in generating json name\n");
    return false;
  }

  json_fd = fopen(json_fname, "w");
  if (NULL == json_fd) {
    printf("Error in opening json file %s\n", json_fname);
    return false;
  }

  dump_preamble(json_fd, ctx);
  dump_creator(json_fd, creator);
  dump_argv(json_fd, argv);
  return true;
}

extern void
json_close(profile_context_t *ctx, const char *program_status)
{
  dump_jobserver(json_fd, ctx);
  dump_status(json_fd, program_status);
  dump_timestamps(json_fd, ctx);
  dump_entry(json_fd, ctx->entry);
  dump_targets(json_fd);
  fprintf(json_fd, "}");
  fclose(json_fd);
  printf("Created json profiling data file: %s\n", json_fname);
}
