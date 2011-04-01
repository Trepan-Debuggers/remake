/* Print working directory. */
/* Copyright (C) 2004, 2005, 2007, 2008, 2009, 2011 R. Bernstein 
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
static debug_return_t 
dbg_cmd_pwd(char *psz_args) 
{
  if (!psz_args || 0==strlen(psz_args)) {
    char wd[300];
    if (NULL == getcwd (wd, sizeof(wd))) {
      printf (_("cannot get current directory %s\n"), strerror(errno));
    } else {
      printf (_("Working directory %s.\n"), wd);
    }
  } else {
    printf(_("The \"pwd\" command does not take an argument: %s\n"), psz_args);
  }

  return debug_readloop;
}

static void
dbg_cmd_pwd_init(unsigned int c) 
{
  short_command[c].func = &dbg_cmd_pwd;
  short_command[c].use = _("pwd");
  short_command[c].doc = 
    _("Print working directory.  This is used for your program as well.");
}

/* 
 * Local variables:
 * eval: (c-set-style "gnu")
 * indent-tabs-mode: nil
 * End:
 */
