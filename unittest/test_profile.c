/*
Copyright (C) 2015, 2017, 2020 R. Bernstein <rocky@gnu.org>
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

#include "../src/config.h"
#include "../src/types.h"
#include "../src/file_basic.h"
#include "../src/profile.h"
#include "../src/dep.h"

#define TS_RES       (1000)
#define TS_INC       (5 * TS_RES)
static FILE_TIMESTAMP fake_clock = 0;

void
die (int status)
{
  exit (status);
}

FILE_TIMESTAMP
file_timestamp_now (int *resolution) {
  *resolution = TS_RES;

  fake_clock += TS_INC;
  return fake_clock;
}

int main(int argc, const char * const* argv) {
  bool rc = profile_init(PACKAGE_TARNAME " " PACKAGE_VERSION, argv, 0);
  (void)argc;
  init_hash_files();

  if (rc) {
    file_t *target = enter_file("Makefile");
    file_t *target2, *target3;
    goaldep_t *goals = alloc_goaldep();

    target->floc.filenm = "Makefile";
    goals->file = target;

    target2 = enter_file("all");
    target2->floc.filenm = "Makefile";
    target2->floc.lineno = 5;
    target2->elapsed_time = 500;
    profile_add_dependency(target2, NULL);

    target3 = enter_file("all-recursive");
    target3->floc.filenm = "Makefile";
    target3->floc.lineno = 5;
    target3->elapsed_time = 1000;
    profile_add_dependency(target3, target2);

    profile_close("Program termination", goals, 0);
  }

  if (rc == true) {
    return 0;
  } else {
    return 1;
  }
}
