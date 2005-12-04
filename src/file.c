/* $Id: file.c,v 1.8 2005/12/04 13:22:48 rockyb Exp $
Target file hash table management for GNU Make.
Copyright (C) 1988, 1989, 1990, 1991, 1992, 1993, 1994, 1995, 1996, 1997,
2002, 2004, 2005 Free Software Foundation, Inc.
This file is part of GNU Make.

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

#include "print.h"

#include <assert.h>

#include "dep.h"
#include "commands.h"
#include "debug.h"

/* The below variable is to make sure the enumerations are accessible
   in a debugger. */
print_target_mask_t debugger_enum_mask;

/* Hash table of files the makefile knows how to make.  */

static unsigned long
file_hash_1 (const void *key)
{
  return_ISTRING_HASH_1 (((file_t const *) key)->hname);
}

static unsigned long
file_hash_2 (const void *key)
{
  return_ISTRING_HASH_2 (((file_t const *) key)->hname);
}

static int
file_hash_cmp (const void *x, const void *y)
{
  return_ISTRING_COMPARE (((file_t const *) x)->hname,
			  ((file_t const *) y)->hname);
}

#ifndef	FILE_BUCKETS
#define FILE_BUCKETS	1007
#endif

/*! Whether or not .SECONDARY with no prerequisites was given.  */
int all_secondary = 0;

/*! Access the hash table of all file records.
   lookup_file  given a name, return the file_t * for that name,
           or nil if there is none.
   enter_file   similar, but create one if there is none. 
 */
file_t *
lookup_file (char *name)
{
  file_t *f;
  file_t file_key;
#if defined(VMS) && !defined(WANT_CASE_SENSITIVE_TARGETS)
  char *lname, *ln;
#endif

  assert (*name != '\0');

  /* This is also done in parse_file_seq, so this is redundant
     for names read from makefiles.  It is here for names passed
     on the command line.  */
  while (name[0] == '.' && name[1] == '/' && name[2] != '\0')
    {
      name += 2;
      while (*name == '/')
	/* Skip following slashes: ".//foo" is "foo", not "/foo".  */
	++name;
    }

  if (*name == '\0')
    /* It was all slashes after a dot.  */
    name = "./";

  file_key.hname = name;
  f = (file_t *) hash_find_item (&files, &file_key);
  return f;
}

file_t *
enter_file (char *name, const floc_t *p_floc)
{
  file_t *f;
  file_t *new;
  file_t **file_slot;
  file_t file_key;

  assert (*name != '\0');

  file_key.hname = name;
  file_slot = (file_t **) hash_find_slot (&files, &file_key);
  f = *file_slot;
  if (! HASH_VACANT (f) && !f->double_colon)
    {
      return f;
    }

  new = CALLOC(file_t, 1);
  new->name = new->hname = name;
  new->update_status = -1;
  new->tracing = 0;
  if (p_floc) {
    new->floc = *p_floc;
  } else {
    new->floc.lineno = 0;
    new->floc.filenm = NULL;
  }

  if (HASH_VACANT (f))
    hash_insert_at (&files, new, file_slot);
  else
    {
      /* There is already a double-colon entry for this file.  */
      new->double_colon = f;
      while (f->prev != 0)
	f = f->prev;
      f->prev = new;
    }

  return new;
}

void
free_file (file_t *p_name)
{
  free(p_name);
}


/*! Rename FILE to NAME.  This is not as simple as resetting
   the `name' member, since it must be put in a new hash bucket,
   and possibly merged with an existing file called NAME. 
*/
void
rename_file (file_t *from_file, char *to_hname)
{
  rehash_file (from_file, to_hname);
  while (from_file)
    {
      from_file->name = from_file->hname;
      from_file = from_file->prev;
    }
}

