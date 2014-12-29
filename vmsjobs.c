/* --------------- Moved here from job.c ---------------
   This file must be #included in job.c, as it accesses static functions.

Copyright (C) 1996-2014 Free Software Foundation, Inc.
This file is part of GNU Make.

GNU Make is free software; you can redistribute it and/or modify it under the
terms of the GNU General Public License as published by the Free Software
Foundation; either version 3 of the License, or (at your option) any later
version.

GNU Make is distributed in the hope that it will be useful, but WITHOUT ANY
WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR
A PARTICULAR PURPOSE.  See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License along with
this program.  If not, see <http://www.gnu.org/licenses/>.  */

#include <string.h>
#include <descrip.h>
#include <clidef.h>

char *vmsify (char *name, int type);

static int vms_jobsefnmask = 0;

/* Wait for nchildren children to terminate */
static void
vmsWaitForChildren(int *status)
{
  while (1)
    {
      if (!vms_jobsefnmask)
        {
          *status = 0;
          return;
        }

      *status = sys$wflor (32, vms_jobsefnmask);
    }
  return;
}

/* Set up IO redirection.  */

static char *
vms_redirect (struct dsc$descriptor_s *desc, char *fname, char *ibuf)
{
  char *fptr;
  char saved;

  ibuf++;
  while (isspace ((unsigned char)*ibuf))
    ibuf++;
  fptr = ibuf;
  while (*ibuf && !isspace ((unsigned char)*ibuf))
    ibuf++;
  saved = *ibuf;
  *ibuf = 0;
  if (strcmp (fptr, "/dev/null") != 0)
    {
      strcpy (fname, vmsify (fptr, 0));
      if (strchr (fname, '.') == 0)
        strcat (fname, ".");
    }
  desc->dsc$w_length = strlen (fname);
  desc->dsc$a_pointer = fname;
  desc->dsc$b_dtype = DSC$K_DTYPE_T;
  desc->dsc$b_class = DSC$K_CLASS_S;

  if (*fname == 0)
    printf (_("Warning: Empty redirection\n"));
  if (saved=='\0')
    return ibuf;
  *ibuf = saved;
  return --ibuf;
}


/* found apostrophe at (p-1)
   inc p until after closing apostrophe.
*/

static char *
vms_handle_apos (char *p)
{
  int alast;
  alast = 0;

  while (*p != 0)
    if (*p == '"')
      if (alast)
        {
          alast = 0;
          p++;
        }
      else
        {
          p++;
          if (*p!='"')
            break;
          alast = 1;
        }
    else
      p++;

  return p;
}

static int ctrlYPressed= 0;
/* This is called at main or AST level. It is at AST level for DONTWAITFORCHILD
   and at main level otherwise. In any case it is called when a child process
   terminated. At AST level it won't get interrupted by anything except a
   inner mode level AST.
*/
static int
vmsHandleChildTerm(struct child *child)
{
  int exit_code;
  register struct child *lastc, *c;
  int child_failed;

  vms_jobsefnmask &= ~(1 << (child->efn - 32));

  lib$free_ef (&child->efn);
  if (child->comname)
    {
      if (!ISDB (DB_JOBS) && !ctrlYPressed)
        unlink (child->comname);
      free (child->comname);
    }

  (void) sigblock (fatal_signal_mask);

  child_failed = !(child->cstatus & 1);
  if (child_failed)
    exit_code = child->cstatus;

  /* Search for a child matching the deceased one.  */
  lastc = 0;
#if defined(RECURSIVEJOBS) /* I've had problems with recursive stuff and process handling */
  for (c = children; c != 0 && c != child; lastc = c, c = c->next)
    ;
#else
  c = child;
#endif

  if (child_failed && !c->noerror && !ignore_errors_flag)
    {
      /* The commands failed.  Write an error message,
         delete non-precious targets, and abort.  */
      child_error (c, c->cstatus, 0, 0, 0);
      c->file->update_status = us_failed;
      delete_child_targets (c);
    }
  else
    {
      if (child_failed)
        {
          /* The commands failed, but we don't care.  */
          child_error (c, c->cstatus, 0, 0, 1);
          child_failed = 0;
        }

#if defined(RECURSIVEJOBS) /* I've had problems with recursive stuff and process handling */
      /* If there are more commands to run, try to start them.  */
      start_job (c);

      switch (c->file->command_state)
        {
        case cs_running:
          /* Successfully started.  */
          break;

        case cs_finished:
          if (c->file->update_status != us_success)
            /* We failed to start the commands.  */
            delete_child_targets (c);
          break;

        default:
          OS (error, NILF,
              _("internal error: '%s' command_state"), c->file->name);
          abort ();
          break;
        }
#endif /* RECURSIVEJOBS */
    }

  /* Set the state flag to say the commands have finished.  */
  c->file->command_state = cs_finished;
  notice_finished_file (c->file);

#if defined(RECURSIVEJOBS) /* I've had problems with recursive stuff and process handling */
  /* Remove the child from the chain and free it.  */
  if (lastc == 0)
    children = c->next;
  else
    lastc->next = c->next;
  free_child (c);
#endif /* RECURSIVEJOBS */

  /* There is now another slot open.  */
  if (job_slots_used > 0)
    --job_slots_used;

  /* If the job failed, and the -k flag was not given, die.  */
  if (child_failed && !keep_going_flag)
    die (exit_code);

  (void) sigsetmask (sigblock (0) & ~(fatal_signal_mask));

  return 1;
}

