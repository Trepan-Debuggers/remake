/*
Copyright (C) 2011, 2020 R. Bernstein <rocky@gnu.org>
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

/**
 *  \brief Debugger `source` command.
 *
 * Read and run debugger commands from a file.
 **/

#include "../../src/trace.h"
#include "../cmd.h"
#include "../fns.h"
#include "../msg.h"

#define DEPENDS_COMMANDS " depends commands"

#include <glob.h>
#include "help/source.h"

extern debug_return_t
dbg_cmd_source(char *psz_filename)
{
  if (psz_filename && *psz_filename) {
    FILE *p_source_file;
    char *psz_expanded_file;
	glob_t p;

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
                 psz_filename, (size_t)p.gl_pathv);
      return debug_cmd_error;

    } else {
      psz_expanded_file = p.gl_pathv[0];
    }

    p_source_file = fopen(psz_expanded_file, "r");

    if (p_source_file != NULL) {
      debug_return_t debug_return = debug_readloop;
      char line[2048];
      while (!feof(p_source_file) && debug_return == debug_readloop)
        {
          char *s;
          char *shut_up_gcc_warning = fgets(line, 2048, p_source_file);
          if (feof(p_source_file)) break;
          chomp(shut_up_gcc_warning);
          if ( *(s=stripwhite(line)) ) debug_return=execute_line(s);
        }
      fclose(p_source_file);
    } else
      dbg_errmsg("error reading file %s (expanded to %s):\n\t%s",
                 psz_filename, psz_expanded_file, strerror(errno));

    globfree(&p);
  }  else {
    dbg_errmsg("Expecting a file name");
    return debug_cmd_error;
  }
  return debug_readloop;
}


/*
 * Local variables:
 * eval: (c-set-style "gnu")
 * indent-tabs-mode: nil
 * End:
 */
