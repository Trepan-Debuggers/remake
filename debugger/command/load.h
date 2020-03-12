/* Read and evaluate a Makefile. */
/*
Copyright (C) 2020 R. Bernstein <rocky@gnu.org>

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

#include <glob.h>
#include "read.h"
#include "filedef.h"

static debug_return_t
dbg_cmd_load(char *psz_filename)
{
  if (psz_filename && *psz_filename) {
    glob_t p;
    char *psz_expanded_file;
    struct goaldep *p_goaldep;

    glob(psz_filename, 0, NULL, &p);
    if (0 == p.gl_pathc) {
      struct stat stat_buf;
      int ret = stat(psz_filename, &stat_buf);
      if (ret != 0) {
        dbg_errmsg("Can't find file %s:\n\t%s", psz_filename, strerror(errno));
        return debug_cmd_error;
      }
      psz_expanded_file = psz_filename;

    } else if (1 != p.gl_pathc) {
      dbg_errmsg("Expansion of %s doesn't lead to a single filename. \n"
                 "Got %zu matches",
                 psz_filename, p.gl_pathc);
      return debug_cmd_error;

    } else {
      psz_expanded_file = p.gl_pathv[0];
    }

    snapped_deps = 0;
    p_goaldep = eval_makefile(psz_expanded_file, 0);

    if (p_goaldep == NULL) {
      dbg_errmsg("error reading Makefile %s (expanded to %s):",
                 psz_filename, psz_expanded_file);
      snapped_deps = 1;
      return debug_cmd_error;
    }  else {
      snap_deps();
      free_goaldep(p_goaldep);
    }
    globfree(&p);
  } else {
    dbg_errmsg("load command expects a filename");
  }
  return debug_readloop;
}

static void
dbg_cmd_load_init(unsigned int c)
{
  short_command[c].func = &dbg_cmd_load;
  short_command[c].use = _("load *Makefile*");
}


/*
 * Local variables:
 * eval: (c-set-style "gnu")
 * indent-tabs-mode: nil
 * End:
 */
