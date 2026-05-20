/* Definitions for Windows path manipulation.
Copyright (C) 1996-2025 Free Software Foundation, Inc.
Copyright (C) 2026 Rocky Bernstein
This file is part of GNU Remake.

GNU Make is free software; you can redistribute it and/or modify it under the
terms of the GNU General Public License as published by the Free Software
Foundation; either version 3 of the License, or (at your option) any later
version.

GNU Make is distributed in the hope that it will be useful, but WITHOUT ANY
WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR
A PARTICULAR PURPOSE.  See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License along with
this program.  If not, see <https://www.gnu.org/licenses/>.  */

#ifndef _PATHSTUFF_H
#define _PATHSTUFF_H

extern char *convert_Path_to_windows32(char *Path, char to_delim);
extern char *convert_vpath_to_windows32(char *Path, char to_delim);
extern char *w32ify(const char *file, int resolve);
extern char *w32_getcwd(char *buf, int len);
extern FILE* w32_fopen(const char* filename, const char* mode);

#endif
