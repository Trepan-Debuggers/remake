/* Directory hashing for GNU Make.
Copyright (C) 1988, 1989, 1991 Free Software Foundation, Inc.
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

#if defined (USGr3) && !defined (DIRENT)
#define DIRENT
#endif /* USGr3 */
#if defined (Xenix) && !defined (SYSNDIR)
#define SYSNDIR
#endif /* Xenix */

#if defined (POSIX) || defined (DIRENT) || defined (__GNU_LIBRARY__)
#include <dirent.h>
#define direct dirent
#define D_NAMLEN(d) strlen((d)->d_name)
#else /* not POSIX or DIRENT */
#define D_NAMLEN(d) ((d)->d_namlen)
#if defined (USG) && !defined (sgi)
#if defined (SYSNDIR)
#include <sys/ndir.h>
#else /* SYSNDIR */
#include "ndir.h"
#endif /* not SYSNDIR */
#else /* not USG */
#include <sys/dir.h>
#endif /* USG */
#endif /* POSIX or DIRENT or __GNU_LIBRARY__ */

#if defined (POSIX) && !defined (__GNU_LIBRARY__)
/* Posix does not require that the d_ino field be present, and some
   systems do not provide it. */
#define REAL_DIR_ENTRY(dp) 1
#else
#define REAL_DIR_ENTRY(dp) (dp->d_ino != 0)
#endif /* POSIX */

/* Hash table of directories.  */

struct directory
  {
    struct directory *next;
    char *name;			/* Name of the directory.  */
    struct dirfile **files;	/* Files in this directory.  */
    DIR *dirstream;		/* Stream reading this directory.  */
  };

#ifndef	DIRECTORY_BUCKETS
#define DIRECTORY_BUCKETS 23
#endif

static struct directory *directories[DIRECTORY_BUCKETS];


/* Never have more than this many directories open at once.  */

#define MAX_OPEN_DIRECTORIES 10

static unsigned int open_directories = 0;


/* Hash table of files in each directory.  */

struct dirfile
  {
    struct dirfile *next;
    char *name;			/* Name of the file.  */
    char impossible;		/* This file is impossible.  */
  };

#ifndef	DIRFILE_BUCKETS
#define DIRFILE_BUCKETS 1007
#endif

/* Find the directory named NAME and return its `struct directory'.  */

static struct directory *
find_directory (name)
     register char *name;
{
  register unsigned int hash = 0;
  register char *p;
  register struct directory *dir;

  for (p = name; *p != '\0'; ++p)
    HASH (hash, *p);
  hash %= DIRECTORY_BUCKETS;

  for (dir = directories[hash]; dir != 0; dir = dir->next)
    if (streq (dir->name, name))
      break;

  if (dir == 0)
    {
      /* The directory was not found.  Create a new entry
	 for it and start its directory stream reading.  */
      dir = (struct directory *) xmalloc (sizeof (struct directory));
      dir->next = directories[hash];
      directories[hash] = dir;
      dir->name = savestring (name, p - name);
      dir->dirstream = opendir (name);
      if (dir->dirstream == 0)
	/* Couldn't open the directory.  Mark this by
	   setting the `files' member to a nil pointer.  */
	dir->files = 0;
      else
	{
	  /* Allocate an array of hash buckets for files and zero it.  */
	  dir->files = (struct dirfile **)
	    xmalloc (sizeof (struct dirfile) * DIRFILE_BUCKETS);
	  bzero ((char *) dir->files,
		 sizeof (struct dirfile) * DIRFILE_BUCKETS);

	  /* Keep track of how many directories are open.  */
	  ++open_directories;
	  if (open_directories == MAX_OPEN_DIRECTORIES)
	    /* Read the entire directory and then close it.  */
	    (void) dir_file_exists_p (dir->name, (char *) 0);
	}
    }

  return dir;
}

/* Return 1 if the name FILENAME in directory DIRNAME
   is entered in the dir hash table.
   FILENAME must contain no slashes.  */

int
dir_file_exists_p (dirname, filename)
     register char *dirname;
     register char *filename;
{
  register unsigned int hash;
  register char *p;
  register struct directory *dir;
  register struct dirfile *df;
  register struct direct *d;
  dir = find_directory (dirname);

  if (dir->files == 0)
    /* The directory could not be opened.  */
    return 0;

  hash = 0;
  if (filename != 0)
    {
      if (*filename == '\0')
	/* Checking if the directory exists.  */
	return 1;

      for (p = filename; *p != '\0'; ++p)
	HASH (hash, *p);
      hash %= DIRFILE_BUCKETS;

      /* Search the list of hashed files.  */

      for (df = dir->files[hash]; df != 0; df = df->next)
	if (streq (df->name, filename))
	  return !df->impossible;
    }

  /* The file was not found in the hashed list.
     Try to read the directory further.  */

  if (dir->dirstream == 0)
    /* The directory has been all read in.  */
    return 0;

  while ((d = readdir (dir->dirstream)) != 0)
    {
      /* Enter the file in the hash table.  */
      register unsigned int newhash = 0;
      register unsigned int i;

      if (!REAL_DIR_ENTRY (d))
	continue;

      for (i = 0; i < D_NAMLEN(d); ++i)
	HASH (newhash, d->d_name[i]);
      newhash %= DIRFILE_BUCKETS;

      df = (struct dirfile *) xmalloc (sizeof (struct dirfile));
      df->next = dir->files[newhash];
      dir->files[newhash] = df;
      df->name = savestring (d->d_name, D_NAMLEN(d));
      df->impossible = 0;

      /* Check if the name matches the one we're searching for.  */
      if (filename != 0
	  && newhash == hash && streq (d->d_name, filename))
	return 1;
    }

  /* If the directory has been completely read in,
     close the stream and reset the pointer to nil.  */
  if (d == 0)
    {
      --open_directories;
      closedir (dir->dirstream);
      dir->dirstream = 0;
    }

  return 0;
}

