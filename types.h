/* $Id: types.h,v 1.8 2005/12/25 10:08:35 rockyb Exp $
Miscellaneous types
Copyright (c) 2005, 2008  Rocky Bernstein <rocky@panix.com>

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

/** \file types.h 
 *
 *  \brief Miscellaneous types
 */


#ifndef MAKE_TYPES_H
#define MAKE_TYPES_H

#include "config.h"
#include "make.h"

#if defined(HAVE_STDINT_H)
# include <stdint.h>
#elif defined(HAVE_INTTYPES_H)
# include <inttypes.h>
#endif /* HAVE_STDINT_H */

#if defined(HAVE_STDBOOL_H)
# include <stdbool.h>
#else
   /* ISO/IEC 9899:1999 <stdbool.h> missing -- enabling workaround */
  
#   define false   0
#   define true    1
#   define bool uint8_t
#endif /*HAVE_STDBOOL_H*/

typedef unsigned long int lineno_t;

#define NILF ((struct floc *)0)

typedef struct child             child_t;
typedef struct commands          commands_t;
typedef struct dep               dep_t;
typedef struct file              file_t;
typedef struct floc              floc_t;
typedef struct nameseq           nameseq_t;
typedef struct pattern_var       pattern_var_t;
typedef struct pspec             pspec_t;
typedef struct rule              rule_t;
typedef struct stringlist        stringlist_t;
typedef struct variable          variable_t;
typedef enum   variable_origin   variable_origin_t;
typedef struct variable_set      variable_set_t;
typedef struct variable_set_list variable_set_list_t;

#define	CALLOC(t, n) ((t *) calloc (sizeof (t), (n)))

#endif /*TYPES_H*/
