/*
Copyright (C) 2005, 2007-2008, 2015, 2020 R. Bernstein <rocky@gnu.org>
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

/** debugger command stack routines. */

#include <assert.h>
#include <regex.h>
#include "break.h"
#include "cmd.h"
#include "msg.h"
#include "filedef.h"
#include "print.h"

enum breakpoint_type
{
  LINE,
  COMMAND_WATCH
};

/*! Node for an item in the target call stack */
struct breakpoint_node
{
  enum breakpoint_type type;
  file_t               *p_target;
  unsigned int         i_num;
  brkpt_mask_t         brkpt_mask;
  regex_t              regex;
  char                 *psz_text;
  breakpoint_node_t    *p_next;
};

/** Pointers to top/bottom of current breakpoint target stack */
breakpoint_node_t *p_breakpoint_top    = NULL;
breakpoint_node_t *p_breakpoint_bottom = NULL;

unsigned int i_breakpoints = 0;

/*! Add "p_target" to the list of breakpoints. Return true if
    there were no errors
*/
bool
add_breakpoint (file_t *p_target, const brkpt_mask_t brkpt_mask)
{
  breakpoint_node_t *p_new   = CALLOC (breakpoint_node_t, 1);

  if (!p_new) return false;

  /* Add breakpoint to list of breakpoints. */
  if (!p_breakpoint_top) {
    assert(!p_breakpoint_bottom);
    p_breakpoint_top            = p_breakpoint_bottom = p_new;
  } else {
    p_breakpoint_bottom->p_next = p_new;
  }
  p_breakpoint_bottom           = p_new;
  p_new->type                   = LINE;
  p_new->p_target               = p_target;
  p_new->i_num                  = ++i_breakpoints;
  p_new->brkpt_mask             = brkpt_mask;


  /* Finally, note that we are tracing this target. */
  if (p_target->tracing & (BRK_BEFORE_PREREQ & brkpt_mask)) {
    dbg_msg(_("Note: prerequisite breakpoint already set at target %s."),
            p_target->name);
  }
  if (p_target->tracing & (BRK_AFTER_PREREQ & brkpt_mask)) {
    dbg_msg(_("Note: command breakpoint already set at target %s."),
            p_target->name);
  }
  if (p_target->tracing & (BRK_AFTER_CMD & brkpt_mask)) {
    dbg_msg(_("Note: target end breakpont set at target %s."),
            p_target->name);
  }
  p_target->tracing = brkpt_mask;
  printf(_("Breakpoint %u on target `%s', mask 0x%02x"), i_breakpoints,
         p_target->name, brkpt_mask);
  if (p_target->floc.filenm)
    dbg_msg(": file %s, line %lu.", p_target->floc.filenm,
            p_target->floc.lineno);
  else
    printf(".\n");
  if (p_target->updated)
      dbg_msg("Warning: target is already updated; so it might not get stopped at again.");
  else if (p_target->updating && (brkpt_mask & (BRK_BEFORE_PREREQ | BRK_AFTER_PREREQ))) {
      dbg_msg("Warning: target is in the process of being updated;");
      dbg_msg("so it might not get stopped at again.");
  }
  return true;

}

/*! Remove breakpoint i from the list of breakpoints. Return true if
    there were no errors
*/
bool
remove_breakpoint (unsigned int i, bool silent)
{
  if (!i) {
    dbg_errmsg(_("Invalid Breakpoint number 0."));
    return false;
  }
  if (i > i_breakpoints) {
    dbg_errmsg(_("Breakpoint number %u is too high. "
                 "%u is the highest breakpoint number."), i, i_breakpoints);
    return false;
  } else {
    /* Find breakpoint i */
    breakpoint_node_t *p_prev = NULL;
    breakpoint_node_t *p;
    for (p = p_breakpoint_top; p && p->i_num != i; p = p->p_next) {
      p_prev = p;
    }

    if (p && p->i_num == i) {
      /* Delete breakpoint */
      if (!p->p_next) p_breakpoint_bottom = p_prev;
      if (p == p_breakpoint_top) p_breakpoint_top = p->p_next;

      if (p_prev) p_prev->p_next = p->p_next;

      if (p->type == COMMAND_WATCH) {
          dbg_msg(_("Command watchpoint %u cleared."), i);
          regfree(&p->regex);
          free(p->psz_text);
          free(p);
          return true;
      } else if (p->p_target->tracing) {
	p->p_target->tracing = BRK_NONE;
	dbg_msg(_("Breakpoint %u on target `%s' cleared."),
	       i, p->p_target->name);
	free(p);
	return true;
      } else {
	dbg_msg(_("No breakpoint at target `%s'; nothing cleared."),
	       p->p_target->name);
	free(p);
	return false;
      }
    } else {
      if (!silent)
        dbg_errmsg(_("No Breakpoint number %u set."), i);
      return false;
    }
  }
}

