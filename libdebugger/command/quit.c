/*
Copyright (C) 2004-2005, 2007-2009, 2011, 2020 R. Bernstein
<rocky@gnu.org>
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

/** \file libdebugger/command/quit.c
 *
 *  \brief Debugger `quit` command.
 *
 *  Debugger command to terminate `remake`.
 */

#include "../../src/trace.h"
#include "../../src/debug.h"
#include "../fns.h"
#include "../msg.h"

extern debug_return_t
dbg_cmd_quit(char *psz_arg)
{
  if (!psz_arg || !*psz_arg) {
    in_debugger = DEBUGGER_QUIT_RC;
    dbg_msg("remake: That's all, folks...\n");
    die(DEBUGGER_QUIT_RC);
  } else {
    int rc=0;
    if (get_int(psz_arg, &rc, true)) {
      dbg_msg("remake: That's all, folks...\n");
      die(rc);
    }
  }
  return debug_readloop;
}

/*
 * Local variables:
 *  c-file-style: "gnu"
 *  indent-tabs-mode: nil
 * End:
 */