/* Return 1 if the file named NAME exists.  */

int
file_exists_p (name)
     register char *name;
{
  char *dirend;
  char *dirname;

#ifndef	NO_ARCHIVES
  if (ar_name (name))
    return ar_member_date (name) != (time_t) -1;
#endif

  dirend = rindex (name, '/');
  if (dirend == 0)
    return dir_file_exists_p (".", name);

  dirname = (char *) alloca (dirend - name + 1);
  bcopy (name, dirname, dirend - name);
  dirname[dirend - name] = '\0';
  return dir_file_exists_p (dirname, dirend + 1);
}

/* Mark FILENAME as `impossible' for `file_impossible_p'.
   This means an attempt has been made to search for FILENAME
   as an intermediate file, and it has failed.  */

void
file_impossible (filename)
     register char *filename;
{
  char *dirend;
  register char *p = filename;
  register unsigned int hash;
  register struct directory *dir;
  register struct dirfile *new;

  dirend = rindex (p, '/');
  if (dirend == 0)
    dir = find_directory (".");
  else
    {
      char *dirname = (char *) alloca (dirend - p + 1);
      bcopy (p, dirname, dirend - p);
      dirname[dirend - p] = '\0';
      dir = find_directory (dirname);
      filename = p = dirend + 1;
    }

  for (hash = 0; *p != '\0'; ++p)
    HASH (hash, *p);
  hash %= DIRFILE_BUCKETS;

  if (dir->files == 0)
    {
      /* The directory was not opened; we must allocate the hash buckets.  */
      dir->files = (struct dirfile **)
	xmalloc (sizeof (struct dirfile) * DIRFILE_BUCKETS);
      bzero ((char *) dir->files, sizeof (struct dirfile) * DIRFILE_BUCKETS);
    }

  /* Make a new entry and put it in the table.  */

  new = (struct dirfile *) xmalloc (sizeof (struct dirfile));
  new->next = dir->files[hash];
  dir->files[hash] = new;
  new->name = savestring (filename, strlen (filename));
  new->impossible = 1;
}

/* Return nonzero if FILENAME has been marked impossible.  */

int
file_impossible_p (filename)
     char *filename;
{
  char *dirend;
  register char *p = filename;
  register unsigned int hash;
  register struct directory *dir;
  register struct dirfile *next;

  dirend = rindex (filename, '/');
  if (dirend == 0)
    dir = find_directory (".");
  else
    {
      char *dirname = (char *) alloca (dirend - filename + 1);
      bcopy (p, dirname, dirend - p);
      dirname[dirend - p] = '\0';
      dir = find_directory (dirname);
      p = dirend + 1;
    }

  if (dir->files == 0)
    /* There are no files entered for this directory.  */
    return 0;

  for (hash = 0; *p != '\0'; ++p)
    HASH (hash, *p);
  hash %= DIRFILE_BUCKETS;

  for (next = dir->files[hash]; next != 0; next = next->next)
    if (streq (filename, next->name))
      return next->impossible;

  return 0;
}

/* Return the already allocated name in the
   directory hash table that matches DIR.  */

char *
dir_name (dir)
     char *dir;
{
  return find_directory (dir)->name;
}

/* Print the data base of directories.  */

void
print_dir_data_base ()
{
  register unsigned int i, dirs, files, impossible;
  register struct directory *dir;

  puts ("\n# Directories\n");

  dirs = files = impossible = 0;
  for (i = 0; i < DIRECTORY_BUCKETS; ++i)
    for (dir = directories[i]; dir != 0; dir = dir->next)
      {
	++dirs;
	if (dir->files == 0)
	  printf ("# %s: could not be opened.\n", dir->name);
	else
	  {
	    register unsigned int f = 0, im = 0;
	    register unsigned int j;
	    register struct dirfile *df;
	    for (j = 0; j < DIRFILE_BUCKETS; ++j)
	      for (df = dir->files[j]; df != 0; df = df->next)
		if (df->impossible)
		  ++im;
		else
		  ++f;
	    printf ("# %s: ", dir->name);
	    if (f == 0)
	      fputs ("No", stdout);
	    else
	      printf ("%u", f);
	    fputs (" files, ", stdout);
	    if (im == 0)
	      fputs ("no", stdout);
	    else
	      printf ("%u", im);
	    fputs (" impossibilities", stdout);
	    if (dir->dirstream == 0)
	      puts (".");
	    else
	      puts (" so far.");
	    files += f;
	    impossible += im;
	  }
      }

  fputs ("\n# ", stdout);
  if (files == 0)
    fputs ("No", stdout);
  else
    printf ("%u", files);
  fputs (" files, ", stdout);
  if (impossible == 0)
    fputs ("no", stdout);
  else
    printf ("%u", impossible);
  printf (" impossibilities in %u directories.\n", dirs);
}
