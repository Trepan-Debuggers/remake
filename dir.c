/* Directory hashing for GNU Make.
Copyright (C) 1988, 1989, 1991, 1992, 1993 Free Software Foundation, Inc.
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

#if	defined (POSIX) || defined (DIRENT) || defined (__GNU_LIBRARY__)
#include <dirent.h>
#ifndef	__GNU_LIBRARY__
#define D_NAMLEN(d) strlen((d)->d_name)
#else	/* GNU C library.  */
#define D_NAMLEN(d) ((d)->d_namlen)
#endif	/* Not GNU C library.  */
#else	/* Not POSIX or DIRENT.  */
#define direct dirent
#define D_NAMLEN(d) ((d)->d_namlen)
#ifdef	SYSNDIR
#include <sys/ndir.h>
#endif	/* SYSNDIR */
#ifdef	SYSDIR
#include <sys/dir.h>
#endif	/* SYSDIR */
#ifdef NDIR
#include <ndir.h>
#endif	/* NDIR */
#endif	/* POSIX or DIRENT or __GNU_LIBRARY__.  */

#if defined (POSIX) && !defined (__GNU_LIBRARY__)
/* Posix does not require that the d_ino field be present, and some
   systems do not provide it. */
#define REAL_DIR_ENTRY(dp) 1
#else
#define REAL_DIR_ENTRY(dp) (dp->d_ino != 0)
#endif /* POSIX */

/* Hash table of directories.  */

#ifndef	DIRECTORY_BUCKETS
#define DIRECTORY_BUCKETS 199
#endif

struct directory_contents
  {
    struct directory_contents *next;

    int dev, ino;		/* Device and inode numbers of this dir.  */

    struct dirfile **files;	/* Files in this directory.  */
    DIR *dirstream;		/* Stream reading this directory.  */
  };

/* Table of directory contents hashed by device and inode number.  */
static struct directory_contents *directories_contents[DIRECTORY_BUCKETS];

struct directory
  {
    struct directory *next;

    char *name;			/* Name of the directory.  */

    /* The directory's contents.  This data may be shared by several
       entries in the hash table, which refer to the same directory
       (identified uniquely by `dev' and `ino') under different names.  */
    struct directory_contents *contents;
  };

/* Table of directories hashed by name.  */
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
#define DIRFILE_BUCKETS 107
#endif

static int dir_contents_file_exists_p ();

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
      struct stat st;

      /* The directory was not found.  Create a new entry for it.  */

      dir = (struct directory *) xmalloc (sizeof (struct directory));
      dir->next = directories[hash];
      directories[hash] = dir;
      dir->name = savestring (name, p - name);

      /* The directory is not in the name hash table.
	 Find its device and inode numbers, and look it up by them.  */

      if (stat (name, &st) < 0)
	/* Couldn't stat the directory.  Mark this by
	   setting the `contents' member to a nil pointer.  */
	dir->contents = 0;
      else
	{
	  /* Search the contents hash table; device and inode are the key.  */

	  struct directory_contents *dc;

	  hash = ((unsigned int) st.st_dev << 16) | (unsigned int) st.st_ino;
	  hash %= DIRECTORY_BUCKETS;

	  for (dc = directories_contents[hash]; dc != 0; dc = dc->next)
	    if (dc->dev == st.st_dev && dc->ino == st.st_ino)
	      break;

	  if (dc == 0)
	    {
	      /* Nope; this really is a directory we haven't seen before.  */

	      dc = (struct directory_contents *)
		xmalloc (sizeof (struct directory_contents));

	      /* Enter it in the contents hash table.  */
	      dc->dev = st.st_dev;
	      dc->ino = st.st_ino;
	      dc->next = directories_contents[hash];
	      directories_contents[hash] = dc;

	      dc->dirstream = opendir (name);
	      if (dc->dirstream == 0)
		/* Couldn't open the directory.  Mark this by
		   setting the `files' member to a nil pointer.  */
		dc->files = 0;
	      else
		{
		  /* Allocate an array of buckets for files and zero it.  */
		  dc->files = (struct dirfile **)
		    xmalloc (sizeof (struct dirfile *) * DIRFILE_BUCKETS);
		  bzero ((char *) dc->files,
			 sizeof (struct dirfile *) * DIRFILE_BUCKETS);

		  /* Keep track of how many directories are open.  */
		  ++open_directories;
		  if (open_directories == MAX_OPEN_DIRECTORIES)
		    /* We have too many directories open already.
		       Read the entire directory and then close it.  */
		    (void) dir_contents_file_exists_p (dc, (char *) 0);
		}
	    }

	  /* Point the name-hashed entry for DIR at its contents data.  */
	  dir->contents = dc;
	}
    }

  return dir;
}

/* Return 1 if the name FILENAME is entered in DIR's hash table.
   FILENAME must contain no slashes.  */

