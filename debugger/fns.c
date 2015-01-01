/*
Copyright (C) 2004, 2005, 2007, 2008, 2010, 2011
R. Bernstein <rocky@gnu.org>
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

/* Helper routines for debugger command interface. */

#include "../rule.h"
#include "../trace.h"
#include "../commands.h"
#include "../expand.h"
#include "fns.h"
#include "stack.h"
#include "debug.h"
#include "print.h"
#include "msg.h"

#ifdef HAVE_STRING_H
#include <string.h>
#endif

#ifdef HAVE_SYS_WAIT_H
#include <sys/wait.h>
#endif

#ifdef HAVE_LIBREADLINE

# ifdef bcmp
#   undef bcmp
# endif


# ifdef bzero
#   undef bzero
# endif


# ifdef bcopy
#   undef bcopy
# endif


#include <readline/readline.h>
#endif

gmk_floc *p_target_loc = NULL;
char   *psz_target_name = NULL;

/*! We use the if when we fake a line number because
   a real one hasn't been recorded on the stack. */
gmk_floc  fake_floc;

/* From readline. ?? Should this be in configure?  */
#ifndef whitespace
#define whitespace(c) (((c) == ' ') || ((c) == '\t'))
#endif

brkpt_mask_t
get_brkpt_option(const char *psz_break_type)
{
  if (is_abbrev_of (psz_break_type, "all", 1)) {
    return BRK_ALL;
  } else if (is_abbrev_of (psz_break_type, "prerequisite", 3)) {
    return BRK_BEFORE_PREREQ;
  } else if (is_abbrev_of (psz_break_type, "run", 1)) {
    return BRK_AFTER_PREREQ;
  } else if (is_abbrev_of (psz_break_type, "end", 1)) {
    return BRK_AFTER_CMD;
  } else if (is_abbrev_of (psz_break_type, "temp", 1)) {
    return BRK_TEMP;
  } else {
    dbg_errmsg("Unknown breakpoint modifier %s", psz_break_type);
    return BRK_NONE;
  }
}

/*! Parse psz_arg for a signed integer. The value is returned in
    *pi_result. If warn is true, then we'll give a warning if no
    integer found. The return value is true if parsing succeeded in
    any event..
 */
bool
get_int(const char *psz_arg, int *pi_result, bool b_warn)
{
  int i;
  char *endptr;

  if (!psz_arg || 0==strlen(psz_arg)) return 0;

  i = strtol(psz_arg, &endptr, 10);
  if (*endptr != '\0') {
    if (b_warn)
      dbg_errmsg("expecting %s to be an integer", psz_arg);
    return false;
  }
  *pi_result = i;
  return true;
}

