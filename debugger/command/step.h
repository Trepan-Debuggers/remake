/* Step execution until another stopping point is reached Argument N
   means do this N times (or until there's another reason to stop. */

static debug_return_t 
dbg_cmd_step (void)
{
  char *psz_arg = psz_debugger_args;
  if (!psz_arg || !*psz_arg) {
    i_debugger_stepping = 1;
    i_debugger_nexting  = 0;
    return continue_execution;
  } 
  if (get_uint(psz_arg, &i_debugger_stepping)) 
    return continue_execution;
  else 
    return continue_execution;
}

static void
dbg_cmd_step_init(void) 
{
  short_command['s'].func = &dbg_cmd_step;
  short_command['s'].use = _("step [AMOUNT]");
  short_command['s'].doc = 
    _("Step execution until another stopping point is reached.\n"
      "Argument AMOUNT means do this AMOUNT times (or until there's another\n"
      "reason to stop.");
}


/* 
 * Local variables:
 * eval: (c-set-style "gnu")
 * indent-tabs-mode: nil
 * End:
 */
