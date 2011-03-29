/* Continue until the next command to be executed. */
#define DEPENDS_COMMANDS " depends commands"
static debug_return_t 
dbg_cmd_source(char *psz_filename)
{
  if (psz_filename && *psz_filename) {
    FILE *p_source_file = fopen(psz_filename, "r");
    debug_return_t debug_return = debug_readloop;
    if (p_source_file != NULL) {
      char line[2048];
      while (!feof(p_source_file) && debug_return == debug_readloop)
	{
	  char *s;
	  (void) fgets(line, 2048, p_source_file);
	  if (feof(p_source_file)) break;
	  chomp(line);
	  if ( *(s=stripwhite(line)) ) debug_return=execute_line(s);
	}
      fclose(p_source_file);
    } else
      printf("error reading file %s: %s\n", psz_filename, strerror(errno));
  }  else {
    printf("Expecting a file name\n");
  }
  return debug_readloop;
}

static void
dbg_cmd_source_init(void) 
{
  short_command['<'].func = &dbg_cmd_source;
  short_command['<'].use = _("source FILENAME");
  short_command['<'].doc = _("Execute debugger commands from FILENAME.\n");
}


/* 
 * Local variables:
 * eval: (c-set-style "gnu")
 * indent-tabs-mode: nil
 * End:
 */
