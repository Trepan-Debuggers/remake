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

#ifndef _REMAKE_CALLGRIND_FORMAT_H
#define _REMAKE_CALLGRIND_FORMAT_H

#include "profile.h"

/**
 * \brief Initializes callgrind output for profiling
 *
 * \param[in] ctx Context of the profiler
 * \param[in] creator String identifying creator of the file
 * \param[in] argv Array of strings that represent the command used to start remake
 */
extern bool callgrind_init(profile_context_t *ctx, const char *creator, const char *const *argv);

/**
 * \brief Dumps profiling data and closes the callgrind output file
 *
 * \param[in] ctx Context of the profiler
 * \param[in] program_status Textual representation of the exit reason
 */
extern void callgrind_close(profile_context_t *ctx, const char *program_status);

#endif // _REMAKE_CALLGRIND_FORMAT_H
