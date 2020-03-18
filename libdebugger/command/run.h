/*
Copyright (C) 2004-2005, 2007-2009, 2011, 2020 R. Bernstein
<rocky@gnu.org>
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

/** \file libdebugger/command/run.h
 *
 *  \brief Debugger command to restart remake
 */

static debug_return_t
dbg_cmd_run(char *psz_arg)
{
  char * const *ppsz_argv = (char * const *)global_argv;
  const char *psz_make_cmd = global_argv[0];
  printf("Changing directory to %s and restarting...\n",
	 directory_before_chdir);

  if (chdir(directory_before_chdir) == -1)
    fprintf(stderr, "changing working directory; %s\n",
            strerror(errno));

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
  short_command[c].use = _("run [ARGS]");
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
