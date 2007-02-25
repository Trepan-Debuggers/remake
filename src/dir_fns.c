/* $Id: dir_fns.c,v 1.9 2006/02/09 02:53:25 rockyb Exp $
Directory hashing for GNU Make.
Copyright (C) 1988, 1989, 1991, 1992, 1993, 1994, 1995, 1996, 1997,
2002, 2003, 2004, 2005, 2006 Free Software Foundation, Inc.
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

#include "misc.h"
#include "ar_fns.h"
#include "hash.h"

#ifdef	HAVE_STDLIB_H
#include <stdlib.h>
#endif

#ifdef HAVE_ALLOCA_H
#include <alloca.h>
#endif

#ifdef	HAVE_DIRENT_H
# include <dirent.h>
# define NAMLEN(dirent) strlen((dirent)->d_name)
#else
# define dirent direct
# define NAMLEN(dirent) (dirent)->d_namlen
# ifdef HAVE_SYS_NDIR_H
#  include <sys/ndir.h>
# endif
# ifdef HAVE_SYS_DIR_H
#  include <sys/dir.h>
# endif
# ifdef HAVE_NDIR_H
#  include <ndir.h>
# endif
#endif

#include "dir_fns.h"

/* In GNU systems, <dirent.h> defines this macro for us.  */
#ifdef _D_NAMLEN
# undef NAMLEN
# define NAMLEN(d) _D_NAMLEN(d)
#endif

#if (defined (POSIX) || defined (WINDOWS32)) && !defined (__GNU_LIBRARY__)
/* Posix does not require that the d_ino field be present, and some
   systems do not provide it. */
# define REAL_DIR_ENTRY(dp) 1
# define FAKE_DIR_ENTRY(dp)
#else
# define REAL_DIR_ENTRY(dp) (dp->d_ino != 0)
# define FAKE_DIR_ENTRY(dp) (dp->d_ino = 1)
#endif /* POSIX */

#ifdef __MSDOS__
#include <ctype.h>
#include <fcntl.h>

/* If it's MSDOS that doesn't have _USE_LFN, disable LFN support.  */
#ifndef _USE_LFN
#define _USE_LFN 0
#endif

static char *
dosify (char *filename)
{
  static char dos_filename[14];
  char *df;
  int i;

  if (filename == 0 || _USE_LFN)
    return filename;

  /* FIXME: what about filenames which violate
     8+3 constraints, like "config.h.in", or ".emacs"?  */
  if (strpbrk (filename, "\"*+,;<=>?[\\]|") != 0)
    return filename;

  df = dos_filename;

  /* First, transform the name part.  */
  for (i = 0; *filename != '\0' && i < 8 && *filename != '.'; ++i)
    *df++ = tolower ((unsigned char)*filename++);

  /* Now skip to the next dot.  */
  while (*filename != '\0' && *filename != '.')
    ++filename;
  if (*filename != '\0')
    {
      *df++ = *filename++;
      for (i = 0; *filename != '\0' && i < 3 && *filename != '.'; ++i)
	*df++ = tolower ((unsigned char)*filename++);
    }

  /* Look for more dots.  */
  while (*filename != '\0' && *filename != '.')
    ++filename;
  if (*filename == '.')
    return filename;
  *df = 0;
  return dos_filename;
}
#endif /* __MSDOS__ */

#ifdef WINDOWS32
#include "pathstuff.h"
#endif

#ifdef HAVE_CASE_INSENSITIVE_FS
static char *
downcase (char *filename)
{
  static char new_filename[PATH_MAX];
  char *df;
  int i;

  if (filename == 0)
    return 0;

  df = new_filename;

  /* First, transform the name part.  */
  for (i = 0; *filename != '\0'; ++i)
  {
    *df++ = tolower ((unsigned char)*filename);
    ++filename;
  }

  *df = 0;

  return new_filename;
}
#endif /* HAVE_CASE_INSENSITIVE_FS */


/* Hash table of directories.  */

#ifndef	DIRECTORY_BUCKETS
#define DIRECTORY_BUCKETS 199
#endif

