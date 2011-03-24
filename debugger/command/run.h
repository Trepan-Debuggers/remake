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