/*!
 Rehash FILE to NAME.  This is not as simple as resetting
 the `hname' member, since it must be put in a new hash bucket,
 and possibly merged with an existing file called NAME.  
*/
void
rehash_file (file_t *from_file, char *to_hname)
{
  file_t file_key;
  file_t **file_slot;
  file_t *to_file;
  file_t *deleted_file;
  file_t *f;

  file_key.hname = to_hname;
  if (0 == file_hash_cmp (from_file, &file_key))
    return;

  file_key.hname = from_file->hname;
  while (from_file->renamed != 0)
    from_file = from_file->renamed;
  if (file_hash_cmp (from_file, &file_key))
    /* hname changed unexpectedly */
    abort ();

  deleted_file = hash_delete (&files, from_file);
  if (deleted_file != from_file)
    /* from_file isn't the one stored in files */
    abort ();

  file_key.hname = to_hname;
  file_slot = (file_t **) hash_find_slot (&files, &file_key);
  to_file = *file_slot;

  from_file->hname = to_hname;
  for (f = from_file->double_colon; f != 0; f = f->prev)
    f->hname = to_hname;

  if (HASH_VACANT (to_file))
    hash_insert_at (&files, from_file, file_slot);
  else
    {
      /* TO_FILE already exists under TO_HNAME.
	 We must retain TO_FILE and merge FROM_FILE into it.  */

      if (from_file->cmds != 0)
	{
	  if (to_file->cmds == 0)
	    to_file->cmds = from_file->cmds;
	  else if (from_file->cmds != to_file->cmds)
	    {
	      /* We have two sets of commands.  We will go with the
		 one given in the rule explicitly mentioning this name,
		 but give a message to let the user know what's going on.  */
	      if (to_file->cmds->fileinfo.filenm != 0)
                error (&from_file->cmds->fileinfo,
		       _("Commands were specified for file `%s' at %s:%lu,"),
		       from_file->name, to_file->cmds->fileinfo.filenm,
		       to_file->cmds->fileinfo.lineno);
	      else
		error (&from_file->cmds->fileinfo,
		       _("Commands for file `%s' were found by implicit rule search,"),
		       from_file->name);
	      error (&from_file->cmds->fileinfo,
		     _("but `%s' is now considered the same file as `%s'."),
		     from_file->name, to_hname);
	      error (&from_file->cmds->fileinfo,
		     _("Commands for `%s' will be ignored in favor of those for `%s'."),
		     to_hname, from_file->name);
	    }
	}

      /* Merge the dependencies of the two files.  */

      if (to_file->deps == 0)
	to_file->deps = from_file->deps;
      else
	{
	  dep_t *deps = to_file->deps;
	  while (deps->next != 0)
	    deps = deps->next;
	  deps->next = from_file->deps;
	}

      merge_variable_set_lists (&to_file->variables, from_file->variables);

      if (to_file->double_colon && from_file->is_target && !from_file->double_colon)
	fatal (NILF, _("can't rename single-colon `%s' to double-colon `%s'"),
	       from_file->name, to_hname);
      if (!to_file->double_colon  && from_file->double_colon)
	{
	  if (to_file->is_target)
	    fatal (NILF, _("can't rename double-colon `%s' to single-colon `%s'"),
		   from_file->name, to_hname);
	  else
	    to_file->double_colon = from_file->double_colon;
	}

      if (from_file->last_mtime > to_file->last_mtime)
	/* %%% Kludge so -W wins on a file that gets vpathized.  */
	to_file->last_mtime = from_file->last_mtime;

      to_file->mtime_before_update = from_file->mtime_before_update;

#define MERGE(field) to_file->field |= from_file->field
      MERGE (precious);
      MERGE (tried_implicit);
      MERGE (updating);
      MERGE (updated);
      MERGE (is_target);
      MERGE (cmd_target);
      MERGE (phony);
      MERGE (ignore_vpath);
#undef MERGE

      from_file->renamed = to_file;
    }
}


