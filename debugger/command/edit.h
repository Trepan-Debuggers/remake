/* edit starting at current location. */
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
dbg_cmd_edit(char *psz_args) 
{
  char *editor;

  if ((editor = (char *) getenv ("EDITOR")) == NULL)
      editor = "/bin/ex";

  if (!psz_args || 0==strlen(psz_args)) {
    const floc_t *p_floc = get_current_floc();
    if (p_floc) {
      unsigned int cmd_size = strlen(editor) + strlen(p_floc->filenm) + 20;
      char *edit_cmd = calloc(1, cmd_size);
      int rc;
      /* Quote the file name, in case it has whitespace or other special
	 characters.  */
      snprintf(edit_cmd, cmd_size, 
	       "%s +%lu \"%s\"", editor, p_floc->lineno, p_floc->filenm);
      printf(_("Running %s\n"), edit_cmd);
      rc = system (edit_cmd);
      shell_rc_status(rc);
    } else {
      printf (_("cannot get target file location\n"));
    }
  } else {
    printf(_("The \"edit\" command does not take an argument: %s\n"), psz_args);
  }
  return debug_readloop;
}

static void
dbg_cmd_edit_init(unsigned int c) 
{
  short_command[c].func = &dbg_cmd_edit;
  short_command[c].use = _("edit");
  short_command[c].doc = 
    _("edit"
      ""
      "Enter an editor using the current location. "
      "Uses EDITOR environment variable contents as editor (or ex as default)."
      "Assumes the editor positions at a file using options +linenumber filename.");
}

/* 
 * Local variables:
 * eval: (c-set-style "gnu")
 * indent-tabs-mode: nil
 * End:
 */