typedef struct directory_contents
  {
    dev_t dev;			/* Device and inode numbers of this dir.  */
#ifdef WINDOWS32
    /*
     * Inode means nothing on WINDOWS32. Even file key information is
     * unreliable because it is random per file open and undefined
     * for remote filesystems. The most unique attribute I can
     * come up with is the fully qualified name of the directory. Beware
     * though, this is also unreliable. I'm open to suggestion on a better
     * way to emulate inode.
     */
    char *path_key;
    int   ctime;
    int   mtime;        /* controls check for stale directory cache */
    int   fs_flags;     /* FS_FAT, FS_NTFS, ... */
#define FS_FAT      0x1
#define FS_NTFS     0x2
#define FS_UNKNOWN  0x4
#else
    ino_t ino;
#endif /* WINDOWS32 */
    hash_table_t dirfiles;	/* Files in this directory.  */
    DIR *dirstream;		/* Stream reading this directory.  */
  } directory_contents_t;

static unsigned long
directory_contents_hash_1 (const void *key_0)
{
  directory_contents_t const *key = (directory_contents_t const *) key_0;
  unsigned long hash;

#ifdef WINDOWS32
  hash = 0;
  ISTRING_HASH_1 (key->path_key, hash);
  hash ^= ((unsigned int) key->dev << 4) ^ (unsigned int) key->ctime;
#else
  hash = ((unsigned int) key->dev << 4) ^ (unsigned int) key->ino;
#endif /* WINDOWS32 */
  return hash;
}

/* Sometimes it's OK to use subtraction to get this value:
     result = X - Y;
   But, if we're not sure of the type of X and Y they may be too large for an
   int (on a 64-bit system for example).  So, use ?: instead.
   See Savannah bug #15534.

   NOTE!  This macro has side-effects!
*/

#define MAKECMP(_x,_y)  ((_x)<(_y)?-1:((_x)==(_y)?0:1))

static unsigned long
directory_contents_hash_2 (const void *key_0)
{
  directory_contents_t const *key = (directory_contents_t const *) key_0;
  unsigned long hash;

#ifdef WINDOWS32
  hash = 0;
  ISTRING_HASH_2 (key->path_key, hash);
  hash ^= ((unsigned int) key->dev << 4) ^ (unsigned int) ~key->ctime;
#else
  hash = ((unsigned int) key->dev << 4) ^ (unsigned int) ~key->ino;
#endif /* WINDOWS32 */

  return hash;
}

static int
directory_contents_hash_cmp (const void *xv, const void *yv)
{
  directory_contents_t const *x = (directory_contents_t const *) xv;
  directory_contents_t const *y = (directory_contents_t const *) yv;
  int result;

#ifdef WINDOWS32
  ISTRING_COMPARE (x->path_key, y->path_key, result);
  if (result)
    return result;
  result = MAKECMP(x->ctime, y->ctime);
  if (result)
    return result;
#else
  result = MAKECMP(x->ino, y->ino);
  if (result)
    return result;
#endif /* WINDOWS32 */

  return MAKECMP(x->dev, y->dev);
}

/* Table of directory contents hashed by device and inode number.  */
static hash_table_t directory_contents;

typedef struct directory
  {
    char *name;			/* Name of the directory.  */

    /* The directory's contents.  This data may be shared by several
       entries in the hash table, which refer to the same directory
       (identified uniquely by `dev' and `ino') under different names.  */
    directory_contents_t *contents;
  } directory_t;


static unsigned long
directory_hash_1 (const void *key)
{
  return_ISTRING_HASH_1 (((directory_t const *) key)->name);
}

static unsigned long
directory_hash_2 (const void *key)
{
  return_ISTRING_HASH_2 (((directory_t const *) key)->name);
}

static int
directory_hash_cmp (const void *x, const void *y)
{
  return_ISTRING_COMPARE (((directory_t const *) x)->name,
			  ((directory_t const *) y)->name);
}

/* Table of directories hashed by name.  */
static hash_table_t directories;

/* Never have more than this many directories open at once.  */

#define MAX_OPEN_DIRECTORIES 10

static unsigned int open_directories = 0;


/* Hash table of files in each directory.  */

typedef struct dirfile
  {
    char *name;			/* Name of the file.  */
    short length;
    short impossible;		/* This file is impossible.  */
  } dirfile_t;

static unsigned long
dirfile_hash_1 (const void *key)
{
  return_ISTRING_HASH_1 (((dirfile_t const *) key)->name);
}

static unsigned long
dirfile_hash_2 (const void *key)
{
  return_ISTRING_HASH_2 (((dirfile_t const *) key)->name);
}