/*!
  Remove all nonprecious intermediate files.
  If SIG is nonzero, this was caused by a fatal signal,
  meaning that a different message will be printed, and
  the message will go to stderr rather than stdout.  
*/
void
remove_intermediates (int sig)
{
  file_t **file_slot;
  file_t **file_end;
  int doneany = 0;

  /* If there's no way we will ever remove anything anyway, punt early.  */
  if (question_flag || touch_flag || all_secondary)
    return;

  if (sig && just_print_flag)
    return;

  file_slot = (file_t **) files.ht_vec;
  file_end = file_slot + files.ht_size;
  for ( ; file_slot < file_end; file_slot++)
    if (! HASH_VACANT (*file_slot))
      {
	file_t *f = *file_slot;
        /* Is this file eligible for automatic deletion?
           Yes, IFF: it's marked intermediate, it's not secondary, it wasn't
           given on the command-line, and it's either a -include makefile or
           it's not precious.  */
	if (f->intermediate && (f->dontcare || !f->precious)
	    && !f->secondary && !f->cmd_target)
	  {
	    int status;
	    if (f->update_status == -1)
	      /* If nothing would have created this file yet,
		 don't print an "rm" command for it.  */
	      continue;
	    if (just_print_flag)
	      status = 0;
	    else
	      {
		status = unlink (f->name);
		if (status < 0 && errno == ENOENT)
		  continue;
	      }
	    if (!f->dontcare)
	      {
		if (sig)
		  error (NILF, _("*** Deleting intermediate file `%s'"), f->name);
		else
		  {
		    if (! doneany)
		      DB (DB_BASIC, (_("Removing intermediate files...\n")));
		    if (!silent_flag)
		      {
			if (! doneany)
			  {
			    fputs ("rm ", stdout);
			    doneany = 1;
			  }
			else
			  putchar (' ');
			fputs (f->name, stdout);
			fflush (stdout);
		      }
		  }
		if (status < 0)
		  perror_with_name ("unlink: ", f->name);
	      }
	  }
      }

  if (doneany && !sig)
    {
      putchar ('\n');
      fflush (stdout);
    }
}

/* Set the `command_state' member of FILE and all its `also_make's.  */

void
set_command_state (file_t *file, enum cmd_state state)
{
  dep_t *d;

  file->command_state = state;

  for (d = file->also_make; d != 0; d = d->next)
    d->file->command_state = state;
}

/* Convert an external file timestamp to internal form.  */

FILE_TIMESTAMP
file_timestamp_cons (const char *fname, time_t s, int ns)
{
  int offset = ORDINARY_MTIME_MIN + (FILE_TIMESTAMP_HI_RES ? ns : 0);
  FILE_TIMESTAMP product = (FILE_TIMESTAMP) s << FILE_TIMESTAMP_LO_BITS;
  FILE_TIMESTAMP ts = product + offset;

  if (! (s <= FILE_TIMESTAMP_S (ORDINARY_MTIME_MAX)
	 && product <= ts && ts <= ORDINARY_MTIME_MAX))
    {
      char buf[FILE_TIMESTAMP_PRINT_LEN_BOUND + 1];
      ts = s <= OLD_MTIME ? ORDINARY_MTIME_MIN : ORDINARY_MTIME_MAX;
      file_timestamp_sprintf (buf, ts);
      error (NILF, _("%s: Timestamp out of range; substituting %s"),
	     fname ? fname : _("Current time"), buf);
    }

  return ts;
}

/* Return the current time as a file timestamp, setting *RESOLUTION to
   its resolution.  */
FILE_TIMESTAMP
file_timestamp_now (int *resolution)
{
  int r;
  time_t s;
  int ns;

  /* Don't bother with high-resolution clocks if file timestamps have
     only one-second resolution.  The code below should work, but it's
     not worth the hassle of debugging it on hosts where it fails.  */
#if FILE_TIMESTAMP_HI_RES
# if HAVE_CLOCK_GETTIME && defined CLOCK_REALTIME
  {
    struct timespec timespec;
    if (clock_gettime (CLOCK_REALTIME, &timespec) == 0)
      {
	r = 1;
	s = timespec.tv_sec;
	ns = timespec.tv_nsec;
	goto got_time;
      }
  }
# endif
# if HAVE_GETTIMEOFDAY
  {
    struct timeval timeval;
    if (gettimeofday (&timeval, 0) == 0)
      {
	r = 1000;
	s = timeval.tv_sec;
	ns = timeval.tv_usec * 1000;
	goto got_time;
      }
  }
# endif
#endif

  r = 1000000000;
  s = time ((time_t *) 0);
  ns = 0;

#if FILE_TIMESTAMP_HI_RES
 got_time:
#endif
  *resolution = r;
  return file_timestamp_cons (0, s, ns);
}

