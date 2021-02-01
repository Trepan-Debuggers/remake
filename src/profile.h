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

/** \file profile.h
 *
 *  \brief Profiler API for capturing dependency and timing information
 *  of targets
 */

#ifndef _REMAKE_PROFILE_H
#define _REMAKE_PROFILE_H

#include <sys/time.h>
#include "filedef.h"

struct profile_entry;

/*! \brief Node for an item in the target call stack */
typedef struct profile_call   {
  struct profile_entry   *target;   /*!< Pointer to profile entry to called target */
  struct profile_call *next;        /*!< Pointer to next profile call */
} profile_call_t;

/*! \brief Node for a target */
typedef struct profile_entry   {
  FILE_TIMESTAMP start;     /*!< Timestamp when the entry is initially consider */
  FILE_TIMESTAMP deps;      /*!< Timestamp when the entry is waiting on dependencies */
  FILE_TIMESTAMP recipe;    /*!< Timestamp when the entry is running it's recipe */
  FILE_TIMESTAMP end;       /*!< Timestamp when the entry is completed */
  FILE_TIMESTAMP elapsed_total;   /*!< Total time from initial check to finished */
  FILE_TIMESTAMP elapsed_recipe;  /*!< Total time recipe is running */

  const char *name;         /*!< Pointer to the name of the target */
  gmk_floc floc;            /*!< location in Makefile - for tracing */
  profile_call_t *calls;    /*!< List of targets this target calls.  */
} profile_entry_t;

typedef struct profile_context {
  int jobs;                     /*!< Number of jobs allowed */
  bool jobserver;               /*!< If connected to jobserver */
  pid_t pid;                    /*!< PID of this instance of remake */
  FILE_TIMESTAMP start;         /*!< Timestamp of profiler init */
  FILE_TIMESTAMP end;           /*!< Timestamp of profiler closer */
  int resolution;               /*!< Resolution of the timestamps */

  FILE_TIMESTAMP elapsed;       /*!< Total time running remake */
  const struct goaldep *entry;  /*!< Pointer to chain of entries to remake */
} profile_context_t;

/*! \brief Function for processing a profile entry for output */
typedef void (*profile_dump_entry_fn)(const void *item);

/*! \brief Initialize profiler
 *
 *  \param[in] creator String identifying the creator of the profiling output
 *  \param[in] argv Array of strings that represent the command use to start remake
 *  \param[in] jobs Number of jobs 
 *
 *  \return True if successfully initialized, False otherwise
 */
extern bool profile_init(const char *creator, const char *const *argv, int jobs);

/*! \brief Update a target's profile entry
 *
 *  \note Will create a profiler entry for target if it doesn't exist
 *
 *  \param target Pointer to target's file struct
 */
extern void profile_update_target(file_t *target);

/*! \brief Add a dependency relationship for a target
 *
 *  \note Will create a profiler entry for target and parent if they doesn't exist
 *
 *  \param target Pointer to target's file struct
 *  \param parent Pointer to a target's parent's file struct
 */
extern void profile_add_dependency(file_t *target, file_t *parent);

/*! \brief Add a timestamp for a target based on it's state
 *
 *  \note Will create a profiler entry for target if it doesn't exist
 *
 *  \param target Pointer to target's file struct
 */
extern void profile_add_timestamp(file_t *target);

/*! \brief Processes all entries recorded using the provided function
 *
 *  Should be called by an output formatter during it's close routine.
 *
 *  \param[in] fn Function for processing a profiler entry
 */
extern void profile_dump_entries(profile_dump_entry_fn fn);

/*! \brief Close the profiler
 *
 *  \param[in] program_status Textual representation of the exit reason
 *  \param[in] goals Pointer to chain of entry points of this run of remake
 *  \param[in] jobserver True if using a jobserver
 */
extern void profile_close(const char *program_status, const struct goaldep *goals, bool jobserver);

#endif /*_REMAKE_PROFILE_H */
