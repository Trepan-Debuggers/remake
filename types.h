/*
Miscellaneous types
Copyright (c) 2005, 2008, 2011  Rocky Bernstein <rocky@gnu.org>

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


#ifndef REMAKE_TYPES_H
#define REMAKE_TYPES_H

#include "config.h"
#include "make.h"

#define UNUSED_ARGUMENT(x) (void)x

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

/** Debugger breakpoint type */
typedef enum {
  BRK_NONE           = 0x00, /**< Mask when none of the below are set. */
  BRK_BEFORE_PREREQ  = 0x01, /**< Stop after prequisites checking done */
  BRK_AFTER_PREREQ   = 0x02, /**< Stop after prequisites checking done */
  BRK_AFTER_CMD      = 0x04, /**< Stop after running commands */
  BRK_ALL            = (BRK_BEFORE_PREREQ|BRK_AFTER_PREREQ|BRK_AFTER_CMD),
  BRK_TEMP           = 0x08, /**< Temporary or one-time breakpoint */
} breakpoint_mask_t;

typedef  unsigned int brkpt_mask_t;

typedef unsigned long int lineno_t;

typedef struct child             child_t;
typedef struct commands          commands_t;
typedef struct dep               dep_t;
typedef struct file              file_t;
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

#endif /*REMAKE_TYPES_H*/
/*
 * Local variables:
 * eval: (c-set-style "gnu")
 * indent-tabs-mode: nil
 * End:
 */

// Regular font with color
#define TXTBLK "\e[0;30m" // Black
#define TXTRED "\e[0;31m" // Red
#define TXTGRN "\e[0;32m" // Green
#define TXTYLW "\e[0;33m" // Yellow
#define TXTBLU "\e[0;34m" // Blue
#define TXTPUR "\e[0;35m" // Purple
#define TXTCYN "\e[0;36m" // Cyan
#define TXTWHT "\e[0;37m" // White
// Bold font with color
#define BLDBLK "\e[1;30m" // Black
#define BLDRED "\e[1;31m" // Red
#define BLDGRN "\e[1;32m" // Green
#define BLDYLW "\e[1;33m" // Yellow
#define BLDBLU "\e[1;34m" // Blue
#define BLDPUR "\e[1;35m" // Purple
#define BLDCYN "\e[1;36m" // Cyan
#define BLDWHT "\e[1;37m" // White
// With Underline
#define UNKBLK "\e[4;30m" // Black
#define UNDRED "\e[4;31m" // Red
#define UNDGRN "\e[4;32m" // Green
#define UNDYLW "\e[4;33m" // Yellow
#define UNDBLU "\e[4;34m" // Blue
#define UNDPUR "\e[4;35m" // Purple
#define UNDCYN "\e[4;36m" // Cyan
#define UNDWHT "\e[4;37m" // White
// Background color
#define BAKBLK "\e[40m"   // Black
#define BAKRED "\e[41m"   // Red
#define BADGRN "\e[42m"   // Green
#define BAKYLW "\e[43m"   // Yellow
#define BAKBLU "\e[44m"   // Blue
#define BAKPUR "\e[45m"   // Purple
#define BAKCYN "\e[46m"   // Cyan
#define BAKWHT "\e[47m"   // White
// Reset
#define TXTRST "\e[0m"    // Text Reset