/*! List breakpoints.*/
void
list_breakpoints (void)
{
  breakpoint_node_t *p;

  if (!p_breakpoint_top) {
    dbg_msg(_("No breakpoints."));
    return;
  }

  dbg_msg(  "Num Type           Disp Enb Mask Target  Location");
  for (p = p_breakpoint_top; p; p = p->p_next) {
    if (p->type == LINE) {
      printf("%3u breakpoint     keep   y 0x%02x %s",
             p->i_num,
             p->brkpt_mask,
             p->p_target->name);
      if (p->p_target->floc.filenm) {
          printf(" at ");
          print_floc_prefix(&(p->p_target->floc));
      }
    } else {
      printf("%3u command watchpoint     %s", p->i_num, p->psz_text);
    }
    printf("\n");
  }
}

bool
add_command_watchpoint(const char *psz_regex)
{
  int re_compile_status;
  breakpoint_node_t *p_new   = CALLOC (breakpoint_node_t, 1);

  if (!p_new) return false;

  re_compile_status             = regcomp(&p_new->regex, psz_regex, REG_EXTENDED);

  // Not sure if regex_t can be memmoved, so initializing it in the allocated breakpoint_node_t.
  if (re_compile_status != 0) {
    char reg_error_message[100];
    regerror(re_compile_status, &p_new->regex, reg_error_message, sizeof(reg_error_message)-1);
    dbg_msg("Could not compile regex: %s: %s", psz_regex, reg_error_message);
    regfree(&p_new->regex);
    free(p_new);
    return false;
  }

  p_new->type                   = COMMAND_WATCH;
  p_new->psz_text               = strdup(psz_regex);
  p_new->i_num                  = ++i_breakpoints;
  p_new->p_target               = NULL;
  p_new->brkpt_mask             = 0;

  /* Add breakpoint to list of breakpoints. */
  if (!p_breakpoint_top) {
    assert(!p_breakpoint_bottom);
    p_breakpoint_top            = p_breakpoint_bottom = p_new;
  } else {
    p_breakpoint_bottom->p_next = p_new;
  }
  p_breakpoint_bottom           = p_new;

  printf(_("Watchpoint %u for regex `%s' added\n"),
         p_new->i_num, p_new->psz_text);
  return true;
}

static size_t
print_till_eol(const char *psz_line)
{
  int count = 0;
  while (psz_line[count] != 0 && psz_line[count] != '\n') {
    putchar(psz_line[count]);
    count++;
  }
  return count;
}

static void
print_underline(size_t offset, size_t length)
{
  printf ("%*s", (int) offset, "");
  for (int i = 0; i < length; i++) {
    putchar('^');
  }
  putchar('\n');
}


static void
print_command_underlined(const char *psz_command, size_t sub_offset, size_t sub_length)
{
  int tail_length;
  // TODO: Doesn't account for \t, \r.

  do {
    long line_length = print_till_eol(psz_command);
    putchar('\n');
    psz_command += line_length; // and a newline/null

    if (*psz_command == '\n') {
      psz_command++;
    }

    if (sub_offset < line_length) {
      size_t underline_length = MIN(sub_length, line_length - sub_offset);
      print_underline(sub_offset, underline_length);
      break;
    }

    sub_offset -= line_length;

    // Skip the newline
    if (sub_offset == 0) {
      // The offset can, in theory, start on newline.

      sub_length--;
      while (*psz_command == '\n') {
        putchar('\n');
        // If only \n-s need to be underlined, then underline the last of them.
        if (sub_length == 1) {
          puts ("^");
          psz_command++;
          break;
        }
        psz_command++;
        sub_length--;
      }

    } else {
      sub_offset--;
    }

  } while (*psz_command != '\0');

  tail_length = printf("%s", psz_command) - 1;
  if (tail_length > 0 && psz_command[tail_length] != '\n') {
    putchar('\n');
  }
}


static bool
any_command_breakpoint_matches(const char *psz_expanded_command)
{
  breakpoint_node_t *p;
  int match_status;

  for (p = p_breakpoint_top; p; p = p->p_next) {
    if (p->type == COMMAND_WATCH) {
      regmatch_t pmatch;
      match_status = regexec(&p->regex, psz_expanded_command, (size_t) 1, &pmatch, 0);
      if (match_status == 0) {
        // TODO: Determine which line of command it was, if it was multi-line.
        printf (_("Command matched a watchpoint %u at (%d:%d) '%s':\n%s\n"),
                p->i_num, (int) pmatch.rm_so, (int) pmatch.rm_eo, p->psz_text, psz_expanded_command);
        print_command_underlined(psz_expanded_command, pmatch.rm_so, pmatch.rm_eo - pmatch.rm_so);
        return true;
      }
    }
  }

  return false;
}

void
check_command_watchpoint(target_stack_node_t *p_call_stack, file_t *p_target, const char *psz_expanded_command)
{
  if (any_command_breakpoint_matches (psz_expanded_command)) {
    enter_debugger(p_call_stack, p_target, 0, DEBUG_WATCHPOINT);
  }
}


/*
 * Local variables:
 * eval: (c-set-style "gnu")
 * indent-tabs-mode: nil
 * End:
 */
