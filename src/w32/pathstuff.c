/* Path conversion for Windows pathnames.
Copyright (C) 1996-2025 Free Software Foundation, Inc.
This file is part of GNU Make.

GNU Make is free software; you can redistribute it and/or modify it under the
terms of the GNU General Public License as published by the Free Software
Foundation; either version 3 of the License, or (at your option) any later
version.

GNU Make is distributed in the hope that it will be useful, but WITHOUT ANY
WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR
A PARTICULAR PURPOSE.  See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License along with
this program.  If not, see <https://www.gnu.org/licenses/>.  */

#include "makeint.h"
#include <string.h>
#include <stdlib.h>
#include "pathstuff.h"

/*
 * Convert delimiter separated vpath to Canonical format.
 */
char *
convert_vpath_to_windows32(char *Path, char to_delim)
{
    char *etok;            /* token separator for old Path */

        /*
         * Convert all spaces to delimiters. Note that pathnames which
         * contain blanks get trounced here. Use 8.3 format as a workaround.
         */
        for (etok = Path; etok && *etok; etok++)
                if (ISBLANK ((unsigned char) *etok))
                        *etok = to_delim;

        return (convert_Path_to_windows32(Path, to_delim));
}

/*
 * Convert delimiter separated path to Canonical format.
 */
char *
convert_Path_to_windows32(char *Path, char to_delim)
{
    char *etok;            /* token separator for old Path */
    char *p;            /* points to element of old Path */

    /* is this a multi-element Path ? */
    /* FIXME: Perhaps use ":;\"" in strpbrk to convert all quotes to
       delimiters as well, as a way to handle quoted directories in
       PATH?  */
    for (p = Path, etok = strpbrk(p, ":;");
         etok;
         etok = strpbrk(p, ":;"))
        if ((etok - p) == 1) {
            if (*(etok - 1) == ';' ||
                *(etok - 1) == ':') {
                etok[-1] = to_delim;
                etok[0] = to_delim;
                p = ++etok;
                continue;    /* ignore empty bucket */
            } else if (!isalpha ((unsigned char) *p)) {
                /* found one to count, handle things like '.' */
                *etok = to_delim;
                p = ++etok;
            } else if ((*etok == ':') && ((etok = strpbrk(etok+1, ":;")) != NULL)) {
                /* found one to count, handle drive letter */
                *etok = to_delim;
                p = ++etok;
            } else
                /* all finished, force abort */
                p += strlen(p);
        } else if (*p == '"') { /* a quoted directory */
            for (p++; *p && *p != '"'; p++) /* skip quoted part */
                ;
            etok = strpbrk(p, ":;");        /* find next delimiter */
            if (etok) {
                *etok = to_delim;
                p = ++etok;
            } else
                p += strlen(p);
        } else {
            /* found another one, no drive letter */
            *etok = to_delim;
            p = ++etok;
        }

    return Path;
}

/*
 * Convert to forward slashes. Resolve to full pathname optionally
 */
char *
w32ify(const char *filename, int resolve)
{
    static char w32_path[FILENAME_MAX];
    char *p;

    if (resolve)
      {
        char *fp = _fullpath (NULL, filename, sizeof (w32_path));
        strncpy (w32_path, fp, sizeof (w32_path) - 1);
        free (fp);
      }
    else
      strncpy(w32_path, filename, sizeof (w32_path) - 1);

    /* Replace \ in w32_path with /  */
    for (p = w32_path; p && *p; p++)
      if (*p == '\\')
        *p = '/';

    return w32_path;
}

char *
getcwd_fs(char* buf, int len)
{
        char *p = getcwd(buf, len);

        if (p) {
                char *q = w32ify(buf, 0);
                strncpy(buf, q, len);
        }

        return p;
}