/* Place into the buffer P a printable representation of the file
   timestamp TS.  */
void
file_timestamp_sprintf (char *p, FILE_TIMESTAMP ts)
{
  time_t t = FILE_TIMESTAMP_S (ts);
  struct tm *tm = localtime (&t);

  if (tm)
    sprintf (p, "%04d-%02d-%02d %02d:%02d:%02d",
	     tm->tm_year + 1900, tm->tm_mon + 1, tm->tm_mday,
	     tm->tm_hour, tm->tm_min, tm->tm_sec);
  else if (t < 0)
    sprintf (p, "%ld", (long) t);
  else
    sprintf (p, "%lu", (unsigned long) t);
  p += strlen (p);

  /* Append nanoseconds as a fraction, but remove trailing zeros.
     We don't know the actual timestamp resolution, since clock_getres
     applies only to local times, whereas this timestamp might come
     from a remote filesystem.  So removing trailing zeros is the
     best guess that we can do.  */
  sprintf (p, ".%09d", FILE_TIMESTAMP_NS (ts));
  p += strlen (p) - 1;
  while (*p == '0')
    p--;
  p += *p != '.';

  *p = '\0';
}


/*! 
Print the data base of files.
*/
void
print_target (const void *item)
{
  file_t *p_target = (file_t *) item;
  print_target_props(p_target, PRINT_TARGET_ALL);
}

