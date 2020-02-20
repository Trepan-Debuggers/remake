/** Move reported target frame position up by psz_amount. */
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
debug_return_t 
dbg_cmd_up (char *psz_amount)
{
  unsigned int i_amount=1;

  if (!psz_amount || !*psz_amount) {
    i_amount = 1;
  } else {
    if (!get_uint(psz_amount, &i_amount, true))
      return debug_readloop;
  }
  return dbg_adjust_frame(i_amount, false);
}

static void
dbg_cmd_up_init(unsigned int c) 
{
  short_command[c].func = &dbg_cmd_up;
  short_command[c].use  = _("up [AMOUNT]");
  short_command[c].doc  = 
    _("Select and print target that caused this one to be examined.\n"
      "An argument says how many targets up to go.");
}



/* 
 * Local variables:
 * eval: (c-set-style "gnu")
 * indent-tabs-mode: nil
 * End:
 */

