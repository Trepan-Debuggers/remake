/* Directory hashing for GNU Make.
Copyright (C) 1988,89,91,92,93,94,95,96,97 Free Software Foundation, Inc.
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

#include "make.h"

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
# ifdef HAVE_VMSDIR_H
#  include "vmsdir.h"
# endif /* HAVE_VMSDIR_H */
#endif

/* In GNU systems, <dirent.h> defines this macro for us.  */
#ifdef _D_NAMLEN
#undef NAMLEN
#define NAMLEN(d) _D_NAMLEN(d)
#endif

#if (defined (POSIX) || defined (WINDOWS32)) && !defined (__GNU_LIBRARY__)
/* Posix does not require that the d_ino field be present, and some
   systems do not provide it. */
#define REAL_DIR_ENTRY(dp) 1
#define FAKE_DIR_ENTRY(dp)
#else
#define REAL_DIR_ENTRY(dp) (dp->d_ino != 0)
#define FAKE_DIR_ENTRY(dp) (dp->d_ino = 1)
#endif /* POSIX */

#ifdef __MSDOS__
#include <ctype.h>
#include <fcntl.h>

/* If it's MSDOS that doesn't have _USE_LFN, disable LFN support.  */
#ifndef _USE_LFN
#define _USE_LFN 0
#endif

