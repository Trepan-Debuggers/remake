/**
 *  \brief Debugger `watch` command.
 *
 * Add a "command watch" breakpoint that triggers when a command is about to be executed matches the given regex.
 **/

#include <stdbool.h>
#include "../break.h"

extern debug_return_t
dbg_cmd_watch (char *psz_regex)
{
  if (!psz_regex || !*psz_regex) {
    return debug_readloop;
  }

  add_command_watchpoint(psz_regex);

  return debug_readloop;
}


/*
 * Local variables:
 * eval: (c-set-style "gnu")
 * indent-tabs-mode: nil
 * End:
 */
