/* Copyright (C) 1988, 1989, 1991 Free Software Foundation, Inc.
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
#include "file.h"
#include "variable.h"


/* Structure used to represent a selective VPATH searchpath.  */

struct vpath
  {
    struct vpath *next;	/* Pointer to next struct in the linked list.  */
    char *pattern;	/* The pattern to match.  */
    char *percent;	/* Pointer into `pattern' where the `%' is.  */
    unsigned int patlen;/* Length of the pattern.  */
    char **searchpath;	/* Null-terminated list of directories.  */
    unsigned int maxlen;/* Maximum length of any entry in the list.  */
  };

/* Linked-list of all selective VPATHs.  */

static struct vpath *vpaths;

/* Structure for the general VPATH given in the variable.  */

static struct vpath *general_vpath;

static int selective_vpath_search ();

/* Reverse the chain of selective VPATH lists so they
   will be searched in the order given in the makefiles
   and construct the list from the VPATH variable.  */

void
build_vpath_lists ()
{
  register struct vpath *new = 0;
  register struct vpath *old, *nexto;
  register char *p;

  /* Reverse the chain.  */
  for (old = vpaths; old != 0; old = nexto)
    {
      nexto = old->next;
      old->next = new;
      new = old;
    }

  vpaths = new;

  /* If there is a VPATH variable with a nonnull value, construct the
     general VPATH list from it.  We use variable_expand rather than just
     calling lookup_variable so that it will be recursively expanded.  */
  p = variable_expand ("$(VPATH)");
  if (*p != '\0')
    {
      construct_vpath_list ("%", p);
      /* VPATHS will be nil if there have been no previous `vpath'
	 directives and none of the given directories exists.  */
      if (vpaths == 0)
	general_vpath = 0;
      else
	{
	  general_vpath = vpaths;
	  /* It was just put into the linked list,
	     but we don't want it there, so we must remove it.  */
	  vpaths = general_vpath->next;
	}
    }
}

/* Construct the VPATH listing for the pattern and searchpath given.

   This function is called to generate selective VPATH lists and also for
   the general VPATH list (which is in fact just a selective VPATH that
   is applied to everything).  The returned pointer is either put in the
   linked list of all selective VPATH lists or in the GENERAL_VPATH
   variable.

   If SEARCHPATH is nil, remove all previous listings with the same
   pattern.  If PATTERN is nil, remove all VPATH listings.
   Existing and readable directories that are not "." given in the
   searchpath separated by colons are loaded into the directory hash
   table if they are not there already and put in the VPATH searchpath
   for the given pattern with trailing slashes stripped off if present
   (and if the directory is not the root, "/").
   The length of the longest entry in the list is put in the structure as well.
   The new entry will be at the head of the VPATHS chain.  */

void
construct_vpath_list (pattern, dirpath)
     char *pattern, *dirpath;
{
  register unsigned int elem;
  register char *p;
  register char **vpath;
  register unsigned int maxvpath;
  unsigned int maxelem;
  char *percent;

  if (pattern != 0)
    {
      pattern = savestring (pattern, strlen (pattern));
      percent = find_percent (pattern);
    }

  if (dirpath == 0)
    {
      /* Remove matching listings.  */
      register struct vpath *path, *lastpath;

      lastpath = vpaths;
      for (path = vpaths; path != 0; lastpath = path, path = path->next)
	if (pattern == 0
	    || (((percent == 0 && path->percent == 0)
		 || (percent - pattern == path->percent - path->pattern))
		&& streq (pattern, path->pattern)))
	  {
	    /* Remove it from the linked list.  */
	    if (lastpath == vpaths)
	      vpaths = path->next;
	    else
	      lastpath->next = path->next;

	    /* Free its unused storage.  */
	    free (path->pattern);
	    free ((char *) path->searchpath);
	    free ((char *) path);
	  }
      if (pattern != 0)
	free (pattern);
      return;
    }

  /* Skip over any initial colons.  */
  p = dirpath;
  while (*p == ':')
    ++p;

  /* Figure out the maximum number of VPATH entries and
     put it in MAXELEM.  We start with 2, one before the
     first colon and one nil, the list terminator and
     increment our estimated number for each colon we find.  */
  maxelem = 2;
  while (*p != '\0')
    if (*p++ == ':')
      ++maxelem;

  vpath = (char **) xmalloc (maxelem * sizeof (char *));
  maxvpath = 0;

  elem = 0;
  p = dirpath;
  while (*p != '\0')
    {
      char *v;
      unsigned int len;

      /* Find the next entry.  */
      while (*p != '\0' && *p == ':')
	++p;
      if (*p == '\0')
	break;

      /* Find the end of this entry.  */
      v = p;
      while (*p != '\0' && *p != ':')
	++p;

      len = p - v;
      /* Make sure there's no trailing slash,
	 but still allow "/" as a directory.  */
      if (len > 1 && p[-1] == '/')
	--len;

      if (len == 1 && *v == '.')
	continue;

      v = savestring (v, len);
      if (dir_file_exists_p (v, ""))
	{
	  vpath[elem++] = dir_name (v);
	  free (v);
	  if (len > maxvpath)
	    maxvpath = len;
	}
      else
	free (v);
    }

  if (elem > 0)
    {
      struct vpath *path;
      /* ELEM is now incremented one element past the last
	 entry, to where the nil-pointer terminator goes.
	 Usually this is maxelem - 1.  If not, shrink down.  */
      if (elem < (maxelem - 1))
	vpath = (char **) xrealloc ((char *) vpath,
				    (elem + 1) * sizeof (char *));

      /* Put the nil-pointer terminator on the end of the VPATH list.  */
      vpath[elem] = 0;

      /* Construct the vpath structure and put it into the linked list.  */
      path = (struct vpath *) xmalloc (sizeof (struct vpath));
      path->searchpath = vpath;
      path->maxlen = maxvpath;
      path->next = vpaths;
      vpaths = path;

      /* Set up the members.  */
      path->pattern = pattern;
      path->percent = percent;
      path->patlen = strlen (pattern);
    }
  else
    /* There were no entries, so free whatever space we allocated.  */
    free ((char *) vpath);
}

