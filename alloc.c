/* Miscellaneous allocations functions for GNU Make.
Copyright (C) 1988-2014 Free Software Foundation, Inc.
This file is part of GNU Make.

GNU Make is free software; you can redistribute it and/or modify it under the
terms of the GNU General Public License as published by the Free Software
Foundation; either version 3 of the License, or (at your option) any later
version.

GNU Make is distributed in the hope that it will be useful, but WITHOUT ANY
WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR
A PARTICULAR PURPOSE.  See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License along with
this program.  If not, see <http://www.gnu.org/licenses/>.  */

#include "makeint.h"


/* Like malloc but get fatal error if memory is exhausted.  */
/* Don't bother if we're using dmalloc; it provides these for us.  */

#ifndef HAVE_DMALLOC_H

#undef xmalloc
#undef xcalloc
#undef xrealloc
#undef xstrdup

void *
xmalloc (unsigned int size)
{
  /* Make sure we don't allocate 0, for pre-ISO implementations.  */
  void *result = malloc (size ? size : 1);
  if (result == 0)
    OUT_OF_MEM();
  return result;
}


void *
xcalloc (unsigned int size)
{
  /* Make sure we don't allocate 0, for pre-ISO implementations.  */
  void *result = calloc (size ? size : 1, 1);
  if (result == 0)
    OUT_OF_MEM();
  return result;
}


void *
xrealloc (void *ptr, unsigned int size)
{
  void *result;

  /* Some older implementations of realloc() don't conform to ISO.  */
  if (! size)
    size = 1;
  result = ptr ? realloc (ptr, size) : malloc (size);
  if (result == 0)
    OUT_OF_MEM();
  return result;
}


char *
xstrdup (const char *ptr)
{
  char *result;

#ifdef HAVE_STRDUP
  result = strdup (ptr);
#else
  result = malloc (strlen (ptr) + 1);
#endif

  if (result == 0)
    OUT_OF_MEM();

#ifdef HAVE_STRDUP
  return result;
#else
  return strcpy (result, ptr);
#endif
}

#endif  /* HAVE_DMALLOC_H */

char *
xstrndup (const char *str, unsigned int length)
{
  char *result;

#ifdef HAVE_STRNDUP
  result = strndup (str, length);
  if (result == 0)
    OUT_OF_MEM();
#else
  result = xmalloc (length + 1);
  if (length > 0)
    strncpy (result, str, length);
  result[length] = '\0';
#endif

  return result;
}
