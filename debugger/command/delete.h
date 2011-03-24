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
/* 
   Delete some breakpoints. Arguments are breakpoint numbers with spaces 
   in between."To delete all breakpoints, give no argument.
*/
static debug_return_t 
dbg_cmd_delete(char *psz_args)
{
  int i_brkpt;
  char *psz_word;

  if (!psz_args || !*psz_args) {
    while(i_breakpoints) 
      remove_breakpoint(1);
    return debug_readloop;
  }
  
  psz_word = get_word(&psz_args);
  while ( psz_word && *psz_word ) {
    if (get_int(psz_word, &i_brkpt, true)) {
      remove_breakpoint(i_brkpt);
    }
    psz_word = get_word(&psz_args);
  }

  return debug_readloop;
}

static void
dbg_cmd_delete_init(void) 
{
  short_command['d'].func = &dbg_cmd_delete;
  short_command['d'].use  = _("delete breakpoint numbers..");
  short_command['d'].doc  = _("Delete some breakpoints\n."
"Arguments are breakpoint numbers with spaces in between.\n"
"To delete all breakpoints, give no argument.");
}

/* 
 * Local variables:
 * eval: (c-set-style "gnu")
 * indent-tabs-mode: nil
 * End:
 */
