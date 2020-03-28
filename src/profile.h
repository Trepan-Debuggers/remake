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

#include <sys/time.h>
#include "filedef.h"
extern bool init_callgrind(const char *creator, const char *const *argv);
extern void add_target(file_t *target, file_t *prev);
extern void close_callgrind(const char *program_status);
extern bool get_time(struct timeval *t);
extern uint64_t time_diff(struct timeval *start_time, struct timeval *finish_time);