bool
get_uint(const char *psz_arg, unsigned int *result, bool b_warn)
{
  unsigned int i;
  char *endptr;

  if (!psz_arg || 0==strlen(psz_arg)) return 0;

  i = strtol(psz_arg, &endptr, 10);
  if (*endptr != '\0') {
    if (b_warn)
      dbg_errmsg("expecting %s to be an integer", psz_arg);
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
    **ppsz_str += 1;

  /* Word starts here at first non blank character. */
  psz_word = *ppsz_str;

  /* Find end of word - next whitespace. */
  while (**ppsz_str && !whitespace (**ppsz_str))
    (*ppsz_str)++;

  if (**ppsz_str) *((*ppsz_str)++) = '\0';

  return psz_word;
}

/*!
  Return the current target from the stack or NULL
  if none set.
 */
const file_t *
get_current_target(void)
{
  if (p_stack && p_stack->p_target)
    return p_stack->p_target;
  else
    return NULL;
}


/*!
  Return the current target from the stack or NULL
  if none set.
 */
const gmk_floc *
get_current_floc(void)
{
  file_t *p_target = (file_t *) get_current_target();
  gmk_floc *p_floc;

  p_floc = &p_target->floc;

  if (p_floc->filenm && p_floc->lineno != 0)
    return p_floc;
  else
    return NULL;
}


/*! Find the target in first word of psz_args or use $@ (the current
    stack) if none.  We also allow $@ or @ explicitly as a target name
    to mean the current target on the stack. NULL is returned if a lookup
    of the target name was not found. ppsz_target is to the name
    looked up.
 */
file_t *
get_target(char **ppsz_args, /*out*/ const char **ppsz_target)
{
  if (!*ppsz_args || !**ppsz_args) {
    file_t *p_target = (file_t *) get_current_target();
    /* Use current target */
    if (p_target && p_target->name) {
      *ppsz_args = (char *) p_target->name;
    } else {
      printf(_("Default target not found here. You must supply one\n"));
      return NULL;
    }
  }

  *ppsz_target = get_word(ppsz_args);

  /* As a special case, we'll allow $@ or @ for the current target. */
  if ( 0 == strcmp("$@", *ppsz_target) || 0 == strcmp("@", *ppsz_target) ) {
    if (p_stack && p_stack->p_target && p_stack->p_target->name)
      *ppsz_target = p_stack->p_target->name;
    else {
      printf(_("No current target found for $@ - supply a target name.\n"));
      return NULL;
    }
  }

  {
    file_t *p_target = lookup_file (*ppsz_target);

    if (!p_target)
      printf(_("Target \"%s\" doesn't appear to be a target name.\n"),
	     *ppsz_target);
    return p_target;
  }
}

/*! Return true if psz_substr is an initial prefix (abbreviation) of
    psz_word. The empty string is not a valid abbreviation. */
bool
is_abbrev_of(const char* psz_substr, const char* psz_word,
	     unsigned int i_min)
{
  if (strlen(psz_substr) < i_min)
    return false;
  else {
    const char *psz = strstr(psz_word, psz_substr);
    return (psz && psz == psz_word);
  }
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

/*! Print an interpretation of the debug level mask. */
void
print_db_level(debug_level_mask_t e_debug_level)
{
  if (e_debug_level & DB_BASIC)
    printf("Basic tracing (0x%x)\n", DB_BASIC);
  if (e_debug_level & DB_TRACE)
    printf("Tracing (0x%x)\n", DB_TRACE);
  if (e_debug_level & DB_VERBOSE)
    printf("Verbose Tracing (0x%x)\n", DB_VERBOSE);
  if (e_debug_level & DB_SHELL)
    printf("Tracing shell commands 0x%x\n", DB_SHELL);
  if (e_debug_level & DB_MAKEFILES)
    printf("Tracing Rebuilding Makefiles 0x%x\n", DB_MAKEFILES);
  if (e_debug_level & DB_UPDATE_GOAL)
    printf("Tracing Goal target updates 0x%x\n", DB_MAKEFILES);
  if (e_debug_level & DB_UPDATE_GOAL)
    printf("Tracing reading Makefiles 0x%x\n", DB_READ_MAKEFILES);
  if (e_debug_level & DB_CALL)
    printf("Tracing function call and returns 0x%x\n", DB_CALL);
}

static char *reason2str[] = {
    "->",
    "..",
    "<-",
    "||",
    "rd",
    "!!",
    "--",
    "++",
    ":o"
};



/** Print where we are in the Makefile. */
void
print_debugger_location(const file_t *p_target, debug_enter_reason_t reason,
			const floc_stack_node_t *p_stack_floc)
{
  if (p_target_loc) {
    if (reason != DEBUG_NOT_GIVEN && reason != DEBUG_STACK_CHANGING)
      printf("%s ", reason2str[reason]);
    printf("(");
    if ( !p_target_loc->filenm && p_target_loc->lineno != 0
	 && p_target && p_target->name ) {
      /* We don't have file location info in the target floc, but we
	 do have it as part of the name, so use that. This happens for
	 example with we've stopped before reading a Makefile.
      */
      if (p_target->cmds) {
	gmk_floc floc;
	memcpy(&floc, &(p_target->cmds->fileinfo.filenm), sizeof(gmk_floc));
	/* HACK: is it okay to assume that the target is on the line
	   before the first command? Or should we list the line
	   that the command starts on - so we know we've faked the location?
	*/
	floc.lineno--;
	p_target_loc->filenm = floc.filenm;
	p_target_loc->lineno = floc.lineno;
	print_floc_prefix(&floc);
	printf (")\n");
      } else if (p_target->phony)
	printf("%s: .PHONY target)\n", p_target->name);
      else
	printf("%s:0)\n", p_target->name);
    } else {
      print_floc_prefix(p_target_loc);
      printf (")\n");
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

  /* Could/should generalize the below into a prompt string. */
  switch (reason)
    {
    case DEBUG_BRKPT_BEFORE_PREREQ:
    case DEBUG_STEP_HIT:
      dbg_cmd_show_exp("$@: $+", true);
      break;
    case DEBUG_STACK_CHANGING:
      break;
    default:
      dbg_cmd_show_exp("$@", true);
      break;
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

/*! See if psz_varname is $varname or $(varname) */
void
try_without_dollar(const char *psz_varname)
{
  printf("Can't find variable `%s'.\n", psz_varname);
  if (psz_varname && psz_varname[0] == '$') {
    const char *psz_nodollar = &(psz_varname[1]);
    char *psz_try = calloc(1, strlen(psz_varname));
    if (psz_nodollar && 1 == sscanf(psz_nodollar, "(%s)", psz_try)) {
      /* Remove trailing ')' */
      if ( ')' == psz_try[strlen(psz_try)-1] )
	psz_try[strlen(psz_try)-1]='\0';
      printf(_("Did you mean `%s'?\n"), psz_try);
    } else
      printf(_("Did you mean `%s'?\n"), psz_nodollar);
    free(psz_try);
  }
}

/*! Show a expression. Set "expand" to 1 if you want variable
   definitions inside the displayed value expanded.
*/
bool
dbg_cmd_show_exp (char *psz_varname, bool expand)
{
  if (!psz_varname || 0==strlen(psz_varname)) {
    printf(_("You need to supply a variable name.\n"));
    return false;
  } else {
    variable_t *p_v;
    variable_set_t *p_set = NULL;
    variable_set_list_t *p_file_vars = NULL;
    if (p_stack && p_stack->p_target && p_stack->p_target->name) {
      const char *psz_target = p_stack->p_target->name;
      file_t *p_target = lookup_file (psz_target);
      if (p_target) {
	initialize_file_variables (p_target, 0);
	set_file_variables (p_target);
	p_file_vars = p_target->variables;
	p_set = p_file_vars->set;
      }
    }
    if (p_set) {
      p_v = lookup_variable_in_set(psz_varname, strlen(psz_varname), p_set);
      if (!p_v)
	/* May be a global variable. */
	p_v = lookup_variable (psz_varname, strlen (psz_varname));
    } else {
      p_v = lookup_variable (psz_varname, strlen (psz_varname));
    }
    if (p_v) {
      if (expand) {
	print_variable_expand(p_v);
      } else
	print_variable(p_v);
    } else {
      if (expand)
	printf("%s\n", variable_expand_set(psz_varname, p_file_vars));
      else {
	try_without_dollar(psz_varname);
	return false;
      }
    }
  }
  return true;
}

void dbg_print_invocation(void)
{
  unsigned int i;
  printf("%s ", global_argv[0]);
  for (i = 1; global_argv[i]; i++) {
    printf(" %s", global_argv[i]);
  }
  printf("\n");
}

rule_t *find_rule (const char *psz_name)
{
  rule_t *r;

  for (r = pattern_rules; r != 0; r = r->next)
    {
      unsigned int i;
      for (i = 0; r->targets[i] != 0; ++i)
	if (0 == strcmp(r->targets[i], psz_name)) return r;
    }
  return NULL;
}

void chomp(char * line)
{
  unsigned int len = strlen(line);
  if (line[len-1] == '\n') line[len-1] = '\0';
}


void shell_rc_status(int rc)
{
  if (rc == -1)
    printf(_("Error: %s\n"), strerror(errno));
  else if (WEXITSTATUS(rc) != 0)
    printf(_("Warning: return code was %d\n"), WEXITSTATUS(rc));
}

/*
 * Local variables:
 * eval: (c-set-style "gnu")
 * indent-tabs-mode: nil
 * End:
 */