static int
dirfile_hash_cmp (const void *xv, const void *yv)
{
  dirfile_t const *x = ((dirfile_t const *) xv);
  dirfile_t const *y = ((dirfile_t const *) yv);
  int result = x->length - y->length;
  if (result)
    return result;
  return_ISTRING_COMPARE (x->name, y->name);
}

#ifndef	DIRFILE_BUCKETS
#define DIRFILE_BUCKETS 107
#endif

static int dir_contents_file_exists_p (directory_contents_t *p_dir, 
				       char *p_filename);
static directory_t *find_directory (char *psz_name);

/* Find the directory named NAME and return its `directory_t'.  */

static directory_t *
find_directory (char *name)
{
  char *p;
  directory_t *dir;
  directory_t **dir_slot;
  directory_t dir_key;
  int r;
#ifdef WINDOWS32
  char* w32_path;
  char  fs_label[BUFSIZ];
  char  fs_type[BUFSIZ];
  unsigned long  fs_serno;
  unsigned long  fs_flags;
  unsigned long  fs_len;
#endif

  dir_key.name = name;
  dir_slot = (directory_t **) hash_find_slot (&directories, &dir_key);
  dir = *dir_slot;

  if (HASH_VACANT (dir))
    {
      struct stat st;

      /* The directory was not found.  Create a new entry for it.  */

      p = name + strlen (name);
      dir = (directory_t *) xmalloc (sizeof (directory_t));
      dir->name = savestring (name, p - name);
      hash_insert_at (&directories, dir, dir_slot);
      /* The directory is not in the name hash table.
	 Find its device and inode numbers, and look it up by them.  */

#ifdef WINDOWS32
      /* Remove any trailing '\'.  Windows32 stat fails even on valid
         directories if they end in '\'. */
      if (p[-1] == '\\')
        p[-1] = '\0';
#endif

      EINTRLOOP (r, stat (name, &st));

#ifdef WINDOWS32
      /* Put back the trailing '\'.  If we don't, we're permanently
         truncating the value!  */
      if (p[-1] == '\0')
        p[-1] = '\\';
#endif

      if (r < 0)
        {
	/* Couldn't stat the directory.  Mark this by
	   setting the `contents' member to a nil pointer.  */
	  dir->contents = 0;
	}
      else
	{
	  /* Search the contents hash table; device and inode are the key.  */

	  directory_contents_t *dc;
	  directory_contents_t **dc_slot;
	  directory_contents_t dc_key;

	  dc_key.dev = st.st_dev;
#ifdef WINDOWS32
	  dc_key.path_key = w32_path = w32ify (name, 1);
	  dc_key.ctime = st.st_ctime;
#else
	  dc_key.ino = st.st_ino;
#endif
	  dc_slot = (directory_contents_t **) hash_find_slot (&directory_contents, &dc_key);
	  dc = *dc_slot;

	  if (HASH_VACANT (dc))
	    {
	      /* Nope; this really is a directory we haven't seen before.  */

	      dc = CALLOC(directory_contents_t, 1);

	      /* Enter it in the contents hash table.  */
	      dc->dev = st.st_dev;
#ifdef WINDOWS32
              dc->path_key = strdup (w32_path);
	      dc->ctime = st.st_ctime;
              dc->mtime = st.st_mtime;

              /*
               * NTFS is the only WINDOWS32 filesystem that bumps mtime
               * on a directory when files are added/deleted from
               * a directory.
               */
              w32_path[3] = '\0';
              if (GetVolumeInformation(w32_path,
                     fs_label, sizeof (fs_label),
                     &fs_serno, &fs_len,
                     &fs_flags, fs_type, sizeof (fs_type)) == FALSE)
                dc->fs_flags = FS_UNKNOWN;
              else if (!strcmp(fs_type, "FAT"))
                dc->fs_flags = FS_FAT;
              else if (!strcmp(fs_type, "NTFS"))
                dc->fs_flags = FS_NTFS;
              else
                dc->fs_flags = FS_UNKNOWN;
#else
	      dc->ino = st.st_ino;
#endif /* WINDOWS32 */
	      hash_insert_at (&directory_contents, dc, dc_slot);
	      ENULLLOOP (dc->dirstream, opendir (name));
	      if (dc->dirstream == 0)
                /* Couldn't open the directory.  Mark this by
                   setting the `files' member to a nil pointer.  */
                dc->dirfiles.ht_vec = NULL;
	      else
		{
		  hash_init (&dc->dirfiles, DIRFILE_BUCKETS,
			     dirfile_hash_1, dirfile_hash_2, dirfile_hash_cmp);
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
dir_contents_file_exists_p (directory_contents_t *dir, char *filename)
{
  unsigned int hash;
  dirfile_t *df;
  struct dirent *d;
#ifdef WINDOWS32
  struct stat st;
  int rehash = 0;
#endif

  if (dir == 0 || dir->dirfiles.ht_vec == 0)
    {
    /* The directory could not be stat'd or opened.  */
      return 0;
    }
#ifdef __MSDOS__
  filename = dosify (filename);
#endif

#ifdef HAVE_CASE_INSENSITIVE_FS
  filename = downcase (filename);
#endif

  hash = 0;
  if (filename != 0)
    {
      dirfile_t dirfile_key;

      if (*filename == '\0')
	{
	  /* Checking if the directory exists.  */
	  return 1;
	}
      dirfile_key.name = filename;
      dirfile_key.length = strlen (filename);
      df = (dirfile_t *) hash_find_item (&dir->dirfiles, &dirfile_key);
      if (df)
	{
	  return !df->impossible;
	}
    }

  /* The file was not found in the hashed list.
     Try to read the directory further.  */

  if (dir->dirstream == 0)
    {
#ifdef WINDOWS32
      /*
       * Check to see if directory has changed since last read. FAT
       * filesystems force a rehash always as mtime does not change
       * on directories (ugh!).
       */
      if (dir->path_key)
	{
          if (!(dir->fs_flags & FS_FAT)
              && (stat(dir->path_key, &st) == 0
                  && st.st_mtime > dir->mtime))
            /* reset date stamp to show most recent re-process */
            dir->mtime = st.st_mtime;

	  /* make sure directory can still be opened */
	  dir->dirstream = opendir(dir->path_key);

	  if (dir->dirstream)
	    rehash = 1;
	  else
	    return 0; /* couldn't re-read - fail */
	}
      else
#endif
	/* The directory has been all read in.  */
	return 0;
    }

  while (1)
    {
      /* Enter the file in the hash table.  */
      unsigned int len;
      dirfile_t dirfile_key;
      dirfile_t **dirfile_slot;

      ENULLLOOP (d, readdir (dir->dirstream));
      if (d == 0)
        break;

      if (!REAL_DIR_ENTRY (d))
	continue;

      len = NAMLEN (d);
      dirfile_key.name = d->d_name;
      dirfile_key.length = len;
      dirfile_slot = (dirfile_t **) hash_find_slot (&dir->dirfiles, &dirfile_key);
#ifdef WINDOWS32
      /*
       * If re-reading a directory, don't cache files that have
       * already been discovered.
       */
      if (! rehash || HASH_VACANT (*dirfile_slot))
#endif
	{
	  df = CALLOC(dirfile_t, 1);
	  df->name = savestring (d->d_name, len);
	  df->length = len;
	  df->impossible = 0;
	  hash_insert_at (&dir->dirfiles, df, dirfile_slot);
	}
      /* Check if the name matches the one we're searching for.  */
      if (filename != 0 && strieq (d->d_name, filename))
	{
	  return 1;
	}
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

/*! Return 1 if the name PSZ_FILENAME in directory PSZ_DIRNAME is
   entered in the dir hash table.  PSZ_FILENAME must contain no
   slashes.  */
int
dir_file_exists_p (char *psz_dirname, char *psz_filename)
{
  return dir_contents_file_exists_p (find_directory (psz_dirname)->contents,
				     psz_filename);
}


/* Return 1 if the file named PSZ_NAME exists.  */
int
file_exists_p (char *psz_name)
{
  char *dirend;
  char *dirname;
  char *slash;

#ifndef	NO_ARCHIVES
  if (ar_name (psz_name))
    return ar_member_date (psz_name) != (time_t) -1;
#endif

  dirend = strrchr (psz_name, '/');

#ifdef HAVE_DOS_PATHS
  /* Forward and backslashes might be mixed.  We need the rightmost one.  */
  {
    char *bslash = strrchr(psz_name, '\\');
    if (!dirend || bslash > dirend)
      dirend = bslash;
    /* The case of "d:file".  */
    if (!dirend && psz_name[0] && psz_name[1] == ':')
      dirend = psz_name + 1;
  }
#endif /* HAVE_DOS_PATHS */

  if (dirend == 0)
    return dir_file_exists_p (".", psz_name);

  slash = dirend;
  if (dirend == psz_name)
    dirname = "/";
  else
    {
#ifdef HAVE_DOS_PATHS
  /* d:/ and d: are *very* different...  */
      if (dirend < psz_name + 3 && psz_name[1] == ':' &&
	  (*dirend == '/' || *dirend == '\\' || *dirend == ':'))
	dirend++;
#endif
      dirname = (char *) alloca (dirend - psz_name + 1);
      memmove (dirname, psz_name, dirend - psz_name);
      dirname[dirend - psz_name] = '\0';
    }
  return dir_file_exists_p (dirname, slash + 1);
}


/*! Mark FILENAME as `impossible' for `file_impossible_p'.
  This means an attempt has been made to search for PSZ_FILENAME
  as an intermediate file, and it has failed.  */
void
file_impossible (char *psz_filename)
{
  char *dirend;
  char *p = psz_filename;
  directory_t *dir;
  dirfile_t *new;

  dirend = strrchr (p, '/');

#ifdef HAVE_DOS_PATHS
  /* Forward and backslashes might be mixed.  We need the rightmost one.  */
  {
    char *bslash = strrchr(p, '\\');
    if (!dirend || bslash > dirend)
      dirend = bslash;
    /* The case of "d:file".  */
    if (!dirend && p[0] && p[1] == ':')
      dirend = p + 1;
  }
#endif /* HAVE_DOS_PATHS */

  if (dirend == 0)
    dir = find_directory (".");
  else
    {
      char *dirname;
      char *slash = dirend;
      if (dirend == p)
	dirname = "/";
      else
	{
#ifdef HAVE_DOS_PATHS
	  /* d:/ and d: are *very* different...  */
	  if (dirend < p + 3 && p[1] == ':' &&
	      (*dirend == '/' || *dirend == '\\' || *dirend == ':'))
	    dirend++;
#endif
	  dirname = (char *) alloca (dirend - p + 1);
	  memmove (dirname, p, dirend - p);
	  dirname[dirend - p] = '\0';
	}
      dir = find_directory (dirname);
      psz_filename = p = slash + 1;
    }

  if (!dir->contents)
    {
      /* The directory could not be stat'd.  We allocate a contents
	 structure for it, but leave it out of the contents hash table.  */
      dir->contents = CALLOC(directory_contents_t, 1);
    }

  if (!dir->contents->dirfiles.ht_vec)
    {
      hash_init (&dir->contents->dirfiles, DIRFILE_BUCKETS,
		 dirfile_hash_1, dirfile_hash_2, dirfile_hash_cmp);
    }

  /* Make a new entry and put it in the table.  */

  new = CALLOC(dirfile_t, 1);
  new->name = strdup (psz_filename);
  new->length = strlen (psz_filename);
  new->impossible = 1;
  hash_insert (&dir->contents->dirfiles, new);
}

/*! Return nonzero if PSZ_FILENAME has been marked impossible.  */
int
file_impossible_p (char *psz_filename)
{
  char *dirend;
  char *p = psz_filename;
  directory_contents_t *dir;
  dirfile_t *dirfile;
  dirfile_t dirfile_key;

  dirend = strrchr (psz_filename, '/');

#ifdef HAVE_DOS_PATHS
  /* Forward and backslashes might be mixed.  We need the rightmost one.  */
  {
    char *bslash = strrchr(psz_filename, '\\');
    if (!dirend || bslash > dirend)
      dirend = bslash;
    /* The case of "d:file".  */
    if (!dirend && psz_filename[0] && psz_filename[1] == ':')
      dirend = psz_filename + 1;
  }
#endif /* HAVE_DOS_PATHS */

  if (dirend == 0)
    dir = find_directory (".")->contents;
  else
    {
      char *dirname;
      char *slash = dirend;
      if (dirend == psz_filename)
	dirname = "/";
      else
	{

#ifdef HAVE_DOS_PATHS
	  /* d:/ and d: are *very* different...  */
	  if (dirend < psz_filename + 3 && psz_filename[1] == ':' &&
	      (*dirend == '/' || *dirend == '\\' || *dirend == ':'))
	    dirend++;
#endif

	  dirname = (char *) alloca (dirend - psz_filename + 1);
	  memmove (dirname, p, dirend - p);
	  dirname[dirend - p] = '\0';
	}
      dir = find_directory (dirname)->contents;
      p = psz_filename = slash + 1;
    }

  if (dir == 0 || dir->dirfiles.ht_vec == 0)
    /* There are no files entered for this directory.  */
    return 0;

#ifdef __MSDOS__
  psz_filename = dosify (p);
#endif
#ifdef HAVE_CASE_INSENSITIVE_FS
  psz_filename = downcase (p);
#endif

  dirfile_key.name = psz_filename;
  dirfile_key.length = strlen (psz_filename);
  dirfile = (dirfile_t *) hash_find_item (&dir->dirfiles, &dirfile_key);
  if (dirfile)
    return dirfile->impossible;

  return 0;
}

/* Return the already allocated name in the
   directory hash table that matches DIR.  */
char *
dir_name (char *psz_dir)
{
  return find_directory (psz_dir)->name;
}

/* Print the data base of directories.  */

void
print_dir_data_base (void)
{
  unsigned int files;
  unsigned int impossible;
  directory_t **dir_slot;
  directory_t **dir_end;

  puts (_("\n# Directories\n"));

  files = impossible = 0;

  dir_slot = (directory_t **) directories.ht_vec;
  dir_end = dir_slot + directories.ht_size;
  for ( ; dir_slot < dir_end; dir_slot++)
    {
      directory_t *dir = *dir_slot;
      if (! HASH_VACANT (dir))
	{
	  if (dir->contents == 0)
	    printf (_("# %s: could not be stat'd.\n"), dir->name);
	  else if (dir->contents->dirfiles.ht_vec == 0)
	    {
#ifdef WINDOWS32
	      printf (_("# %s (key %s, mtime %d): could not be opened.\n"),
		      dir->name, dir->contents->path_key,dir->contents->mtime);
#else  /* WINDOWS32 */
	      printf (_("# %s (device %ld, inode %ld): could not be opened.\n"),
		      dir->name, (long int) dir->contents->dev,
		      (long int) dir->contents->ino);
#endif /* WINDOWS32 */
	    }
	  else
	    {
	      unsigned int f = 0;
	      unsigned int im = 0;
	      dirfile_t **files_slot;
	      dirfile_t **files_end;

	      files_slot = (dirfile_t **) dir->contents->dirfiles.ht_vec;
	      files_end = files_slot + dir->contents->dirfiles.ht_size;
	      for ( ; files_slot < files_end; files_slot++)
		{
		  dirfile_t *df = *files_slot;
		  if (! HASH_VACANT (df))
		    {
		      if (df->impossible)
			++im;
		      else
			++f;
		    }
		}
#ifdef WINDOWS32
	      printf (_("# %s (key %s, mtime %d): "),
		      dir->name, dir->contents->path_key, dir->contents->mtime);
#else  /* !WINDOWS32 */
	      printf (_("# %s (device %ld, inode %ld): "),
		      dir->name,
		      (long)dir->contents->dev, (long)dir->contents->ino);
#endif /* WINDOWS32 */
	      if (f == 0)
		fputs (_("No"), stdout);
	      else
		printf ("%u", f);
	      fputs (_(" files, "), stdout);
	      if (im == 0)
		fputs (_("no"), stdout);
	      else
		printf ("%u", im);
	      fputs (_(" impossibilities"), stdout);
	      if (dir->contents->dirstream == 0)
		puts (".");
	      else
		puts (_(" so far."));
	      files += f;
	      impossible += im;
	    }
	}
    }

  fputs ("\n# ", stdout);
  if (files == 0)
    fputs (_("No"), stdout);
  else
    printf ("%u", files);
  fputs (_(" files, "), stdout);
  if (impossible == 0)
    fputs (_("no"), stdout);
  else
    printf ("%u", impossible);
  printf (_(" impossibilities in %lu directories.\n"), directories.ht_fill);
}

/* Hooks for globbing.  */

#include <glob.h>

/* Structure describing state of iterating through a directory hash table.  */

struct dirstream
  {
    directory_contents_t *contents; /* The directory being read.  */
    dirfile_t **dirfile_slot; /* Current slot in table.  */
  };

/* Forward declarations.  */
static void * open_dirstream (const char *);
static struct dirent *read_dirstream (void *);

static void *
open_dirstream (const char *directory)
{
  struct dirstream *new;
  directory_t *dir = find_directory ((char *)directory);

  if (dir->contents == 0 || dir->contents->dirfiles.ht_vec == 0)
    /* DIR->contents is nil if the directory could not be stat'd.
       DIR->contents->dirfiles is nil if it could not be opened.  */
    return 0;

  /* Read all the contents of the directory now.  There is no benefit
     in being lazy, since glob will want to see every file anyway.  */

  (void) dir_contents_file_exists_p (dir->contents, (char *) 0);

  new = (struct dirstream *) xmalloc (sizeof (struct dirstream));
  new->contents = dir->contents;
  new->dirfile_slot = (dirfile_t **) new->contents->dirfiles.ht_vec;

  return (void *) new;
}

static struct dirent *
read_dirstream (void * stream)
{
  struct dirstream *const ds = (struct dirstream *) stream;
  directory_contents_t *dc = ds->contents;
  dirfile_t **dirfile_end = (dirfile_t **) dc->dirfiles.ht_vec + dc->dirfiles.ht_size;
  static char *buf;
  static unsigned int bufsz;

  while (ds->dirfile_slot < dirfile_end)
    {
      dirfile_t *df = *ds->dirfile_slot++;
      if (! HASH_VACANT (df) && !df->impossible)
	{
	  /* The glob interface wants a `struct dirent',
	     so mock one up.  */
	  struct dirent *d;
	  unsigned int len = df->length + 1;
	  if (sizeof *d - sizeof d->d_name + len > bufsz)
	    {
	      if (buf != 0)
		free (buf);
	      bufsz *= 2;
	      if (sizeof *d - sizeof d->d_name + len > bufsz)
		bufsz = sizeof *d - sizeof d->d_name + len;
	      buf = xmalloc (bufsz);
	    }
	  d = (struct dirent *) buf;
#ifdef __MINGW32__
# if __MINGW32_VERSION_MAJOR < 3 || (__MINGW32_VERSION_MAJOR == 3 && \
				     __MINGW32_VERSION_MINOR == 0)
	  d->d_name = xmalloc(len);
# endif
#endif
	  FAKE_DIR_ENTRY (d);
#ifdef _DIRENT_HAVE_D_NAMLEN
	  d->d_namlen = len - 1;
#endif
#ifdef _DIRENT_HAVE_D_TYPE
	  d->d_type = DT_UNKNOWN;
#endif
	  memcpy (d->d_name, df->name, len);
	  return d;
	}
    }

  return 0;
}

/* On 64 bit ReliantUNIX (5.44 and above) in LFS mode, stat() is actually a
 * macro for stat64().  If stat is a macro, make a local wrapper function to
 * invoke it.
 */
#ifndef stat
# define local_stat stat
#else
static int
local_stat (const char *path, struct stat *buf)
{
  int e;

  EINTRLOOP (e, stat (path, buf));
  return e;
}
#endif

void
dir_setup_glob (glob_t *gl)
{
  /* Bogus sunos4 compiler complains (!) about & before functions.  */
  gl->gl_opendir = open_dirstream;
  gl->gl_readdir = read_dirstream;
  gl->gl_closedir = free;
  gl->gl_stat = local_stat;
  /* We don't bother setting gl_lstat, since glob never calls it.
     The slot is only there for compatibility with 4.4 BSD.  */
}

void
hash_init_directories (void)
{
  hash_init (&directories, DIRECTORY_BUCKETS,
	     directory_hash_1, directory_hash_2, directory_hash_cmp);
  hash_init (&directory_contents, DIRECTORY_BUCKETS,
	     directory_contents_hash_1, directory_contents_hash_2, directory_contents_hash_cmp);
}

/* Free memory consumed by p_dir. */
static void 
dir_free_dirfile(dirfile_t *p_dirfile) 
{
  /* Sometimes p_dirfile->name is not null but whatever it's pointed
     to doesn't exist. Some kind of sharing here? However
     p_dirfile->length is 0 in these cases.

   */
  if (p_dirfile->length) FREE(p_dirfile->name);
  FREE(p_dirfile);
}

/* Free memory consumed by p_dir. */
static void 
dir_free(directory_t *p_dir) 
{
  if (p_dir->contents)
    hash_free(&(p_dir->contents->dirfiles), (free_fn_t) dir_free_dirfile);
  if (p_dir->name) FREE(p_dir->name);
  FREE(p_dir);
}

/*! Free hashes for directories and directory_contents. */
void
hash_free_directories (void)
{
  hash_free(&directories, (free_fn_t) dir_free);
#if FIXED
  hash_free(&directory_contents, (free_fn_t) dir_free_dirfile);
#else 
  hash_free(&directory_contents, free);
#endif
}