/* Search the VPATH list whose pattern matches *FILE for a directory
   where the name pointed to by FILE exists.  If it is found, the pointer
   in FILE is set to the newly malloc'd name of the existing file and
   we return 1.  Otherwise we return 0.  */

int
vpath_search (file)
     char **file;
{
  register struct vpath *v;

  /* If there are no VPATH entries or FILENAME starts at the root,
     there is nothing we can do.  */

  if (**file == '/' || (vpaths == 0 && general_vpath == 0))
    return 0;

  for (v = vpaths; v != 0; v = v->next)
    if (pattern_matches (v->pattern, v->percent, *file))
      if (selective_vpath_search (v, file))
	return 1;

  if (general_vpath != 0
      && selective_vpath_search (general_vpath, file))
    return 1;

  return 0;
}


/* Search the given VPATH list for a directory where the name pointed
   to by FILE exists.  If it is found, the pointer in FILE
   is set to the newly malloc'd name of the existing file and we return 1.
   Otherwise we return 0.  */

static int
selective_vpath_search (path, file)
     struct vpath *path;
     char **file;
{
  int not_target;
  char *name, *n;
  char *filename;
  register char **vpath = path->searchpath;
  unsigned int maxvpath = path->maxlen;
  register unsigned int i;
  unsigned int flen, vlen, name_dplen;
  int exists = 0;

  /* Find out if *FILE is a target.
     If and only if it is NOT a target, we will accept prospective
     files that don't exist but are mentioned in a makefile.  */
  {
    struct file *f = lookup_file (*file);
    not_target = f == 0 || !f->is_target;
  }

  flen = strlen (*file);

  /* Split *FILE into a directory prefix and a name-within-directory.
     NAME_DPLEN gets the length of the prefix; FILENAME gets the
     pointer to the name-within-directory and FLEN is its length.  */

  n = rindex (*file, '/');
  name_dplen = n != 0 ? n - *file : 0;
  filename = name_dplen > 0 ? n + 1 : *file;
  if (name_dplen > 0)
    flen -= name_dplen + 1;

  /* Allocate enough space for the biggest VPATH entry,
     a slash, the directory prefix that came with *FILE,
     another slash (although this one may not always be
     necessary), the filename, and a null terminator.  */
  name = (char *) alloca (maxvpath + 1 + name_dplen + 1 + flen + 1);

  /* Try each VPATH entry.  */
  for (i = 0; vpath[i] != 0; ++i)
    {
      n = name;

      /* Put the next VPATH entry into NAME at N and increment N past it.  */
      vlen = strlen (vpath[i]);
      bcopy (vpath[i], n, vlen);
      n += vlen;

      /* Add the directory prefix already in *FILE.  */
      if (name_dplen > 0)
	{
	  *n++ = '/';
	  bcopy (*file, n, name_dplen);
	  n += name_dplen;
	}

      /* Now add the name-within-directory at the end of NAME.  */
      if (n != name && n[-1] != '/')
	*n = '/';
      bcopy (filename, n + 1, flen + 1);

      if (not_target)
	/* Since *FILE is not a target, if the file is
	   mentioned in a makefile, we consider it existent.  */
	exists = lookup_file (name) != 0;

      if (!exists)
	{
	  /* That file wasn't mentioned in the makefile.
	     See if it actually exists.  */

	  /* Clobber a null into the name at the last slash.
	     Now NAME is the name of the directory to look in.  */
	  *n = '\0';

	  /* Make sure the directory exists and we know its contents.  */
	  if (name_dplen > 0 && !dir_file_exists_p (name, ""))
	    /* It doesn't exist.  */
	    continue;

	  /* We know the directory is in the hash table now because either
	     construct_vpath_list or the code just above put it there.
	     Does the file we seek exist in it?  */
	  exists = dir_file_exists_p (name, filename);
	}

      if (exists)
	{
	  /* We have found a file.
	     Store the name we found into *FILE for the caller.  */

	  /* Put the slash back in NAME.  */
	  *n = '/';

	  *file = savestring (name, (n + 1 - name) + flen);

	  return 1;
	}
    }

  return 0;
}

/* Print the data base of VPATH search paths.  */

void
print_vpath_data_base ()
{
  register unsigned int nvpaths;
  register struct vpath *v;

  puts ("\n# VPATH Search Paths\n");

  nvpaths = 0;
  for (v = vpaths; v != 0; v = v->next)
    {
      register unsigned int i;

      ++nvpaths;

      printf ("vpath %s ", v->pattern);

      for (i = 0; v->searchpath[i] != 0; ++i)
	printf ("%s%c", v->searchpath[i],
		v->searchpath[i + 1] == 0 ? '\n' : ':');
    }

  if (vpaths == 0)
    puts ("# No `vpath' search paths.");
  else
    printf ("\n# %u `vpath' search paths.\n", nvpaths);

  if (general_vpath == 0)
    puts ("\n# No general (`VPATH' variable) search path.");
  else
    {
      register char **path = general_vpath->searchpath;
      register unsigned int i;

      fputs ("\n# General (`VPATH' variable) search path:\n# ", stdout);

      for (i = 0; path[i] != 0; ++i)
	printf ("%s%c", path[i], path[i + 1] == 0 ? '\n' : ':');
    }
}
