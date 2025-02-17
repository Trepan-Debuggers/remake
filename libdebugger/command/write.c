/*
Copyright (C) 2011, 2017, 2020 R. Bernstein  <rocky@gnu.org>
This file is part of GNU Make (remake variant).

GNU Make is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2, or (at your option)
any later version.

GNU Make is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with GNU Make; see the file COPYING.  If not, write to
the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
Boston, MA 02111-1307, USA.  */

#include "../../src/trace.h"
#include "../../src/commands.h"
#include "../../src/dep.h"
#include "../../src/variable.h"

#include "../cmd.h"
#include "../fns.h"

/**
 *  \brief Debugger `write` command.
 *
 * Write commands associated with a given target.
 **/

#include "../../src/trace.h"

extern debug_return_t
dbg_cmd_write(char *psz_args)
{
  file_t *p_target = NULL;
  const char *psz_target = NULL;
  int b_stdout = 0;

  p_target = get_target(&psz_args, &psz_target);
  if (p_target) {
    variable_t *p_v = lookup_variable ("SHELL", strlen ("SHELL"));
    char *psz_filename = NULL;
    FILE *outfd;
    char *s;

    if (! p_target->cmds || ! p_target->cmds->commands) {
      printf(_("Target \"%s\" doesn't have commands associated with it.\n"),
	     psz_target);
      return debug_readloop;
    }

    s = p_target->cmds->commands;

    /* FIXME: should get directory from a variable, e.g. TMPDIR */

    if (psz_args && *psz_args) {
      if (strcmp (psz_args, "here") == 0)
	b_stdout = 1;
      else
	psz_filename = strdup(psz_args);
    } else {
      /* Create target from the basename of the target name. */
      char *psz_target_basename = strrchr(psz_target, '/');
      if (!psz_target_basename)
	psz_target_basename = (char *) psz_target;
      else
	psz_target_basename++; /* Skip delimiter */
      psz_filename = xcalloc(strlen(psz_target_basename) + 10);
      snprintf(psz_filename, MAX_FILE_LENGTH, "/tmp/%s.sh",
	       psz_target_basename);
    }

    /* Skip leading space, MAKE's command prefixes:
          echo suppression  @,
	  ignore-error  -,
	  and recursion +
    */
    while (*s != '\0')
      {
	switch (*s) {
	case '@':
	case '+':
	case '-':
	  ++s;
	  break;
	default:
	  if (!isblank ((unsigned char)*s))
	    goto found_begin;
	  ++s;
	}
      }

  found_begin:
    if ( '\0' == *s ) {
      printf(_("Null command string parsed\n"));
    } else {
      dep_t *d;
      dep_t *ood = 0;

      if (b_stdout)
	outfd = stdout;
      else if (!(outfd = fopen (psz_filename, "w"))) {
	perror ("write target");
	free(psz_filename);
	return debug_readloop;
      }

      if (p_v) {
	fprintf(outfd, "#!%s\n", variable_expand(p_v->value));
      }

      if (!p_target->floc.filenm && p_target->cmds->fileinfo.filenm) {
	/* Fake the location based on the commands - it's better than
	   nothing...
	*/
	fprintf(outfd, "## %s/%s:%lu\n", starting_directory,
		p_target->cmds->fileinfo.filenm,
		p_target->cmds->fileinfo.lineno-1);
      } else {
	fprintf(outfd, "## %s/%s:%lu\n", starting_directory,
		p_target->floc.filenm, p_target->floc.lineno);
      }

      /* FIXME: this code duplicates some code in file.c:
         print_target_props. DRY. */
      fprintf (outfd,
               "## %s:%s", p_target->name,
               p_target->double_colon ? ":" : "");

      /* Print all normal dependencies; note any order-only deps.  */
      for (d = p_target->deps; d != 0; d = d->next)
        if (! d->ignore_mtime) {
          fprintf (outfd, " %s", dep_name (d));
        } else if (! ood)
          ood = d;

      /* Print order-only deps, if we have any.  */
      if (ood)
        {
          fprintf (outfd, " | %s", dep_name (ood));
          for (d = ood->next; d != 0; d = d->next)
            if (d->ignore_mtime)
              fprintf (outfd, " %s", dep_name (d));
        }

      fprintf (outfd, "\n");

      {
	char wd[300];
	if (getcwd (wd, sizeof(wd))) {
	  fprintf(outfd, "\n#cd %s\n", wd);
	}
      }

      initialize_file_variables (p_target, 0);
      set_file_variables (p_target, p_target->stem);

      {
#if 0
	commands_t cmds;
	char **lines;
	unsigned int i;
	memcpy(&cmds, p_target->cmds, sizeof(cmds));
	cmds.command_lines = NULL;
	chop_commands (&cmds);
	lines = xmalloc (cmds.ncommand_lines * sizeof (char *));
	expand_command_lines(&cmds, lines, p_target);
	for (i = 0; i < cmds.ncommand_lines; ++i)
	  {
	    fprintf (outfd, "%s\n", lines[i]);
	    free (lines[i]);
	  }
	free (lines);
#else
	char *line = allocated_variable_expand_for_file (s, p_target);
	fprintf (outfd, "%s\n", line);
	free(line);
#endif
      }

      if (!b_stdout) {
	struct stat buf;
	if (0 == fstat(fileno(outfd), &buf)) {
	  mode_t mode = buf.st_mode;
	  if (buf.st_mode & S_IRUSR) mode |= S_IXUSR;
	  if (buf.st_mode & S_IRGRP) mode |= S_IXGRP;
	  if (buf.st_mode & S_IROTH) mode |= S_IXOTH;
	  if (0 != fchmod(fileno(outfd), mode)) {
	    printf(_("Can't set execute mode on \"%s\".\n"), psz_filename);
	  }
	}
	fclose(outfd);
	printf(_("File \"%s\" written.\n"), psz_filename);
	free(psz_filename);
      }
    }
  }
  return debug_readloop;
}


/*
 * Local variables:
 * eval: (c-set-style "gnu")
 * indent-tabs-mode: nil
 * End:
 */