/*! Print some or all properties of the data base of files.
*/
void
print_target_props (file_t *p_target, print_target_mask_t i_mask)
{
  dep_t *d;
  dep_t *ood = 0;

  putchar ('\n');
  if (!p_target->is_target)
    puts (_("# Not a target:"));
  printf ("%s:%s", p_target->name, p_target->double_colon ? ":" : "");

  if (i_mask & PRINT_TARGET_DEPEND) {
    
    /* Print all normal dependencies; note any order-only deps.  */
    for (d = p_target->deps; d != 0; d = d->next)
      if (! d->ignore_mtime)
	printf (" %s", dep_name (d));
      else if (! ood)
	ood = d;
  }
  
  if (i_mask & PRINT_TARGET_ORDER) {
    /* Print order-only deps, if we have any.  */
    if (ood)
      {
	printf (" | %s", dep_name (ood));
	for (d = ood->next; d != 0; d = d->next)
	  if (d->ignore_mtime)
	    printf (" %s", dep_name (d));
      }
  }
  
  putchar ('\n');

  if (i_mask & PRINT_TARGET_ATTRS) {
    
    if (p_target->precious)
      puts (_("#  Precious file (prerequisite of .PRECIOUS)."));
    if (p_target->phony)
      puts (_("#  Phony target (prerequisite of .PHONY)."));
    if (p_target->cmd_target)
      puts (_("#  Command-line target."));
    if (p_target->dontcare)
      puts (_("#  A default, MAKEFILES, or -include/sinclude makefile."));
    puts (p_target->tried_implicit
	  ? _("#  Implicit rule search has been done.")
	  : _("#  Implicit rule search has not been done."));
    if (p_target->stem != 0)
      printf (_("#  Implicit/static pattern stem: `%s'\n"), p_target->stem);
    if (p_target->intermediate)
      puts (_("#  File is an intermediate prerequisite."));
    if (p_target->also_make != 0)
      {
	fputs (_("#  Also makes:"), stdout);
	for (d = p_target->also_make; d != 0; d = d->next)
	  printf (" %s", dep_name (d));
	putchar ('\n');
      }
  }

  if (i_mask & PRINT_TARGET_TIME) {
    
    if (p_target->last_mtime == UNKNOWN_MTIME)
      puts (_("#  Modification time never checked."));
    else if (p_target->last_mtime == NONEXISTENT_MTIME)
      puts (_("#  File does not exist."));
    else if (p_target->last_mtime == OLD_MTIME)
      puts (_("#  File is very old."));
    else
      {
	char buf[FILE_TIMESTAMP_PRINT_LEN_BOUND + 1];
	file_timestamp_sprintf (buf, p_target->last_mtime);
	printf (_("#  Last modified %s\n"), buf);
      }
    puts (p_target->updated
	  ? _("#  File has been updated.")
	  : _("#  File has not been updated."));
  }

  if (i_mask & PRINT_TARGET_STATE) {
    
    switch (p_target->command_state)
      {
      case cs_running:
	puts (_("#  Commands currently running (THIS IS A BUG)."));
	break;
      case cs_deps_running:
	puts (_("#  Dependencies commands running (THIS IS A BUG)."));
	break;
      case cs_not_started:
      case cs_finished:
	switch (p_target->update_status)
	  {
	  case -1:
	    break;
	  case 0:
	    puts (_("#  Successfully updated."));
	    break;
	  case 1:
	    assert (question_flag);
	    puts (_("#  Needs to be updated (-q is set)."));
	    break;
	  case 2:
	    puts (_("#  Failed to be updated."));
	    break;
	  default:
	    puts (_("#  Invalid value in `update_status' member!"));
	    fflush (stdout);
	    fflush (stderr);
	    abort ();
	  }
	break;
      default:
	puts (_("#  Invalid value in `command_state' member!"));
	fflush (stdout);
	fflush (stderr);
	abort ();
      }
  }
  

  if (p_target->variables != 0 && i_mask & PRINT_TARGET_VARS)
    print_file_variables (p_target, i_mask & PRINT_TARGET_VARS_HASH);

  if (p_target->cmds != 0 && i_mask & PRINT_TARGET_CMDS)
    print_commands (p_target, p_target->cmds, false);

  if (p_target->cmds != 0 && i_mask & PRINT_TARGET_CMDS_EXP)
    print_commands (p_target, p_target->cmds, true);

  if (p_target->prev && i_mask & PRINT_TARGET_PREV)
    print_target_props (p_target->prev, i_mask);
}

void
print_file_data_base (void)
{
  puts (_("\n# Files"));

  hash_map (&files, print_target);

  fputs (_("\n# files hash-table stats:\n# "), stdout);
  hash_print_stats (&files, stdout);
}

#define EXPANSION_INCREMENT(_l)  ((((_l) / 500) + 1) * 500)

char *
build_target_list (char *value)
{
  static unsigned long last_targ_count = 0;

  if (files.ht_fill != last_targ_count)
    {
      unsigned long max = EXPANSION_INCREMENT (strlen (value));
      unsigned long len;
      char *p;
      file_t **fp = (file_t **) files.ht_vec;
      file_t **end = &fp[files.ht_size];

      /* Make sure we have at least MAX bytes in the allocated buffer.  */
      value = xrealloc (value, max);

      p = value;
      len = 0;
      for (; fp < end; ++fp)
        if (!HASH_VACANT (*fp) && (*fp)->is_target)
          {
            file_t *f = *fp;
            int l = strlen (f->name);

            len += l + 1;
            if (len > max)
              {
                unsigned long off = p - value;

                max += EXPANSION_INCREMENT (l + 1);
                value = xrealloc (value, max);
                p = &value[off];
              }

            memmove (p, f->name, l);
            p += l;
            *(p++) = ' ';
          }
      *(p-1) = '\0';

      last_targ_count = files.ht_fill;
    }

  return value;
}

void
init_hash_files (void)
{
  hash_init (&files, 1000, file_hash_1, file_hash_2, file_hash_cmp);
}

/* EOF */
