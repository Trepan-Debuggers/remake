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

#include "../make.h"
#include "../config.h"
#include "../types.h"
#include "../file_basic.h"
#include "../profile.h"
int main(int argc, char **argv) {
  bool rc = init_callgrind(PACKAGE_TARNAME " " PACKAGE_VERSION, argv);
  init_hash_files();
  if (rc) {
    file_t *target = enter_file("Makefile", NILF);
    file_t *target2, *target3;
    target->floc.filenm = "Makefile";


    target2 = enter_file("all", NILF);
    target2->floc.filenm = "Makefile";
    target2->floc.lineno = 5;
    target2->elapsed_time = 500;
    add_target(target2, NULL);

    target3 = enter_file("all-recursive", NILF);
    target3->floc.filenm = "Makefile";
    target3->floc.lineno = 5;
    target3->elapsed_time = 1000;
    add_target(target3, target2);

    close_callgrind("Program termination");
  }
  if (rc == true) {
    return 0;
  } else {
    return 1;
  }
}
