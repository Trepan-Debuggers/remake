/* Comment line - ingore text on line. */
static debug_return_t 
dbg_cmd_chdir (char *psz_args)
{
  if (!psz_args || 0==strlen(psz_args)) {
    printf(_("Argument required (new working directory).\n"));
  } else {
    if ( 0 != chdir(psz_args) ) {
      printf("%s: %s\n", psz_args, strerror(1));
    } else {
      printf (_("Working directory %s.\n"), psz_args);
    }
  }
  return debug_readloop;
}
