/* Continue running program. */
static debug_return_t 
dbg_cmd_continue (char *psz_arg)
{
  if (psz_arg && *psz_arg) {
    if (debug_cmd_error == dbg_cmd_break(psz_arg)) {
      printf(_("Not continuing under these circumstatnces.\n"));
      return debug_cmd_error;
    }
  }

  i_debugger_stepping = 0;
  i_debugger_nexting  = 0;
  return continue_execution;
}