/* VMS:
   Spawn a process executing the command in ARGV and return its pid. */

#define MAXCMDLEN 200

/* local helpers to make ctrl+c and ctrl+y working, see below */
#include <iodef.h>
#include <libclidef.h>
#include <ssdef.h>

static int ctrlMask= LIB$M_CLI_CTRLY;
static int oldCtrlMask;
static int setupYAstTried= 0;
static unsigned short int chan= 0;

static void
reEnableAst(void)
{
  lib$enable_ctrl (&oldCtrlMask,0);
}

static int
astYHandler (void)
{
  struct child *c;
  for (c = children; c != 0; c = c->next)
    sys$delprc (&c->pid, 0, 0);
  ctrlYPressed= 1;
  kill (getpid(),SIGQUIT);
  return SS$_NORMAL;
}

static void
tryToSetupYAst(void)
{
  $DESCRIPTOR(inputDsc,"SYS$COMMAND");
  int     status;
  struct {
    short int       status, count;
    int     dvi;
  } iosb;
  unsigned short int loc_chan;

  setupYAstTried++;

  if (chan)
    loc_chan= chan;
  else
    {
      status= sys$assign(&inputDsc,&loc_chan,0,0);
      if (!(status&SS$_NORMAL))
        {
          lib$signal(status);
          return;
        }
    }
  status= sys$qiow (0, loc_chan, IO$_SETMODE|IO$M_CTRLYAST,&iosb,0,0,
                    astYHandler,0,0,0,0,0);
  if (status==SS$_NORMAL)
    status= iosb.status;
  if (status!=SS$_NORMAL)
    {
      if (!chan)
        sys$dassgn(loc_chan);
      if (status!=SS$_ILLIOFUNC && status!=SS$_NOPRIV)
        lib$signal(status);
      return;
    }

  /* called from AST handler ? */
  if (setupYAstTried>1)
    return;
  if (atexit(reEnableAst))
    fprintf (stderr,
             _("-warning, you may have to re-enable CTRL-Y handling from DCL.\n"));
  status= lib$disable_ctrl (&ctrlMask, &oldCtrlMask);
  if (!(status&SS$_NORMAL))
    {
      lib$signal(status);
      return;
    }
  if (!chan)
    chan = loc_chan;
}

