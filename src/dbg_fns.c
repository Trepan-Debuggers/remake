/* $Id: dbg_fns.c,v 1.3 2005/11/27 17:41:17 rockyb Exp $
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

/* Helper rutines for debugger command interface. */

#include "config.h"
#include "dbg_fns.h"
#include "debug.h"
#include "print.h"
#include "trace.h"

#ifdef HAVE_STRING_H
#include <string.h>
#endif

#ifdef HAVE_LIBREADLINE
#include <readline/readline.h>
#endif

floc_t *p_target_loc = NULL;
char   *psz_target_name = NULL;

/*! We use the if when we fake a line number because
   a real one hasn't been recorded on the stack. */
floc_t  fake_floc;

/* From readline. ?? Should this be in configure?  */
#ifndef whitespace
#define whitespace(c) (((c) == ' ') || ((c) == '\t'))
#endif

/*! Parse psz_arg for a signed integer. The value is returned in 
    *pi_result and bool indicates whether the paring succeeded.
 */
bool
get_int(const char *psz_arg, int *pi_result) 
{
  int i;
  char *endptr;
  
  if (!psz_arg || 0==strlen(psz_arg)) return 0;

  i = strtol(psz_arg, &endptr, 10);
  if (*endptr != '\0') {
    printf("expecting %s to be an integer\n", psz_arg);
    return false;
  }
  *pi_result = i;
  return true;
}

bool
get_uint(const char *psz_arg, unsigned int *result) 
{
  unsigned int i;
  char *endptr;
  
  if (!psz_arg || 0==strlen(psz_arg)) return 0;

  i = strtol(psz_arg, &endptr, 10);
  if (*endptr != '\0') {
    printf("expecting %s to be an integer\n", psz_arg);
    return false;
  }
  *result = i;
  return true;
}

/* Find the next "word" - skip leading blanks and the "word" is the
   largest non-blank characters after that. ppsz_str is modified to
   point after the portion returned and also the region initially
   pointed to by ppsz_str is modified so that word is zero-byte
   termintated.
 */
char *
get_word(char **ppsz_str) 
{
  char *psz_word;
  
  /* Skip leading blanks. */
  while (**ppsz_str && whitespace (**ppsz_str))
    **ppsz_str++;

  /* Word starts here at first non blank character. */
  psz_word = *ppsz_str;

  /* Find end of word - next whitespace. */
  while (**ppsz_str && !whitespace (**ppsz_str))
    (*ppsz_str)++;

  if (**ppsz_str) *((*ppsz_str)++) = '\0';

  return psz_word;
}

/*! Return true if psz_substr is an initial prefix (abbreviation) of
    psz_word. The empty string is not a valid abbreviation. */
bool
is_abbrev_of(const char* psz_substr, const char* psz_word) 
{
  char *psz = strstr(psz_word, psz_substr);
  return (psz && psz == psz_word);
}

/*! toggle var on or on or off depending on psz_onoff */    
void 
on_off_toggle(const char *psz_onoff, int *var) 
{
  if (strcmp (psz_onoff, "on") == 0)
    *var = 1;
  else if (strcmp (psz_onoff, "off") == 0)
    *var = 0;
  else if (strcmp (psz_onoff, "toggle") == 0)
    *var = !*var;
  else 
    printf(_("expecting \"on\", \"off\", or \"toggle\"; got \"%s\" \n"),
	   psz_onoff);
}

/** Print where we are in the Makefile. */
void 
print_debugger_location(const file_t *p_target, 
			const floc_stack_node_t *p_stack_floc)
{
  if (p_target_loc) {
    if ( !p_target_loc->filenm && !p_target_loc->lineno 
	 && p_target->name ) {
      /* We don't have file location info in the target floc, but we
	 do have it as part of the name, so use that. This happens for
	 example with we've stopped before reading a Makefile.
      */
      printf("\n(%s:0)\n", p_target->name);
    } else {
      printf("\n(");
      print_floc_prefix(p_target_loc);
      printf ("): %s\n", psz_target_name);
    }
  } else if (p_stack_floc && p_stack_floc->p_floc) {
      printf("\n(");
      print_floc_prefix(p_stack_floc->p_floc);
      printf (")\n");
  } else if (p_stack_floc_top && p_stack_floc_top->p_floc) {
      printf("\n(");
      print_floc_prefix(p_stack_floc_top->p_floc);
      printf (")\n");
  }
}

/** Strip whitespace from the start and end of STRING.  Return a pointer
   into STRING. */
char *
stripwhite (char *string)
{
  char *s, *t;

  for (s = string; whitespace (*s); s++)
    ;
    
  if (*s == 0)
    return (s);

  t = s + strlen (s) - 1;
  while (t > s && whitespace (*t))
    t--;
  *++t = '\0';

  return s;
}

/* Show if i_bool is "on" or "off" */
char *
var_to_on_off(int i_bool) 
{
  return i_bool ? "on" : "off";
}
