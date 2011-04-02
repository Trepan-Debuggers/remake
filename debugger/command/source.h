/* Continue until the next command to be executed. */
#define DEPENDS_COMMANDS " depends commands"

#include <wordexp.h>

static debug_return_t 
dbg_cmd_source(char *psz_filename)
{
  if (psz_filename && *psz_filename) {
    wordexp_t p;

    wordexp(psz_filename, &p, 0);
    if (1 != p.we_wordc) {
      dbg_errmsg("Expansion of %s doesn't lead to a single filename.", 
                 psz_filename);
    } else {
      debug_return_t debug_return = debug_readloop;
      const char *psz_expanded_file  = p.we_wordv[0];
      FILE *p_source_file = fopen(psz_expanded_file, "r");
        
      if (p_source_file != NULL) {
        char line[2048];
        while (!feof(p_source_file) && debug_return == debug_readloop)
          {
            char *s;
            char *shut_up_gcc_warning = fgets(line, 2048, p_source_file);
            if (feof(p_source_file)) break;
            chomp(shut_up_gcc_warning);
            if ( *(s=stripwhite(line)) ) debug_return=execute_line(s);
          }
        fclose(p_source_file);
      } else
        dbg_errmsg("error reading file %s (expanded to %s):\n\t%s", 
                   psz_filename, psz_expanded_file, strerror(errno));
    }
    wordfree(&p);
  }  else {
    dbg_errmsg("Expecting a file name");
  }
  return debug_readloop;
}

static void
dbg_cmd_source_init(unsigned int c) 
{
  short_command[c].func = &dbg_cmd_source;
  short_command[c].use = _("source FILENAME");
  short_command[c].doc = _("Execute debugger commands from FILENAME.\n");
}


/* 
 * Local variables:
 * eval: (c-set-style "gnu")
 * indent-tabs-mode: nil
 * End:
 */
