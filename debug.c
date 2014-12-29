/* Debugging macros and interface.
  Copyright (c) 2005, 2011 Rocky Bernstein <rocky@gnu.org>

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

#include "makeint.h"
#include "make.h"
#include "debug.h"
#include "print.h"

debug_level_mask_t debug_dummy_level_mask;
debug_enter_debugger_t debug_dummy_enter_debugger_mask;

int db_level   = 0;
int debug_flag = 0;

/*! If true, enter the debugger before updating goal. */
bool b_debugger_goal = false;

/*! If true, enter the debugger before reading any makefiles. */
bool b_debugger_preread = false;
stringlist_t *db_flags;

/** Toggle -d on receipt of SIGUSR1.  */
#ifdef SIGUSR1
RETSIGTYPE
debug_signal_handler (int sig)
{
  UNUSED_ARGUMENT(sig);
  db_level = db_level ? DB_NONE : DB_BASIC;
}
#endif

/*! Set the global db_level mask based on the command option list
  db_flags.
 */
void
decode_debug_flags (int b_debug_flag, stringlist_t *ppsz_db_flags)
{
  const char **pp;

  if (b_debug_flag)
    db_level = DB_ALL;

  if (!ppsz_db_flags)
    return;

  for (pp=ppsz_db_flags->list; *pp; ++pp)
    {
      const char *p = *pp;

      while (1)
        {
          switch (tolower (p[0]))
            {
            case 'a':
              db_level |= DB_ALL;
              break;
            case 'b':
              db_level |= DB_BASIC;
              break;
            case 'i':
              db_level |= DB_BASIC | DB_IMPLICIT;
              break;
            case 'j':
              db_level |= DB_JOBS;
              break;
            case 'm':
              db_level |= DB_BASIC | DB_MAKEFILES;
              break;
            case 'r':
              db_level |= DB_BASIC | DB_READ_MAKEFILES;
              break;
            case 'v':
              db_level |= DB_BASIC | DB_VERBOSE;
              break;
            default:
              OS ( fatal, NILF, _("unknown debug level specification `%s'"), p);
            }

          while (*(++p) != '\0')
            if (*p == ',' || *p == ' ')
              break;

          if (*p == '\0')
            break;

          ++p;
        }
    }
}

/*
 * Local variables:
 * eval: (c-set-style "gnu")
 * indent-tabs-mode: nil
 * End:
 */
