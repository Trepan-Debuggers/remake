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
#ifndef REMAKE_FILE2LINE
#define REMAKE_FILE2LINE
struct hash_table file2lines;

typedef struct lineo_array_s 
{
  const char *hname; /**< Name stored in hash table */
  unsigned int size; /**< Number of entries in array */
  file_t **array;    /**< target name or NULL. */
} lineno_array_t;

/*!
  Initializes hash table file2lines. file2lines is used in breakpoints
  only. So we do this on demand.
*/
extern bool file2lines_init(void);
extern file_t *target_for_file_and_line (const char *psz_filename, 
					 unsigned int lineno);

extern void file2lines_dump(void);
#endif

/* 
 * Local variables:
 * eval: (c-set-style "gnu")
 * indent-tabs-mode: nil
 * End:
 */
