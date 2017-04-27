#include "debug.h"
#include "trace.h"

void
die (int status)
{
  exit (status);
}

debug_return_t enter_debugger (target_stack_node_t *p,
			       file_t *p_target, int errcode,
			       debug_enter_reason_t reason)
{
  printf("%p %p %d %u\n", p, p_target, errcode, reason);
  return continue_execution;
}

/*! Show target information: location and name. */
extern void
print_file_target_prefix (const file_t *p_target)
{
  printf("%p\n", p_target);
}
