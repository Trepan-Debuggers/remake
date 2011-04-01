/* Restart/run program. */
static debug_return_t 
dbg_cmd_run(char *psz_arg)
{
  char **ppsz_argv = global_argv;
  const char *psz_make_cmd = global_argv[0];
  int rc;
  printf("Changing directory to %s and restarting...\n", 
	 directory_before_chdir);
  rc = chdir (directory_before_chdir);
  if (psz_arg && strlen(psz_arg)) {
    unsigned int len = strlen(global_argv[0]) + strlen(psz_arg) + 2;
    char *psz_full_args = CALLOC(char, len);
    snprintf(psz_full_args, len, "%s %s", global_argv[0], psz_arg);
    ppsz_argv = buildargv(psz_full_args);
    free(psz_full_args);
  }
  execvp (psz_make_cmd, ppsz_argv);
  /* NOT USED: */
  return debug_readloop;
}

static void
dbg_cmd_run_init(unsigned int c) 
{
  short_command[c].func = &dbg_cmd_run;
  short_command[c].use = _("Run Makefile from the beginning");
  short_command[c].doc = 
    _("Run Makefile from the beginning.\n"
      "You may specify arguments to give it.\n"
      "With no arguments, uses arguments last specified (with \"run\")");
}

/* 
 * Local variables:
 *  c-file-style: "gnu"
 *  indent-tabs-mode: nil
 * End:
 */
