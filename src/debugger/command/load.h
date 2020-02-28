/* Continue until the next command to be executed. */
#define DEPENDS_COMMANDS " depends commands"

#include <wordexp.h>
#include "read.h"
#include "filedef.h"

static debug_return_t
dbg_cmd_load(char *psz_filename)
{
  if (psz_filename && *psz_filename) {
    wordexp_t p;
    char *psz_expanded_file;
    struct goaldep *p_goaldep;

    wordexp(psz_filename, &p, 0);
    if (0 == p.we_wordc) {
      struct stat stat_buf;
      int ret = stat(psz_filename, &stat_buf);
      if (ret != 0) {
        dbg_errmsg("Can't find file %s:\n\t%s", psz_filename, strerror(errno));
        return debug_cmd_error;
      }
      psz_expanded_file = psz_filename;

    } else if (1 != p.we_wordc) {
      dbg_errmsg("Expansion of %s doesn't lead to a single filename. \n"
                 "Got %zu matches",
                 psz_filename, p.we_wordc);
      return debug_cmd_error;

    } else {
      psz_expanded_file = p.we_wordv[0];
    }

    snapped_deps = 0;
    p_goaldep = eval_makefile(psz_expanded_file, 0);

    if (p_goaldep == NULL) {
      dbg_errmsg("error reading Makefile %s (expanded to %s):",
                 psz_filename, psz_expanded_file);
      snapped_deps = 1;
      return debug_cmd_error;
    }  else {
      snap_deps();
      free_goaldep(p_goaldep);
    }
  } else {
    dbg_errmsg("load command expects a filename\n");
  }
  return debug_readloop;
}

static void
dbg_cmd_load_init(unsigned int c)
{
  short_command[c].func = &dbg_cmd_load;
  short_command[c].use = _("load Makefile");
  short_command[c].doc = _("Read in Makefile FILENAME.\n");
}


/*
 * Local variables:
 * eval: (c-set-style "gnu")
 * indent-tabs-mode: nil
 * End:
 */
