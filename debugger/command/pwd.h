/* Print working directory. */
static debug_return_t 
dbg_cmd_pwd(void) 
{
  char *psz_args = psz_debugger_args;
  if (!psz_args || 0==strlen(psz_args)) {
    char wd[300];
    if (NULL == getcwd (wd, sizeof(wd))) {
      printf (_("cannot get current directory %s\n"), strerror(errno));
    } else {
      printf (_("Working directory %s.\n"), wd);
    }
  } else {
    printf(_("The \"pwd\" does not take an argument: %s\n"), psz_args);
  }

  return debug_readloop;
}
