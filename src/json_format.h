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

/** \file json_format.h
 *
 *  \brief Header for routines for outputting profiling information in json
 *
 *  JSON output dumps target information in an easily parsable format
 *  for post-processing by external tools. The file is named build.XYZ.json
 *  where XYZ is the pid of the remake process and which is also captured in the
 *  file.
 *
 *  The output format uses the following scheme:
 *
 *  \code{.json}
 *  {
 *      "version":"1.0.0",
 *      "pid":1234,
 *      "parent":{
 *          "pid":1230,
 *          "target":"target-name"
 *      },
 *      "jobs": 8,
 *      "server": 1,
 *      "creator":"remake"
 *      "argv":[
 *          "arg0",
 *          "arg1",
 *          "arg2"
 *      ],
 *      "directory":"path/to/working/dir",
 *      "status":"Normal program termination",
 *      "start":1235.6789,
 *      "end":1240.0123,
 *      "resolution":1000,
 *      "entry":[
 *          "all",
 *          "other"
 *      ],
 *      "targets":[
 *          {
 *              "name":"all",
 *              "file":"path/to/file/with/target",
 *              "line":100,
 *              "start":123.123,
 *              "deps":null,
 *              "recipe":124.0,
 *              "end":124.124,
 *              "depends":[
 *                  "dependent",
 *                  "target0",
 *                  "target1",
 *                  "target2"
 *              ]
 *          }
 *      ]
 *  }
 *  \endcode
 */

#ifndef _REMAKE_JSON_FORMAT_H
#define _REMAKE_JSON_FORMAT_H

#include "profile.h"

/**
 * \brief Initializes json output for profiling
 *
 * \param[in] ctx Context of the profiler
 * \param[in] creator String identifying creator of the file
 * \param[in] argv Array of strings that represent the command used to start remake
 */
extern bool json_init(profile_context_t *ctx, const char *creator, const char *const *argv);

/**
 * \brief Dumps profiling data and closes the json output file
 *
 * \param[in] ctx Context of the profiler
 * \param[in] program_status Textual representation of the exit reason
 */
extern void json_close(profile_context_t *ctx, const char *program_status);

#endif // _REMAKE_JSON_FORMAT_H
