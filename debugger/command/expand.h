/* Show a string with variable references expanded. */
/* 
Copyright (C) 2004, 2005, 2007, 2008, 2009, 2011 R. Bernstein 
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
dbg_cmd_expand (char *psz_string)
{
  static char *psz_last_string = NULL;

  if (!psz_string || !*psz_string) {
    /* Use last target value */
    if (psz_last_string)
      psz_string = psz_last_string;
    else {
      printf("No current expand string - must supply something to print\n");
      return debug_readloop;
    }
  }
  
  if (dbg_cmd_show_exp(psz_string, true)) {
    if (psz_last_string) free(psz_last_string);
    psz_last_string = strdup(psz_string);
  }
  return debug_readloop;
}
