/* Miscellaneous global declarations and portability cruft for GNU Make.
Copyright (C) 2005 Free Software Foundation, Inc.
This file is part of GNU Make.

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

#ifndef MAKE_TYPES_H
#define MAKE_TYPES_H

# if defined(HAVE_STDBOOL_H)
#  include <stdbool.h>
# else
   /* ISO/IEC 9899:1999 <stdbool.h> missing -- enabling workaround */
  
#   define false   0
#   define true    1
#   define bool uint8_t
# endif /*HAVE_STDBOOL_H*/

typedef unsigned long int lineno_t;

/** File location. Used in reporting where we are.*/
struct floc
  {
    char    *filenm;
    lineno_t lineno;
  };

typedef struct floc floc_t;


#endif /*MAKE_H*/