static int
nextnl(char *cmd, int l)
{
  int instring;
  instring = 0;
  while (cmd[l])
    {
      if (cmd[l]=='"')
        instring = !instring;
      else if (cmd[l]=='\n' && !instring)
        return ++l;
      ++l;
    }
  return l;
}
int
child_execute_job (char *argv, struct child *child)
{
  int i;
  static struct dsc$descriptor_s cmddsc;
  static struct dsc$descriptor_s pnamedsc;
  static struct dsc$descriptor_s ifiledsc;
  static struct dsc$descriptor_s ofiledsc;
  static struct dsc$descriptor_s efiledsc;
  int have_redirection = 0;
  int have_append = 0;
  int have_newline = 0;

  int spflags = CLI$M_NOWAIT;
  int status;
  char *cmd = alloca (strlen (argv) + 512), *p, *q;
  char ifile[256], ofile[256], efile[256];
  int comnamelen;
  char procname[100];
  int in_string;

  /* Parse IO redirection.  */

  ifile[0] = 0;
  ofile[0] = 0;
  efile[0] = 0;
  child->comname = NULL;

  DB (DB_JOBS, ("child_execute_job (%s)\n", argv));

  while (isspace ((unsigned char)*argv))
    argv++;

  if (*argv == 0)
    return 0;

  sprintf (procname, "GMAKE_%05x", getpid () & 0xfffff);
  pnamedsc.dsc$w_length = strlen(procname);
  pnamedsc.dsc$a_pointer = procname;
  pnamedsc.dsc$b_dtype = DSC$K_DTYPE_T;
  pnamedsc.dsc$b_class = DSC$K_CLASS_S;

  in_string = 0;
  /* Handle comments and redirection.
     For ONESHELL, the redirection must be on the first line. Any other
     redirection token is handled by DCL, that is, the pipe command with
     redirection can be used, but it should not be used on the first line
     for ONESHELL. */
  for (p = argv, q = cmd; *p; p++, q++)
    {
      if (*p == '"')
        in_string = !in_string;
      if (in_string)
        {
          *q = *p;
          continue;
        }
      switch (*p)
        {
        case '#':
          *p-- = 0;
          *q-- = 0;
          break;
        case '\\':
          p++;
          if (*p == '\n')
            p++;
          if (isspace ((unsigned char)*p))
            {
              do { p++; } while (isspace ((unsigned char)*p));
              p--;
            }
          *q = *p;
          break;
        case '<':
          if (have_newline==0)
            {
              p = vms_redirect (&ifiledsc, ifile, p);
              *q = ' ';
              have_redirection = 1;
            }
          else
            *q = *p;
          break;
        case '>':
          if (have_newline==0)
            {
              have_redirection = 1;
              if (*(p-1) == '2')
                {
                  q--;
                  if (strncmp (p, ">&1", 3) == 0)
                    {
                      p += 2;
                      strcpy (efile, "sys$output");
                      efiledsc.dsc$w_length = strlen(efile);
                      efiledsc.dsc$a_pointer = efile;
                      efiledsc.dsc$b_dtype = DSC$K_DTYPE_T;
                      efiledsc.dsc$b_class = DSC$K_CLASS_S;
                    }
                  else
                    p = vms_redirect (&efiledsc, efile, p);
                }
              else
                {
                  if (*(p+1) == '>')
                    {
                      have_append = 1;
                      p += 1;
                    }
                  p = vms_redirect (&ofiledsc, ofile, p);
                }
              *q = ' ';
            }
          else
            *q = *p;
          break;
        case '\n':
          have_newline++;
        default:
          *q = *p;
          break;
        }
    }
  *q = *p;
  while (isspace ((unsigned char)*--q))
    *q = '\0';


#define VMS_EMPTY_ECHO "write sys$output \"\""
  if (have_newline == 0)
    {
      /* multiple shells */
      if (strncmp(cmd, "builtin_", 8) == 0)
        {
          child->pid = 270163;
          child->efn = 0;
          child->cstatus = 1;

          DB(DB_JOBS, (_("BUILTIN [%s][%s]\n"), cmd, cmd + 8));

          p = cmd + 8;

          if ((*(p) == 'c') && (*(p + 1) == 'd')
              && ((*(p + 2) == ' ') || (*(p + 2) == '\t')))
            {
              p += 3;
              while ((*p == ' ') || (*p == '\t'))
                p++;
              DB(DB_JOBS, (_("BUILTIN CD %s\n"), p));
              if (chdir(p))
                return 0;
              else
                return 1;
            }
          else if ((*(p) == 'e')
              && (*(p+1) == 'c')
              && (*(p+2) == 'h')
              && (*(p+3) == 'o')
              && ((*(p+4) == ' ') || (*(p+4) == '\t') || (*(p+4) == '\0')))
            {
              /* This is not a real builtin, it is a built in pre-processing
                 for the VMS/DCL echo (write sys$output) to ensure the to be echoed
                 string is correctly quoted (with the DCL quote character '"'). */
              char *vms_echo;
              p += 4;
              if (*p == '\0')
                cmd = VMS_EMPTY_ECHO;
              else
                {
                  p++;
                  while ((*p == ' ') || (*p == '\t'))
                    p++;
                  if (*p == '\0')
                    cmd = VMS_EMPTY_ECHO;
                  else
                    {
                      vms_echo = alloca(strlen(p) + sizeof VMS_EMPTY_ECHO);
                      strcpy(vms_echo, VMS_EMPTY_ECHO);
                      vms_echo[sizeof VMS_EMPTY_ECHO - 2] = '\0';
                      strcat(vms_echo, p);
                      strcat(vms_echo, "\"");
                      cmd = vms_echo;
                    }
                }
              DB (DB_JOBS, (_("BUILTIN ECHO %s->%s\n"), p, cmd));
            }
          else
            {
              printf(_("Unknown builtin command '%s'\n"), cmd);
              fflush(stdout);
              return 0;
            }
        }
      /* expand ':' aka 'do nothing' builtin for bash and friends */
      else if (cmd[0]==':' && cmd[1]=='\0')
        {
          cmd = "continue";
        }
    }
  else
    {
      /* todo: expand ':' aka 'do nothing' builtin for bash and friends */
      /* For 'one shell' expand all the
           builtin_echo
         to
           write sys$output ""
         where one is  ......7 bytes longer.
         At the same time ensure that the echo string is properly terminated.
         For that, allocate a command buffer big enough for all possible expansions
         (have_newline is the count), then expand, copy and terminate. */
      char *tmp_cmd;
      int nloff = 0;
      int vlen = 0;
      int clen = 0;
      int inecho;

      tmp_cmd = alloca(strlen(cmd) + (have_newline + 1) * 7 + 1);
      tmp_cmd[0] = '\0';
      inecho = 0;
      while (cmd[nloff])
        {
          if (inecho)
            {
              if (clen < nloff - 1)
                {
                  memcpy(&tmp_cmd[vlen], &cmd[clen], nloff - clen - 1);
                  vlen += nloff - clen - 1;
                  clen = nloff;
                }
              inecho = 0;
              tmp_cmd[vlen] = '"';
              vlen++;
              tmp_cmd[vlen] = '\n';
              vlen++;
            }
          if (strncmp(&cmd[nloff], "builtin_", 8) == 0)
            {
              /* ??? */
              child->pid = 270163;
              child->efn = 0;
              child->cstatus = 1;

              DB (DB_JOBS, (_("BUILTIN [%s][%s]\n"), &cmd[nloff], &cmd[nloff+8]));
              p = &cmd[nloff + 8];
              if ((*(p) ==        'e')
                  && (*(p + 1) == 'c')
                  && (*(p + 2) == 'h')
                  && (*(p + 3) == 'o')
                  && ((*(p + 4) == ' ') || (*(p + 4) == '\t') || (*(p + 4) == '\0')))
                {
                  if (clen < nloff - 1)
                    {
                      memcpy(&tmp_cmd[vlen], &cmd[clen], nloff - clen - 1);
                      vlen += nloff - clen - 1;
                      clen = nloff;
                      if (inecho)
                        {
                          inecho = 0;
                          tmp_cmd[vlen] = '"';
                          vlen++;
                        }
                      tmp_cmd[vlen] = '\n';
                      vlen++;
                    }
                  inecho = 1;
                  p += 4;
                  while ((*p == ' ') || (*p == '\t'))
                    p++;
                  clen = p - cmd;
                  memcpy(&tmp_cmd[vlen], VMS_EMPTY_ECHO,
                      sizeof VMS_EMPTY_ECHO - 2);
                  vlen += sizeof VMS_EMPTY_ECHO - 2;
                }
              else
                {
                  printf (_("Builtin command is unknown or unsupported in .ONESHELL: '%s'\n"), &cmd[nloff]);
                  fflush(stdout);
                  return 0;
                }
            }
          nloff = nextnl(cmd, nloff + 1);
        }
      if (clen < nloff)
        {
          memcpy(&tmp_cmd[vlen], &cmd[clen], nloff - clen);
          vlen += nloff - clen;
          clen = nloff;
          if (inecho)
            {
              inecho = 0;
              tmp_cmd[vlen] = '"';
              vlen++;
            }
        }

      tmp_cmd[vlen] = '\0';

      cmd = tmp_cmd;
    }

#ifdef USE_DCL_COM_FILE
  /* Enforce the creation of a command file.
     Then all the make environment variables are written as DCL symbol
     assignments into the command file as well, so that they are visible
     in the sub-process but do not affect the current process.
     Further, this way DCL reads the input stream and therefore does
     'forced' symbol substitution, which it doesn't do for one-liners when
     they are 'lib$spawn'ed. */
#else
  /* Create a *.com file if either the command is too long for
     lib$spawn, or the command contains a newline, or if redirection
     is desired. Forcing commands with newlines into DCLs allows to
     store search lists on user mode logicals.  */
  if (strlen (cmd) > MAXCMDLEN
      || (have_redirection != 0)
      || (have_newline != 0))
#endif
    {
      FILE *outfile;
      char c;
      char *sep;
      int alevel = 0;   /* apostrophe level */
      int tmpstrlen;
      char *tmpstr;
      if (strlen (cmd) == 0)
        {
          printf (_("Error, empty command\n"));
          fflush (stdout);
          return 0;
        }

      outfile = output_tmpfile (&child->comname, "sys$scratch:CMDXXXXXX.COM");
      /*                                          012345678901234567890 */
#define TMP_OFFSET 12
#define TMP_LEN 9
      if (outfile == 0)
        pfatal_with_name (_("fopen (temporary file)"));
      comnamelen = strlen (child->comname);
      tmpstr = &child->comname[TMP_OFFSET];
      tmpstrlen = TMP_LEN;
      /* The whole DCL "script" is executed as one action, and it behaves as
         any DCL "script", that is errors stop it but warnings do not. Usually
         the command on the last line, defines the exit code.  However, with
         redirections there is a prolog and possibly an epilog to implement
         the redirection.  Both are part of the script which is actually
         executed. So if the redirection encounters an error in the prolog,
         the user actions will not run; if in the epilog, the user actions
         ran, but output is not captured. In both error cases, the error of
         redirection is passed back and not the exit code of the actions. The
         user should be able to enable DCL "script" verification with "set
         verify". However, the prolog and epilog commands are not shown. Also,
         if output redirection is used, the verification output is redirected
         into that file as well. */
      fprintf (outfile, "$ %.*s_1 = \"''f$verify(0)'\"\n", tmpstrlen, tmpstr);
      if (ifile[0])
        {
          fprintf (outfile, "$ assign/user %s sys$input\n", ifile);
          DB (DB_JOBS, (_("Redirected input from %s\n"), ifile));
          ifiledsc.dsc$w_length = 0;
        }

      if (efile[0])
        {
          fprintf (outfile, "$ define sys$error %s\n", efile);
          DB (DB_JOBS, (_("Redirected error to %s\n"), efile));
          efiledsc.dsc$w_length = 0;
        }

      if (ofile[0])
        if (have_append)
          {
            fprintf (outfile, "$ define sys$output %.*s\n", comnamelen-3, child->comname);
            fprintf (outfile, "$ on error then $ goto %.*s\n", tmpstrlen, tmpstr);
            DB (DB_JOBS, (_("Append output to %s\n"), ofile));
            ofiledsc.dsc$w_length = 0;
          }
        else
          {
            fprintf (outfile, "$ define sys$output %s\n", ofile);
            DB (DB_JOBS, (_("Redirected output to %s\n"), ofile));
            ofiledsc.dsc$w_length = 0;
          }
#ifdef USE_DCL_COM_FILE
      /* Export the child environment into DCL symbols */
      if (child->environment != 0)
        {
          char **ep = child->environment;
          char *valstr;
          while (*ep != 0)
            {
              valstr = strchr(*ep, '=');
              if (valstr == NULL)
                continue;
              fprintf(outfile, "$ %.*s=\"%s\"\n", valstr - *ep, *ep,
                  valstr + 1);
              ep++;
            }
        }
#endif
      fprintf (outfile, "$ %.*s_ = f$verify(%.*s_1)\n", tmpstrlen, tmpstr, tmpstrlen, tmpstr);

      /* TODO: give 78 a name! Whether 78 is a good number is another question.
         Trim, split and write the command lines.
         Splitting of a command is done after 78 output characters at an
         appropriate place (after strings, after comma or space and
         before slash): appending a hyphen indicates that the DCL command
         is being continued.
         Trimming is to skip any whitespace around - including - a
         leading $ from the command to ensure writing exactly one "$ "
         at the beginning of the line of the output file. Trimming is
         done when a new command is seen, indicated by a '\n' (outside
         of a string).
         The buffer so far is written and reset, when a new command is
         seen, when a split was done and at the end of the command.
         Only for ONESHELL there will be several commands separated by
         '\n'. But there can always be multiple continuation lines. */
      p = sep = q = cmd;
      for (c = '\n'; c; c = *q++)
        {
          switch (c)
          {
          case '\n':
            if (q > p)
              {
                fwrite(p, 1, q - p, outfile);
                p = q;
              }
            fputc('$', outfile);
            fputc(' ', outfile);
            while (isspace((unsigned char) *p))
              p++;
            if (*p == '$')
              p++;
            while (isspace((unsigned char) *p))
              p++;
            q = sep = p;
            break;
          case '"':
            q = vms_handle_apos(q);
            sep = q;
            break;
          case ',':
          case ' ':
            sep = q;
            break;
          case '/':
          case '\0':
            sep = q - 1;
            break;
          default:
            break;
          }
          if (sep - p > 78)
            {
              /* Enough stuff for a line. */
              fwrite(p, 1, sep - p, outfile);
              p = sep;
              if (*sep)
                {
                  /* The command continues.  */
                  fputc('-', outfile);
                }
              fputc('\n', outfile);
            }
        }

      if (*p)
        {
          fwrite(p, 1, --q - p, outfile);
          fputc('\n', outfile);
        }

      if (have_append)
        {
          fprintf (outfile, "$ %.*s: ! 'f$verify(0)\n", tmpstrlen, tmpstr);
          fprintf (outfile, "$ %.*s_2 = $status\n", tmpstrlen, tmpstr);
          fprintf (outfile, "$ on error then $ exit\n");
          fprintf (outfile, "$ deassign sys$output\n");
          if (efile[0])
            fprintf (outfile, "$ deassign sys$error\n");
          fprintf (outfile, "$ append:=append\n");
          fprintf (outfile, "$ delete:=delete\n");
          fprintf (outfile, "$ append/new %.*s %s\n", comnamelen-3, child->comname, ofile);
          fprintf (outfile, "$ delete %.*s;*\n", comnamelen-3, child->comname);
          fprintf (outfile, "$ exit '%.*s_2 + (0*f$verify(%.*s_1))\n", tmpstrlen, tmpstr, tmpstrlen, tmpstr);
          DB (DB_JOBS, (_("Append %.*s and cleanup\n"), comnamelen-3, child->comname));
        }

      fclose (outfile);

      sprintf (cmd, "$ @%s", child->comname);

      DB (DB_JOBS, (_("Executing %s instead\n"), cmd));
    }

  cmddsc.dsc$w_length = strlen(cmd);
  cmddsc.dsc$a_pointer = cmd;
  cmddsc.dsc$b_dtype = DSC$K_DTYPE_T;
  cmddsc.dsc$b_class = DSC$K_CLASS_S;

  child->efn = 0;
  while (child->efn < 32 || child->efn > 63)
    {
      status = lib$get_ef ((unsigned long *)&child->efn);
      if (!(status & 1))
        {
          if (child->comname)
            {
              if (!ISDB (DB_JOBS))
                unlink (child->comname);
              free (child->comname);
            }
          return 0;
        }
    }

  sys$clref (child->efn);

  vms_jobsefnmask |= (1 << (child->efn - 32));

  /*
    LIB$SPAWN  [command-string]
    [,input-file]
    [,output-file]
    [,flags]
    [,process-name]
    [,process-id] [,completion-status-address] [,byte-integer-event-flag-num]
    [,AST-address] [,varying-AST-argument]
    [,prompt-string] [,cli] [,table]
  */

#ifndef DONTWAITFORCHILD
  /*
   * Code to make ctrl+c and ctrl+y working.
   * The problem starts with the synchronous case where after lib$spawn is
   * called any input will go to the child. But with input re-directed,
   * both control characters won't make it to any of the programs, neither
   * the spawning nor to the spawned one. Hence the caller needs to spawn
   * with CLI$M_NOWAIT to NOT give up the input focus. A sys$waitfr
   * has to follow to simulate the wanted synchronous behaviour.
   * The next problem is ctrl+y which isn't caught by the crtl and
   * therefore isn't converted to SIGQUIT (for a signal handler which is
   * already established). The only way to catch ctrl+y, is an AST
   * assigned to the input channel. But ctrl+y handling of DCL needs to be
   * disabled, otherwise it will handle it. Not to mention the previous
   * ctrl+y handling of DCL needs to be re-established before make exits.
   * One more: At the time of LIB$SPAWN signals are blocked. SIGQUIT will
   * make it to the signal handler after the child "normally" terminates.
   * This isn't enough. It seems reasonable for simple command lines like
   * a 'cc foobar.c' spawned in a subprocess but it is unacceptable for
   * spawning make. Therefore we need to abort the process in the AST.
   *
   * Prior to the spawn it is checked if an AST is already set up for
   * ctrl+y, if not one is set up for a channel to SYS$COMMAND. In general
   * this will work except if make is run in a batch environment, but there
   * nobody can press ctrl+y. During the setup the DCL handling of ctrl+y
   * is disabled and an exit handler is established to re-enable it.
   * If the user interrupts with ctrl+y, the assigned AST will fire, force
   * an abort to the subprocess and signal SIGQUIT, which will be caught by
   * the already established handler and will bring us back to common code.
   * After the spawn (now /nowait) a sys$waitfr simulates the /wait and
   * enables the ctrl+y be delivered to this code. And the ctrl+c too,
   * which the crtl converts to SIGINT and which is caught by the common
   * signal handler. Because signals were blocked before entering this code
   * sys$waitfr will always complete and the SIGQUIT will be processed after
   * it (after termination of the current block, somewhere in common code).
   * And SIGINT too will be delayed. That is ctrl+c can only abort when the
   * current command completes. Anyway it's better than nothing :-)
   */

  if (!setupYAstTried)
    tryToSetupYAst();
  status = lib$spawn (&cmddsc,                                  /* cmd-string */
                      (ifiledsc.dsc$w_length == 0)?0:&ifiledsc, /* input-file */
                      (ofiledsc.dsc$w_length == 0)?0:&ofiledsc, /* output-file */
                      &spflags,                                 /* flags */
                      &pnamedsc,                                /* proc name */
                      &child->pid, &child->cstatus, &child->efn,
                      0, 0,
                      0, 0, 0);
  if (status & 1)
    {
      status= sys$waitfr (child->efn);
      vmsHandleChildTerm(child);
    }
#else
  status = lib$spawn (&cmddsc,
                      (ifiledsc.dsc$w_length == 0)?0:&ifiledsc,
                      (ofiledsc.dsc$w_length == 0)?0:&ofiledsc,
                      &spflags,
                      &pnamedsc,
                      &child->pid, &child->cstatus, &child->efn,
                      vmsHandleChildTerm, child,
                      0, 0, 0);
#endif

  if (!(status & 1))
    {
      printf (_("Error spawning, %d\n") ,status);
      fflush (stdout);
      switch (status)
        {
        case 0x1c:
          errno = EPROCLIM;
          break;
        default:
          errno = EFAIL;
        }
    }

  return (status & 1);
}
