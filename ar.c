/* Interface to `ar' archives for GNU Make.
Copyright (C) 1988, 1989, 1990, 1991, 1992, 1993 Free Software Foundation, Inc.
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
the Free Software Foundation, 675 Mass Ave, Cambridge, MA 02139, USA.  */

#include "make.h"

#ifndef	NO_ARCHIVES

#include "filedef.h"
#include "dep.h"
#include <fnmatch.h>

/* Defined in arscan.c.  */
extern long int ar_scan PARAMS ((char *archive, long int (*function) (), long int arg));
extern int ar_member_touch PARAMS ((char *arname, char *memname));
extern int ar_name_equal PARAMS ((char *name, char *mem, int truncated));


/* Return nonzero if NAME is an archive-member reference, zero if not.
   An archive-member reference is a name like `lib(member)'.
   If a name like `lib((entry))' is used, a fatal error is signaled at
   the attempt to use this unsupported feature.  */

int
ar_name (name)
     char *name;
{
  char *p = index (name, '('), *end = name + strlen (name) - 1;
  
  if (p == 0 || p == name || *end != ')')
    return 0;

  if (p[1] == '(' && end[-1] == ')')
    fatal ("attempt to use unsupported feature: `%s'", name);

  return 1;
}


/* Parse the archive-member reference NAME into the archive and member names.
   Put the malloc'd archive name in *ARNAME_P if ARNAME_P is non-nil;
   put the malloc'd member name in *MEMNAME_P if MEMNAME_P is non-nil.  */

void
ar_parse_name (name, arname_p, memname_p)
     char *name, **arname_p, **memname_p;
{
  char *p = index (name, '('), *end = name + strlen (name) - 1;

  if (arname_p != 0)
    *arname_p = savestring (name, p - name);

  if (memname_p != 0)
    *memname_p = savestring (p + 1, end - (p + 1));
}  
 
#ifdef VMS
#include <lbrdef.h>
#include <mhddef.h>
#include <credef.h>
#include <descrip.h>
#include <ctype.h>
#if __DECC
#include <lbr$routines.h>
#endif

#define uppercasify(str) {char *str1; for (str1 = str; *str1; str1++) *str1 = _toupper(*str1);}

#define LBR$_KEYNOTFND 2527330 /* This isn't in any .h file anywhere so I got it from a MACRO library */

time_t
ar_member_date (name)
     char *name;
{
  static char *arname;
  static char *memname;
  char *p,*q;
  long int val;
  static struct {
    struct mhddef mhd;
    struct credef cre;
    char garbage[256];
  } buf;
  int func=LBR$C_READ,
      type=LBR$C_TYP_OBJ,
      rfa[2],
      lidx,
      status;
  $DESCRIPTOR(bufdesc,(char *)&buf);
  $DESCRIPTOR(libdesc,arname);
  $DESCRIPTOR(moddesc,memname);

  /* This "file" is an archive member.  */
  p = index (name, '(');
  arname = savestring (name, p - name);
  val = strlen (p) - 2;
/*
  if (val > 15)
    val = 15;
*/
  memname = savestring (p + 1, val);
#ifdef OLDWAY
  p = rindex (memname, ')');
  if (p != 0) {
      q = rindex(p,'.');
      if (q)
	*q = '\0'; /* to get rid of extension */
  }
#endif

  q = rindex(memname,'.');
  if (q)
    *q = '\0'; /* to get rid of extension */

  uppercasify(memname);

  /* Make sure we know the modtime of the archive itself because
     we are likely to be called just before commands to remake a
     member are run, and they will change the archive itself.  */
  (void) f_mtime (enter_file (arname));

  libdesc.dsc$a_pointer = arname;
  libdesc.dsc$w_length = strlen(arname);
  moddesc.dsc$a_pointer = memname;
  moddesc.dsc$w_length = strlen(memname);

  if (!((status = lbr$ini_control(&lidx,&func,&type,0)) & 1)) {
    printf("Error in lbr$ini_control, %d\n",status);
    return(-1);
  }

  if (!((status = lbr$open(&lidx,&libdesc,0,0,0,0,0)) & 1)) {
    printf("Error opening library %s to lookup member %s, %d\n",arname, memname ,status);
    return(-1);
  }

  if (!((status = lbr$lookup_key(&lidx,&moddesc,rfa)) & 1)) {
      if (status != LBR$_KEYNOTFND)
	printf("Error looking up module %s in library %s, %d\n",memname, arname ,status);
      lbr$close(&lidx);
      return(-1);
  }

  if (!((status = lbr$set_module(&lidx,rfa,&bufdesc,&bufdesc,0)) & 1)) {
    printf("Error getting module info, %d\n",status);
    lbr$close(&lidx);
    return(-1);
  }

  lbr$close(&lidx);

  val = SHELL$FIX_TIME(&buf.mhd.mhd$l_datim);

  free (arname);
  free (memname);
  return (val <= 0 ? (time_t) -1 : (time_t) val);
}

#else

static long int ar_member_date_1 PARAMS ((int desc, char *mem, int truncated, long int hdrpos,
	long int datapos, long int size, long int date, int uid, int gid, int mode, char *name));

/* Return the modtime of NAME.  */