static int
dir_contents_file_exists_p (dir, filename)
     register struct directory_contents *dir;
     register char *filename;
{
  register unsigned int hash;
  register char *p;
  register struct dirfile *df;
  register struct dirent *d;

  if (dir == 0 || dir->files == 0)
    /* The directory could not be stat'd or opened.  */
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
      unsigned int len;
      register unsigned int i;

      if (!REAL_DIR_ENTRY (d))
	continue;

      len = D_NAMLEN (d);
      while (d->d_name[len - 1] == '\0')
	--len;

      for (i = 0; i < len; ++i)
	HASH (newhash, d->d_name[i]);
      newhash %= DIRFILE_BUCKETS;

      df = (struct dirfile *) xmalloc (sizeof (struct dirfile));
      df->next = dir->files[newhash];
      dir->files[newhash] = df;
      df->name = savestring (d->d_name, len);
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

/* Return 1 if the name FILENAME in directory DIRNAME
   is entered in the dir hash table.
   FILENAME must contain no slashes.  */

int
dir_file_exists_p (dirname, filename)
     register char *dirname;
     register char *filename;
{
  return dir_contents_file_exists_p (find_directory (dirname)->contents,
				     filename);
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

  if (dir->contents == 0)
    {
      /* The directory could not be stat'd.  We allocate a contents
	 structure for it, but leave it out of the contents hash table.  */
      dir->contents = (struct directory_contents *)
	xmalloc (sizeof (struct directory_contents));
      dir->contents->dev = dir->contents->ino = 0;
      dir->contents->files = 0;
      dir->contents->dirstream = 0;
    }

  if (dir->contents->files == 0)
    {
      /* The directory was not opened; we must allocate the hash buckets.  */
      dir->contents->files = (struct dirfile **)
	xmalloc (sizeof (struct dirfile) * DIRFILE_BUCKETS);
      bzero ((char *) dir->contents->files,
	     sizeof (struct dirfile) * DIRFILE_BUCKETS);
    }

  /* Make a new entry and put it in the table.  */

  new = (struct dirfile *) xmalloc (sizeof (struct dirfile));
  new->next = dir->contents->files[hash];
  dir->contents->files[hash] = new;
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
  register struct directory_contents *dir;
  register struct dirfile *next;

  dirend = rindex (filename, '/');
  if (dirend == 0)
    dir = find_directory (".")->contents;
  else
    {
      char *dirname = (char *) alloca (dirend - filename + 1);
      bcopy (p, dirname, dirend - p);
      dirname[dirend - p] = '\0';
      dir = find_directory (dirname)->contents;
      p = dirend + 1;
    }

  if (dir == 0 || dir->files == 0)
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
	if (dir->contents == 0)
	  printf ("# %s: could not be stat'd.\n", dir->name);
	else if (dir->contents->files == 0)
	  printf ("# %s (device %d, inode %d): could not be opened.\n",
		  dir->name, dir->contents->dev, dir->contents->ino);
	else
	  {
	    register unsigned int f = 0, im = 0;
	    register unsigned int j;
	    register struct dirfile *df;
	    for (j = 0; j < DIRFILE_BUCKETS; ++j)
	      for (df = dir->contents->files[j]; df != 0; df = df->next)
		if (df->impossible)
		  ++im;
		else
		  ++f;
	    printf ("# %s (device %d, inode %d): ",
		    dir->name, dir->contents->dev, dir->contents->ino);
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
	    if (dir->contents->dirstream == 0)
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

/* Hooks for globbing.  */

#include <glob.h>

/* Structure describing state of iterating through a directory hash table.  */

struct dirstream
  {
    struct directory_contents *contents; /* The directory being read.  */

    unsigned int bucket;	/* Current hash bucket.  */
    struct dirfile *elt;	/* Current elt in bucket.  */
  };

/* Forward declarations.  */
static __ptr_t open_dirstream __P ((const char *));
static const char *read_dirstream __P ((__ptr_t));

static __ptr_t
open_dirstream (directory)
     const char *directory;
{
  struct dirstream *new;
  struct directory *dir = find_directory (directory);

  if (dir->contents == 0 || dir->contents->files == 0)
    /* DIR->contents is nil if the directory could not be stat'd.
       DIR->contents->files is nil if it could not be opened.  */
    return 0;

  /* Read all the contents of the directory now.  There is no benefit
     in being lazy, since glob will want to see every file anyway.  */

  (void) dir_contents_file_exists_p (dir->contents, (char *) 0);

  new = (struct dirstream *) xmalloc (sizeof (struct dirstream));
  new->contents = dir->contents;
  new->bucket = 0;
  new->elt = new->contents->files[0];

  return (__ptr_t) new;
}

static const char *
read_dirstream (stream)
     __ptr_t stream;
{
  struct dirstream *const ds = (struct dirstream *) stream;
  register struct dirfile *df;

  while (ds->bucket < DIRFILE_BUCKETS)
    {
      while ((df = ds->elt) != 0)
	{
	  ds->elt = df->next;
	  if (!df->impossible)
	    return df->name;
	}
      if (++ds->bucket == DIRFILE_BUCKETS)
	break;
      ds->elt = ds->contents->files[ds->bucket];
    }

  return 0;
}

void
init_dir ()
{
  __glob_opendir_hook = open_dirstream;
  __glob_readdir_hook = read_dirstream;
  __glob_closedir_hook = (void (*) __P ((__ptr_t stream))) free;
}