static char *
dosify (filename)
     char *filename;
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
    *df++ = tolower (*filename++);

  /* Now skip to the next dot.  */
  while (*filename != '\0' && *filename != '.')
    ++filename;
  if (*filename != '\0')
    {
      *df++ = *filename++;
      for (i = 0; *filename != '\0' && i < 3 && *filename != '.'; ++i)
	*df++ = tolower (*filename++);
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

#ifdef _AMIGA
#include <ctype.h>
#endif

#ifdef HAVE_CASE_INSENSITIVE_FS
static char *
downcase (filename)
     char *filename;
{
#ifdef _AMIGA
  static char new_filename[136];
#else
  static char new_filename[PATH_MAX];
#endif
  char *df;
  int i;

  if (filename == 0)
    return 0;

  df = new_filename;

  /* First, transform the name part.  */
  for (i = 0; *filename != '\0'; ++i)
  {
    *df++ = tolower (*filename);
    ++filename;
  }

  *df = 0;

  return new_filename;
}
#endif /* HAVE_CASE_INSENSITIVE_FS */

#ifdef VMS

static int
vms_hash (name)
    char *name;
{
  int h = 0;
  int g;

  while (*name)
    {
      h = (h << 4) + *name++;
      g = h & 0xf0000000;
      if (g)
	{
	  h = h ^ (g >> 24);
	  h = h ^ g;
	}
    }
  return h;
}

/* fake stat entry for a directory */
static int
vmsstat_dir (name, st)
    char *name;
    struct stat *st;
{
  char *s;
  int h;
  DIR *dir;

  dir = opendir (name);
  if (dir == 0)
    return -1;
  closedir (dir);
  s = strchr (name, ':');	/* find device */
  if (s)
    {
      *s++ = 0;
      st->st_dev = (char *)vms_hash (name);
      h = vms_hash (s);
      *(s-1) = ':';
    }
  else
    {
      st->st_dev = 0;
      s = name;
      h = vms_hash (s);
    }

  st->st_ino[0] = h & 0xff;
  st->st_ino[1] = h & 0xff00;
  st->st_ino[2] = h >> 16;

  return 0;
}
#endif /* VMS */

/* Hash table of directories.  */

#ifndef	DIRECTORY_BUCKETS
#define DIRECTORY_BUCKETS 199
#endif

struct directory_contents
  {
    struct directory_contents *next;

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
    int   mtime;        /* controls check for stale directory cache */
    int   fs_flags;     /* FS_FAT, FS_NTFS, ... */
#define FS_FAT      0x1
#define FS_NTFS     0x2
#define FS_UNKNOWN  0x4
#else
#ifdef VMS
    ino_t ino[3];
#else
    ino_t ino;
#endif
#endif /* WINDOWS32 */
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

static int dir_contents_file_exists_p PARAMS ((struct directory_contents *dir, char *filename));
static struct directory *find_directory PARAMS ((char *name));

/* Find the directory named NAME and return its `struct directory'.  */

static struct directory *
find_directory (name)
     register char *name;
{
  register unsigned int hash = 0;
  register char *p;
  register struct directory *dir;
#ifdef WINDOWS32
  char* w32_path;
  char  fs_label[BUFSIZ];
  char  fs_type[BUFSIZ];
  long  fs_serno;
  long  fs_flags;
  long  fs_len;
#endif
#ifdef VMS
  if ((*name == '.') && (*(name+1) == 0))
    name = "[]";
  else
    name = vmsify (name,1);
#endif

  for (p = name; *p != '\0'; ++p)
    HASHI (hash, *p);
  hash %= DIRECTORY_BUCKETS;

  for (dir = directories[hash]; dir != 0; dir = dir->next)
    if (strieq (dir->name, name))
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

#ifdef VMS
      if (vmsstat_dir (name, &st) < 0)
#else

#ifdef WINDOWS32
      /* Remove any trailing '\'.  Windows32 stat fails even on valid
         directories if they end in '\'. */
      if (p[-1] == '\\')
        p[-1] = '\0';
#endif
      if (stat (name, &st) < 0)
#endif
	{
	/* Couldn't stat the directory.  Mark this by
	   setting the `contents' member to a nil pointer.  */
	  dir->contents = 0;
	}
      else
	{
	  /* Search the contents hash table; device and inode are the key.  */

	  struct directory_contents *dc;

#ifdef WINDOWS32
          w32_path = w32ify(name, 1);
          hash = ((unsigned int) st.st_dev << 16) | (unsigned int) st.st_ctime;
#else
#ifdef VMS
	hash = ((unsigned int) st.st_dev << 16)
		| ((unsigned int) st.st_ino[0]
		+ (unsigned int) st.st_ino[1]
		+ (unsigned int) st.st_ino[2]);
#else
	  hash = ((unsigned int) st.st_dev << 16) | (unsigned int) st.st_ino;
#endif
#endif
	  hash %= DIRECTORY_BUCKETS;

	  for (dc = directories_contents[hash]; dc != 0; dc = dc->next)
#ifdef WINDOWS32
            if (strieq(dc->path_key, w32_path))
#else
	    if (dc->dev == st.st_dev
#ifdef VMS
		&& dc->ino[0] == st.st_ino[0]
		&& dc->ino[1] == st.st_ino[1]
		&& dc->ino[2] == st.st_ino[2])
#else
		 && dc->ino == st.st_ino)
#endif
#endif /* WINDOWS32 */
	      break;

	  if (dc == 0)
	    {
	      /* Nope; this really is a directory we haven't seen before.  */

	      dc = (struct directory_contents *)
		xmalloc (sizeof (struct directory_contents));

	      /* Enter it in the contents hash table.  */
	      dc->dev = st.st_dev;
#ifdef WINDOWS32
              dc->path_key = xstrdup(w32_path);
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
#ifdef VMS
	      dc->ino[0] = st.st_ino[0];
	      dc->ino[1] = st.st_ino[1];
	      dc->ino[2] = st.st_ino[2];
#else
	      dc->ino = st.st_ino;
#endif
#endif /* WINDOWS32 */
	      dc->next = directories_contents[hash];
	      directories_contents[hash] = dc;

	      dc->dirstream = opendir (name);
	      if (dc->dirstream == 0)
		{
		/* Couldn't open the directory.  Mark this by
		   setting the `files' member to a nil pointer.  */
		  dc->files = 0;
		}
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
#ifdef WINDOWS32
  struct stat st;
  int rehash = 0;
#endif

  if (dir == 0 || dir->files == 0)
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

#ifdef VMS
  filename = vmsify (filename,0);
#endif

  hash = 0;
  if (filename != 0)
    {
      if (*filename == '\0')
	{
	/* Checking if the directory exists.  */
	  return 1;
	}

      for (p = filename; *p != '\0'; ++p)
	HASH (hash, *p);
      hash %= DIRFILE_BUCKETS;

      /* Search the list of hashed files.  */

      for (df = dir->files[hash]; df != 0; df = df->next)
	{
	  if (strieq (df->name, filename))
	    {
	      return !df->impossible;
	    }
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
      if (dir->path_key &&
          (dir->fs_flags & FS_FAT ||
           (stat(dir->path_key, &st) == 0 &&
            st.st_mtime > dir->mtime))) {

        /* reset date stamp to show most recent re-process */
        dir->mtime = st.st_mtime;

        /* make sure directory can still be opened */
        dir->dirstream = opendir(dir->path_key);

        if (dir->dirstream)
          rehash = 1;
        else
          return 0; /* couldn't re-read - fail */
      } else
#endif
    /* The directory has been all read in.  */
      return 0;
    }

  while ((d = readdir (dir->dirstream)) != 0)
    {
      /* Enter the file in the hash table.  */
      register unsigned int newhash = 0;
      unsigned int len;
      register unsigned int i;

      if (!REAL_DIR_ENTRY (d))
	continue;

      len = NAMLEN (d);
      for (i = 0; i < len; ++i)
	HASHI (newhash, d->d_name[i]);
      newhash %= DIRFILE_BUCKETS;
#ifdef WINDOWS32
      /*
       * If re-reading a directory, check that this file isn't already
       * in the cache.
       */
      if (rehash) {
        for (df = dir->files[newhash]; df != 0; df = df->next)
          if (streq(df->name, d->d_name))
            break;
      } else
        df = 0;

      /*
       * If re-reading a directory, don't cache files that have
       * already been discovered.
       */
      if (!df) {
#endif

      df = (struct dirfile *) xmalloc (sizeof (struct dirfile));
      df->next = dir->files[newhash];
      dir->files[newhash] = df;
      df->name = savestring (d->d_name, len);
      df->impossible = 0;
#ifdef WINDOWS32
      }
#endif
      /* Check if the name matches the one we're searching for.  */
      if (filename != 0
	  && newhash == hash && strieq (d->d_name, filename))
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
  char *slash;

#ifndef	NO_ARCHIVES
  if (ar_name (name))
    return ar_member_date (name) != (time_t) -1;
#endif

#ifdef VMS
  dirend = rindex (name, ']');
  dirend++;
  if (dirend == (char *)1)
    return dir_file_exists_p ("[]", name);
#else /* !VMS */
  dirend = rindex (name, '/');
#if defined (WINDOWS32) || defined (__MSDOS__)
  /* Forward and backslashes might be mixed.  We need the rightmost one.  */
  {
    char *bslash = rindex(name, '\\');
    if (!dirend || bslash > dirend)
      dirend = bslash;
    /* The case of "d:file".  */
    if (!dirend && name[0] && name[1] == ':')
      dirend = name + 1;
  }
#endif /* WINDOWS32 || __MSDOS__ */
  if (dirend == 0)
#ifndef _AMIGA
    return dir_file_exists_p (".", name);
#else /* !VMS && !AMIGA */
    return dir_file_exists_p ("", name);
#endif /* AMIGA */
#endif /* VMS */

  slash = dirend;
  if (dirend == name)
    dirname = "/";
  else
    {
#if defined (WINDOWS32) || defined (__MSDOS__)
  /* d:/ and d: are *very* different...  */
      if (dirend < name + 3 && name[1] == ':' &&
	  (*dirend == '/' || *dirend == '\\' || *dirend == ':'))
	dirend++;
#endif
      dirname = (char *) alloca (dirend - name + 1);
      bcopy (name, dirname, dirend - name);
      dirname[dirend - name] = '\0';
    }
  return dir_file_exists_p (dirname, slash + 1);
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

#ifdef VMS
  dirend = rindex (p, ']');
  dirend++;
  if (dirend == (char *)1)
    dir = find_directory ("[]");
#else
  dirend = rindex (p, '/');
#if defined (WINDOWS32) || defined (__MSDOS__)
  /* Forward and backslashes might be mixed.  We need the rightmost one.  */
  {
    char *bslash = rindex(p, '\\');
    if (!dirend || bslash > dirend)
      dirend = bslash;
    /* The case of "d:file".  */
    if (!dirend && p[0] && p[1] == ':')
      dirend = p + 1;
  }
#endif /* WINDOWS32 or __MSDOS__ */
  if (dirend == 0)
#ifdef _AMIGA
    dir = find_directory ("");
#else /* !VMS && !AMIGA */
    dir = find_directory (".");
#endif /* AMIGA */
#endif /* VMS */
  else
    {
      char *dirname;
      char *slash = dirend;
      if (dirend == p)
	dirname = "/";
      else
	{
#if defined (WINDOWS32) || defined (__MSDOS__)
	  /* d:/ and d: are *very* different...  */
	  if (dirend < p + 3 && p[1] == ':' &&
	      (*dirend == '/' || *dirend == '\\' || *dirend == ':'))
	    dirend++;
#endif
	  dirname = (char *) alloca (dirend - p + 1);
	  bcopy (p, dirname, dirend - p);
	  dirname[dirend - p] = '\0';
	}
      dir = find_directory (dirname);
      filename = p = slash + 1;
    }

  for (hash = 0; *p != '\0'; ++p)
    HASHI (hash, *p);
  hash %= DIRFILE_BUCKETS;

  if (dir->contents == 0)
    {
      /* The directory could not be stat'd.  We allocate a contents
	 structure for it, but leave it out of the contents hash table.  */
      dir->contents = (struct directory_contents *)
	xmalloc (sizeof (struct directory_contents));
#ifdef WINDOWS32
      dir->contents->path_key = NULL;
      dir->contents->mtime = 0;
#else  /* WINDOWS32 */
#ifdef VMS
      dir->contents->dev = 0;
      dir->contents->ino[0] = dir->contents->ino[1] =
	dir->contents->ino[2] = 0;
#else
      dir->contents->dev = dir->contents->ino = 0;
#endif
#endif /* WINDOWS32 */
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
  new->name = xstrdup (filename);
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

#ifdef VMS
  dirend = rindex (filename, ']');
  if (dirend == 0)
    dir = find_directory ("[]")->contents;
#else
  dirend = rindex (filename, '/');
#if defined (WINDOWS32) || defined (__MSDOS__)
  /* Forward and backslashes might be mixed.  We need the rightmost one.  */
  {
    char *bslash = rindex(filename, '\\');
    if (!dirend || bslash > dirend)
      dirend = bslash;
    /* The case of "d:file".  */
    if (!dirend && filename[0] && filename[1] == ':')
      dirend = filename + 1;
  }
#endif /* WINDOWS32 || __MSDOS__ */
  if (dirend == 0)
#ifdef _AMIGA
    dir = find_directory ("")->contents;
#else /* !VMS && !AMIGA */
    dir = find_directory (".")->contents;
#endif /* AMIGA */
#endif /* VMS */
  else
    {
      char *dirname;
      char *slash = dirend;
      if (dirend == filename)
	dirname = "/";
      else
	{
#if defined (WINDOWS32) || defined (__MSDOS__)
	  /* d:/ and d: are *very* different...  */
	  if (dirend < filename + 3 && filename[1] == ':' &&
	      (*dirend == '/' || *dirend == '\\' || *dirend == ':'))
	    dirend++;
#endif
	  dirname = (char *) alloca (dirend - filename + 1);
	  bcopy (p, dirname, dirend - p);
	  dirname[dirend - p] = '\0';
	}
      dir = find_directory (dirname)->contents;
      p = filename = slash + 1;
    }

  if (dir == 0 || dir->files == 0)
    /* There are no files entered for this directory.  */
    return 0;

#ifdef __MSDOS__
  p = filename = dosify (p);
#endif
#ifdef HAVE_CASE_INSENSITIVE_FS
  p = filename = downcase (p);
#endif
#ifdef VMS
  p = filename = vmsify (p, 1);
#endif

  for (hash = 0; *p != '\0'; ++p)
    HASH (hash, *p);
  hash %= DIRFILE_BUCKETS;

  for (next = dir->files[hash]; next != 0; next = next->next)
    if (strieq (filename, next->name))
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

  puts (_("\n# Directories\n"));

  dirs = files = impossible = 0;
  for (i = 0; i < DIRECTORY_BUCKETS; ++i)
    for (dir = directories[i]; dir != 0; dir = dir->next)
      {
	++dirs;
	if (dir->contents == 0)
	  printf (_("# %s: could not be stat'd.\n"), dir->name);
	else if (dir->contents->files == 0)
#ifdef WINDOWS32
          printf (_("# %s (key %s, mtime %d): could not be opened.\n"),
                  dir->name, dir->contents->path_key,dir->contents->mtime);
#else  /* WINDOWS32 */
#ifdef VMS
	  printf (_("# %s (device %d, inode [%d,%d,%d]): could not be opened.\n"),
		  dir->name, dir->contents->dev,
		  dir->contents->ino[0], dir->contents->ino[1],
		  dir->contents->ino[2]);
#else
	  printf (_("# %s (device %ld, inode %ld): could not be opened.\n"),
		  dir->name, (long int) dir->contents->dev,
		  (long int) dir->contents->ino);
#endif
#endif /* WINDOWS32 */
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
#ifdef WINDOWS32
            printf (_("# %s (key %s, mtime %d): "),
                    dir->name, dir->contents->path_key, dir->contents->mtime);
#else  /* WINDOWS32 */
#ifdef VMS
	    printf (_("# %s (device %d, inode [%d,%d,%d]): "),
		    dir->name, dir->contents->dev,
			dir->contents->ino[0], dir->contents->ino[1],
			dir->contents->ino[2]);
#else
	    printf (_("# %s (device %ld, inode %ld): "),
		    dir->name,
                    (long)dir->contents->dev, (long)dir->contents->ino);
#endif
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
  printf (_(" impossibilities in %u directories.\n"), dirs);
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
static __ptr_t open_dirstream PARAMS ((const char *));
static struct dirent *read_dirstream PARAMS ((__ptr_t));

static __ptr_t
open_dirstream (directory)
     const char *directory;
{
  struct dirstream *new;
  struct directory *dir = find_directory ((char *)directory);

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

static struct dirent *
read_dirstream (stream)
     __ptr_t stream;
{
  struct dirstream *const ds = (struct dirstream *) stream;
  register struct dirfile *df;
  static char *buf;
  static unsigned int bufsz;

  while (ds->bucket < DIRFILE_BUCKETS)
    {
      while ((df = ds->elt) != 0)
	{
	  ds->elt = df->next;
	  if (!df->impossible)
	    {
	      /* The glob interface wants a `struct dirent',
		 so mock one up.  */
	      struct dirent *d;
	      unsigned int len = strlen (df->name) + 1;
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
	      FAKE_DIR_ENTRY (d);
#ifdef _DIRENT_HAVE_D_NAMLEN
	      d->d_namlen = len - 1;
#endif
	      memcpy (d->d_name, df->name, len);
	      return d;
	    }
	}
      if (++ds->bucket == DIRFILE_BUCKETS)
	break;
      ds->elt = ds->contents->files[ds->bucket];
    }

  return 0;
}

static void
ansi_free(p)
  void *p;
{
    if (p)
      free(p);
}

void
dir_setup_glob (gl)
     glob_t *gl;
{
#ifndef VMS
  extern int stat ();
#endif

  /* Bogus sunos4 compiler complains (!) about & before functions.  */
  gl->gl_opendir = open_dirstream;
  gl->gl_readdir = read_dirstream;
  gl->gl_closedir = ansi_free;
  gl->gl_stat = stat;
  /* We don't bother setting gl_lstat, since glob never calls it.
     The slot is only there for compatibility with 4.4 BSD.  */
}
