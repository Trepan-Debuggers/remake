/* 
Set a breakpoint at a target.  With a target name, set a break before
running commands of that target.  Without argument, list all breaks.
*/
static debug_return_t 
dbg_cmd_break (char *psz_target)
{
  file_t *p_target;

  if (!psz_target || !*psz_target) {
    list_breakpoints();
    return debug_readloop;
  }
  
  p_target = lookup_file (psz_target);
  if (!p_target) {
    printf("Can't find target %s; breakpoint not set.\n", psz_target);
    return debug_cmd_error;
  }

  add_breakpoint(p_target);
  
  return debug_readloop;
}