time_t
ar_member_date (name)
     char *name;
{
  char *arname;
  int arname_used = 0;
  char *memname;
  long int val;

  ar_parse_name (name, &arname, &memname);

  /* Make sure we know the modtime of the archive itself because we are
     likely to be called just before commands to remake a member are run,
     and they will change the archive itself.

     But we must be careful not to enter_file the archive itself if it does
     not exist, because pattern_search assumes that files found in the data
     base exist or can be made.  */
  {
    struct file *arfile;
    arfile = lookup_file (arname);
    if (arfile == 0 && file_exists_p (arname))
      {
	arfile = enter_file (arname);
	arname_used = 1;
      }

    if (arfile != 0)
      (void) f_mtime (arfile, 0);
  }

  val = ar_scan (arname, ar_member_date_1, (long int) memname);

  if (!arname_used)
    free (arname);
  free (memname);

  return (val <= 0 ? (time_t) -1 : (time_t) val);
}

/* This function is called by `ar_scan' to find which member to look at.  */

/* ARGSUSED */
static long int
ar_member_date_1 (desc, mem, truncated,
		  hdrpos, datapos, size, date, uid, gid, mode, name)
     int desc;
     char *mem;
     int truncated;
     long int hdrpos, datapos, size, date;
     int uid, gid, mode;
     char *name;
{
  return ar_name_equal (name, mem, truncated) ? date : 0;
}
#endif /* !VMS */

/* Set the archive-member NAME's modtime to now.  */

#ifdef VMS
int
ar_touch (name)  
     char *name;
{
  error ("touch archive member is not available on VMS");
  return -1;
}
#else
int
ar_touch (name)
     char *name;
{
  char *arname, *memname;
  int arname_used = 0;
  register int val;

  ar_parse_name (name, &arname, &memname);

  /* Make sure we know the modtime of the archive itself before we
     touch the member, since this will change the archive itself.  */
  {
    struct file *arfile;
    arfile = lookup_file (arname);
    if (arfile == 0)
      {
	arfile = enter_file (arname);
	arname_used = 1;
      }

    (void) f_mtime (arfile, 0);
  }

  val = 1;
  switch (ar_member_touch (arname, memname))
    {
    case -1:
      error ("touch: Archive `%s' does not exist", arname);
      break;
    case -2:
      error ("touch: `%s' is not a valid archive", arname);
      break;
    case -3:
      perror_with_name ("touch: ", arname);
      break;
    case 1:
      error ("touch: Member `%s' does not exist in `%s'", memname, arname);
      break;
    case 0:
      val = 0;
      break;
    default:
      error ("touch: Bad return code from ar_member_touch on `%s'", name);
    }

  if (!arname_used)
    free (arname);
  free (memname);

  return val;
}
#endif /* !VMS */

/* State of an `ar_glob' run, passed to `ar_glob_match'.  */

struct ar_glob_state
  {
    char *arname;
    char *pattern;
    unsigned int size;
    struct nameseq *chain;
    unsigned int n;
  };

/* This function is called by `ar_scan' to match one archive
   element against the pattern in STATE.  */

static long int
ar_glob_match (desc, mem, truncated,
	       hdrpos, datapos, size, date, uid, gid, mode,
	       state)
     int desc;
     char *mem;
     int truncated;
     long int hdrpos, datapos, size, date;
     int uid, gid, mode;
     struct ar_glob_state *state;
{
  if (fnmatch (state->pattern, mem, FNM_PATHNAME|FNM_PERIOD) == 0)
    {
      /* We have a match.  Add it to the chain.  */
      struct nameseq *new = (struct nameseq *) xmalloc (state->size);
      new->name = concat (state->arname, mem, ")");
      new->next = state->chain;
      state->chain = new;
      ++state->n;
    }

  return 0L;
}

/* Alphabetic sorting function for `qsort'.  */

static int
ar_glob_alphacompare (a, b)
     char **a, **b;
{
  return strcmp (*a, *b);
}

/* Return nonzero if PATTERN contains any metacharacters.
   Metacharacters can be quoted with backslashes if QUOTE is nonzero.  */
static int
glob_pattern_p (pattern, quote)
     const char *pattern;
     const int quote;
{
  register const char *p;
  int open = 0;

  for (p = pattern; *p != '\0'; ++p)
    switch (*p)
      {
      case '?':
      case '*':
	return 1;

      case '\\':
	if (quote)
	  ++p;
	break;

      case '[':
	open = 1;
	break;

      case ']':
	if (open)
	  return 1;
	break;
      }

  return 0;
}

/* Glob for MEMBER_PATTERN in archive ARNAME.
   Return a malloc'd chain of matching elements (or nil if none).  */

struct nameseq *
ar_glob (arname, member_pattern, size)
     char *arname, *member_pattern;
     unsigned int size;
{
  struct ar_glob_state state;
  char **names;
  struct nameseq *n;
  unsigned int i;

  if (! glob_pattern_p (member_pattern, 1))
    return 0;

  /* Scan the archive for matches.
     ar_glob_match will accumulate them in STATE.chain.  */
  i = strlen (arname);
  state.arname = (char *) alloca (i + 2);
  bcopy (arname, state.arname, i);
  state.arname[i] = '(';
  state.arname[i + 1] = '\0';
  state.pattern = member_pattern;
  state.size = size;
  state.chain = 0;
  state.n = 0;
  (void) ar_scan (arname, ar_glob_match, (long int) &state);

  if (state.chain == 0)
    return 0;

  /* Now put the names into a vector for sorting.  */
  names = (char **) alloca (state.n * sizeof (char *));
  i = 0;
  for (n = state.chain; n != 0; n = n->next)
    names[i++] = n->name;

  /* Sort them alphabetically.  */
  qsort ((char *) names, i, sizeof (*names), ar_glob_alphacompare);

  /* Put them back into the chain in the sorted order.  */
  i = 0;
  for (n = state.chain; n != 0; n = n->next)
    n->name = names[i++];

  return state.chain;
}

#endif	/* Not NO_ARCHIVES.  */
