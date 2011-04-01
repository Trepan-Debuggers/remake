/* Set a variable definition without variable references but don't 
   expand variable references in the value part of psz_string. */
static debug_return_t 
dbg_cmd_setq(char *psz_string) 
{
  dbg_cmd_set_var(psz_string, 0);
  return debug_readloop;
}

static void
dbg_cmd_setq_init(unsigned int c) 
{
  short_command[c].func = &dbg_cmd_setq;
  short_command[c].use  = _("setq *variable* VALUE");
  short_command[c].doc  = 
    _("Set MAKE variable to value. Variable definitions\n"
      "\tinside VALUE are not expanded before assignment occurs.");
}



/* 
 * Local variables:
 * eval: (c-set-style "gnu")
 * indent-tabs-mode: nil
 * End:
 */

