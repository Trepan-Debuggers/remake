/* Write commands associated with a given target. */
/* 
Copyright (C) 2011 R. Bernstein <rocky@gnu.org>
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
/* Comment line - ingore text on line. */
static debug_return_t 
dbg_cmd_chdir (char *psz_args)
{
  if (!psz_args || 0==strlen(psz_args)) {
    printf(_("Argument required (new working directory).\n"));
  } else {
    if ( 0 != chdir(psz_args) ) {
      printf("%s: %s\n", psz_args, strerror(1));
    } else {
      printf (_("Working directory %s.\n"), psz_args);
    }
  }
  return debug_readloop;
}

static void
dbg_cmd_chdir_init(unsigned int c) 
{
  short_command[c].func = &dbg_cmd_chdir;
  short_command[c].use  = _("cd DIR");
  short_command[c].doc  = 
    _("Set the working directory to DIR.");
}

/* 
 * Local variables:
 * eval: (c-set-style "gnu")
 * indent-tabs-mode: nil
 * End:
 */


