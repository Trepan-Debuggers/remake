/* Begin of l10nflist.c */

/* Handle list of needed message catalogs
   Copyright (C) 1995, 1996, 1997 Free Software Foundation, Inc.
   Contributed by Ulrich Drepper <drepper@gnu.ai.mit.edu>, 1995.

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2, or (at your option)
   any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software Foundation,
   Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.  */

#if HAVE_CONFIG_H
# include <config.h>
#endif

#ifdef __GNUC__
# define alloca __builtin_alloca
# define HAVE_ALLOCA 1
#else
# if defined HAVE_ALLOCA_H || defined _LIBC
#  include <alloca.h>
# else
#  ifdef _AIX
 #pragma alloca
#  else
#   ifndef alloca
char *alloca ();
#   endif
#  endif
# endif
#endif

#if defined HAVE_STRING_H || defined _LIBC
# ifndef _GNU_SOURCE
#  define _GNU_SOURCE	1
# endif
# include <string.h>
#else
# include <strings.h>
# ifndef memcpy
#  define memcpy(Dst, Src, Num) bcopy (Src, Dst, Num)
# endif
#endif
#if !HAVE_STRCHR && !defined _LIBC
# ifndef strchr
#  define strchr index
# endif
#endif

#if defined STDC_HEADERS || defined _LIBC
#else
char *getenv ();
# ifdef HAVE_MALLOC_H
# else
void free ();
# endif
#endif

#if defined _LIBC || defined HAVE_ARGZ_H
# include <argz.h>
#endif
#include <ctype.h>
#include <sys/types.h>

#if defined STDC_HEADERS || defined _LIBC
# include <stdlib.h>
#else
# ifdef HAVE_MEMORY_H
#  include <memory.h>
# endif
#endif

/* Interrupt of l10nflist.c */

/* Begin of loadinfo.h */

/* Copyright (C) 1996, 1997 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contributed by Ulrich Drepper <drepper@cygnus.com>, 1996.  */

#ifndef PARAMS
# if __STDC__
#  define PARAMS(args) args
# else
#  define PARAMS(args) ()
# endif
#endif

/* Encoding of locale name parts.  */
#define CEN_REVISION		1
#define CEN_SPONSOR		2
#define CEN_SPECIAL		4
#define XPG_NORM_CODESET	8
#define XPG_CODESET		16
#define TERRITORY		32
#define CEN_AUDIENCE		64
#define XPG_MODIFIER		128

#define CEN_SPECIFIC	(CEN_REVISION|CEN_SPONSOR|CEN_SPECIAL|CEN_AUDIENCE)
#define XPG_SPECIFIC	(XPG_CODESET|XPG_NORM_CODESET|XPG_MODIFIER)

struct loaded_l10nfile
{
  const char *filename;
  int decided;

  const void *data;

  struct loaded_l10nfile *next;
  struct loaded_l10nfile *successor[1];
};

static const char *_nl_normalize_codeset PARAMS ((const unsigned char *codeset,
						  size_t name_len));

static struct loaded_l10nfile *
_nl_make_l10nflist PARAMS ((struct loaded_l10nfile **l10nfile_list,
			    const char *dirlist, size_t dirlist_len, int mask,
			    const char *language, const char *territory,
			    const char *codeset,
			    const char *normalized_codeset,
			    const char *modifier, const char *special,
			    const char *sponsor, const char *revision,
			    const char *filename, int do_allocate));

static const char *_nl_expand_alias PARAMS ((const char *name));

static int _nl_explode_name PARAMS ((char *name, const char **language,
				     const char **modifier,
				     const char **territory,
				     const char **codeset,
				     const char **normalized_codeset,
				     const char **special,
				     const char **sponsor,
				     const char **revision));

/* End of loadinfo.h */

/* Resume of l10nflist.c */

/* On some strange systems still no definition of NULL is found.  Sigh!  */
#ifndef NULL
# if defined __STDC__ && __STDC__
#  define NULL ((void *) 0)
# else
#  define NULL 0
# endif
#endif

#ifdef _LIBC
/* Rename the non ANSI C functions.  This is required by the standard
   because some ANSI C functions will require linking with this object
   file and the name space must not be polluted.  */
# ifndef stpcpy
#  define stpcpy(dest, src) __stpcpy(dest, src)
# endif
#else
# ifndef HAVE_STPCPY
static char *stpcpy PARAMS ((char *dest, const char *src));
# endif
#endif

/* Define function which are usually not available.  */

#if !defined _LIBC && !defined HAVE___ARGZ_COUNT
/* Returns the number of strings in ARGZ.  */
static size_t argz_count__ PARAMS ((const char *argz, size_t len));

static size_t
argz_count__ (argz, len)
     const char *argz;
     size_t len;
{
  size_t count = 0;
  while (len > 0)
    {
      size_t part_len = strlen (argz);
      argz += part_len + 1;
      len -= part_len + 1;
      count++;
    }
  return count;
}
# undef __argz_count
# define __argz_count(argz, len) argz_count__ (argz, len)
#endif	/* !_LIBC && !HAVE___ARGZ_COUNT */

#if !defined _LIBC && !defined HAVE___ARGZ_STRINGIFY
/* Make '\0' separated arg vector ARGZ printable by converting all the '\0's
   except the last into the character SEP.  */
static void argz_stringify__ PARAMS ((char *argz, size_t len, int sep));

static void
argz_stringify__ (argz, len, sep)
     char *argz;
     size_t len;
     int sep;
{
  while (len > 0)
    {
      size_t part_len = strlen (argz);
      argz += part_len;
      len -= part_len + 1;
      if (len > 0)
	*argz++ = sep;
    }
}
# undef __argz_stringify
# define __argz_stringify(argz, len, sep) argz_stringify__ (argz, len, sep)
#endif	/* !_LIBC && !HAVE___ARGZ_STRINGIFY */

#if !defined _LIBC && !defined HAVE___ARGZ_NEXT
static char *argz_next__ PARAMS ((char *argz, size_t argz_len,
				  const char *entry));

static char *
argz_next__ (argz, argz_len, entry)
     char *argz;
     size_t argz_len;
     const char *entry;
{
  if (entry)
    {
      if (entry < argz + argz_len)
        entry = strchr (entry, '\0') + 1;

      return entry >= argz + argz_len ? NULL : (char *) entry;
    }
  else
    if (argz_len > 0)
      return argz;
    else
      return 0;
}
# undef __argz_next
# define __argz_next(argz, len, entry) argz_next__ (argz, len, entry)
#endif	/* !_LIBC && !HAVE___ARGZ_NEXT */

/* Return number of bits set in X.  */
static int pop PARAMS ((int x));

static inline int
pop (x)
     int x;
{
  /* We assume that no more than 16 bits are used.  */
  x = ((x & ~0x5555) >> 1) + (x & 0x5555);
  x = ((x & ~0x3333) >> 2) + (x & 0x3333);
  x = ((x >> 4) + x) & 0x0f0f;
  x = ((x >> 8) + x) & 0xff;

  return x;
}

static struct loaded_l10nfile *
_nl_make_l10nflist (l10nfile_list, dirlist, dirlist_len, mask, language,
		    territory, codeset, normalized_codeset, modifier, special,
		    sponsor, revision, filename, do_allocate)
     struct loaded_l10nfile **l10nfile_list;
     const char *dirlist;
     size_t dirlist_len;
     int mask;
     const char *language;
     const char *territory;
     const char *codeset;
     const char *normalized_codeset;
     const char *modifier;
     const char *special;
     const char *sponsor;
     const char *revision;
     const char *filename;
     int do_allocate;
{
  char *abs_filename;
  struct loaded_l10nfile *last = NULL;
  struct loaded_l10nfile *retval;
  char *cp;
  size_t entries;
  int cnt;

  /* Allocate room for the full file name.  */
  abs_filename = (char *) malloc (dirlist_len
				  + strlen (language)
				  + ((mask & TERRITORY) != 0
				     ? strlen (territory) + 1 : 0)
				  + ((mask & XPG_CODESET) != 0
				     ? strlen (codeset) + 1 : 0)
				  + ((mask & XPG_NORM_CODESET) != 0
				     ? strlen (normalized_codeset) + 1 : 0)
				  + (((mask & XPG_MODIFIER) != 0
				      || (mask & CEN_AUDIENCE) != 0)
				     ? strlen (modifier) + 1 : 0)
				  + ((mask & CEN_SPECIAL) != 0
				     ? strlen (special) + 1 : 0)
				  + (((mask & CEN_SPONSOR) != 0
				      || (mask & CEN_REVISION) != 0)
				     ? (1 + ((mask & CEN_SPONSOR) != 0
					     ? strlen (sponsor) + 1 : 0)
					+ ((mask & CEN_REVISION) != 0
					   ? strlen (revision) + 1 : 0)) : 0)
				  + 1 + strlen (filename) + 1);

  if (abs_filename == NULL)
    return NULL;

  retval = NULL;
  last = NULL;

  /* Construct file name.  */
  memcpy (abs_filename, dirlist, dirlist_len);
  __argz_stringify (abs_filename, dirlist_len, ':');
  cp = abs_filename + (dirlist_len - 1);
  *cp++ = '/';
  cp = stpcpy (cp, language);

  if ((mask & TERRITORY) != 0)
    {
      *cp++ = '_';
      cp = stpcpy (cp, territory);
    }
  if ((mask & XPG_CODESET) != 0)
    {
      *cp++ = '.';
      cp = stpcpy (cp, codeset);
    }
  if ((mask & XPG_NORM_CODESET) != 0)
    {
      *cp++ = '.';
      cp = stpcpy (cp, normalized_codeset);
    }
  if ((mask & (XPG_MODIFIER | CEN_AUDIENCE)) != 0)
    {
      /* This component can be part of both syntaces but has different
	 leading characters.  For CEN we use `+', else `@'.  */
      *cp++ = (mask & CEN_AUDIENCE) != 0 ? '+' : '@';
      cp = stpcpy (cp, modifier);
    }
  if ((mask & CEN_SPECIAL) != 0)
    {
      *cp++ = '+';
      cp = stpcpy (cp, special);
    }
  if ((mask & (CEN_SPONSOR | CEN_REVISION)) != 0)
    {
      *cp++ = ',';
      if ((mask & CEN_SPONSOR) != 0)
	cp = stpcpy (cp, sponsor);
      if ((mask & CEN_REVISION) != 0)
	{
	  *cp++ = '_';
	  cp = stpcpy (cp, revision);
	}
    }

  *cp++ = '/';
  stpcpy (cp, filename);

  /* Look in list of already loaded domains whether it is already
     available.  */
  last = NULL;
  for (retval = *l10nfile_list; retval != NULL; retval = retval->next)
    if (retval->filename != NULL)
      {
	int compare = strcmp (retval->filename, abs_filename);
	if (compare == 0)
	  /* We found it!  */
	  break;
	if (compare < 0)
	  {
	    /* It's not in the list.  */
	    retval = NULL;
	    break;
	  }

	last = retval;
      }

  if (retval != NULL || do_allocate == 0)
    {
      free (abs_filename);
      return retval;
    }

  retval = (struct loaded_l10nfile *)
    malloc (sizeof (*retval) + (__argz_count (dirlist, dirlist_len)
				* (1 << pop (mask))
				* sizeof (struct loaded_l10nfile *)));
  if (retval == NULL)
    return NULL;

  retval->filename = abs_filename;
  retval->decided = (__argz_count (dirlist, dirlist_len) != 1
		     || ((mask & XPG_CODESET) != 0
			 && (mask & XPG_NORM_CODESET) != 0));
  retval->data = NULL;

  if (last == NULL)
    {
      retval->next = *l10nfile_list;
      *l10nfile_list = retval;
    }
  else
    {
      retval->next = last->next;
      last->next = retval;
    }

  entries = 0;
  /* If the DIRLIST is a real list the RETVAL entry corresponds not to
     a real file.  So we have to use the DIRLIST separation mechanism
     of the inner loop.  */
  cnt = __argz_count (dirlist, dirlist_len) == 1 ? mask - 1 : mask;
  for (; cnt >= 0; --cnt)
    if ((cnt & ~mask) == 0
	&& ((cnt & CEN_SPECIFIC) == 0 || (cnt & XPG_SPECIFIC) == 0)
	&& ((cnt & XPG_CODESET) == 0 || (cnt & XPG_NORM_CODESET) == 0))
      {
	/* Iterate over all elements of the DIRLIST.  */
	char *dir = NULL;

	while ((dir = __argz_next ((char *) dirlist, dirlist_len, dir))
	       != NULL)
	  retval->successor[entries++]
	    = _nl_make_l10nflist (l10nfile_list, dir, strlen (dir) + 1, cnt,
				  language, territory, codeset,
				  normalized_codeset, modifier, special,
				  sponsor, revision, filename, 1);
      }
  retval->successor[entries] = NULL;

  return retval;
}

/* Normalize codeset name.  There is no standard for the codeset
   names.  Normalization allows the user to use any of the common
   names.  */
static const char *
_nl_normalize_codeset (codeset, name_len)
     const unsigned char *codeset;
     size_t name_len;
{
  int len = 0;
  int only_digit = 1;
  char *retval;
  char *wp;
  size_t cnt;

  for (cnt = 0; cnt < name_len; ++cnt)
    if (isalnum (codeset[cnt]))
      {
	++len;

	if (isalpha (codeset[cnt]))
	  only_digit = 0;
      }

  retval = (char *) malloc ((only_digit ? 3 : 0) + len + 1);

  if (retval != NULL)
    {
      if (only_digit)
	wp = stpcpy (retval, "iso");
      else
	wp = retval;

      for (cnt = 0; cnt < name_len; ++cnt)
	if (isalpha (codeset[cnt]))
	  *wp++ = tolower (codeset[cnt]);
	else if (isdigit (codeset[cnt]))
	  *wp++ = codeset[cnt];

      *wp = '\0';
    }

  return (const char *) retval;
}

/* End of l10nflist.c */

/* Begin of explodename.c */

/* Copyright (C) 1995, 1996, 1997, 1998 Free Software Foundation, Inc.
   Contributed by Ulrich Drepper <drepper@gnu.ai.mit.edu>, 1995.  */

#if defined STDC_HEADERS || defined _LIBC
#endif

#if defined HAVE_STRING_H || defined _LIBC
#else
#endif

static int
_nl_explode_name (name, language, modifier, territory, codeset,
		  normalized_codeset, special, sponsor, revision)
     char *name;
     const char **language;
     const char **modifier;
     const char **territory;
     const char **codeset;
     const char **normalized_codeset;
     const char **special;
     const char **sponsor;
     const char **revision;
{
  enum { undecided, xpg, cen } syntax;
  char *cp;
  int mask;

  *modifier = NULL;
  *territory = NULL;
  *codeset = NULL;
  *normalized_codeset = NULL;
  *special = NULL;
  *sponsor = NULL;
  *revision = NULL;

  /* Now we determine the single parts of the locale name.  First
     look for the language.  Termination symbols are `_' and `@' if
     we use XPG4 style, and `_', `+', and `,' if we use CEN syntax.  */
  mask = 0;
  syntax = undecided;
  *language = cp = name;
  while (cp[0] != '\0' && cp[0] != '_' && cp[0] != '@'
	 && cp[0] != '+' && cp[0] != ',')
    ++cp;

  if (*language == cp)
    /* This does not make sense: language has to be specified.  Use
       this entry as it is without exploding.  Perhaps it is an alias.  */
    cp = strchr (*language, '\0');
  else if (cp[0] == '_')
    {
      /* Next is the territory.  */
      cp[0] = '\0';
      *territory = ++cp;

      while (cp[0] != '\0' && cp[0] != '.' && cp[0] != '@'
	     && cp[0] != '+' && cp[0] != ',' && cp[0] != '_')
	++cp;

      mask |= TERRITORY;

      if (cp[0] == '.')
	{
	  /* Next is the codeset.  */
	  syntax = xpg;
	  cp[0] = '\0';
	  *codeset = ++cp;

	  while (cp[0] != '\0' && cp[0] != '@')
	    ++cp;

	  mask |= XPG_CODESET;

	  if (*codeset != cp && (*codeset)[0] != '\0')
	    {
	      *normalized_codeset = _nl_normalize_codeset ((const unsigned
                                                            char *)*codeset,
							   cp - *codeset);
	      if (strcmp (*codeset, *normalized_codeset) == 0)
		free ((char *) *normalized_codeset);
	      else
		mask |= XPG_NORM_CODESET;
	    }
	}
    }

  if (cp[0] == '@' || (syntax != xpg && cp[0] == '+'))
    {
      /* Next is the modifier.  */
      syntax = cp[0] == '@' ? xpg : cen;
      cp[0] = '\0';
      *modifier = ++cp;

      while (syntax == cen && cp[0] != '\0' && cp[0] != '+'
	     && cp[0] != ',' && cp[0] != '_')
	++cp;

      mask |= XPG_MODIFIER | CEN_AUDIENCE;
    }

  if (syntax != xpg && (cp[0] == '+' || cp[0] == ',' || cp[0] == '_'))
    {
      syntax = cen;

      if (cp[0] == '+')
	{
 	  /* Next is special application (CEN syntax).  */
	  cp[0] = '\0';
	  *special = ++cp;

	  while (cp[0] != '\0' && cp[0] != ',' && cp[0] != '_')
	    ++cp;

	  mask |= CEN_SPECIAL;
	}

      if (cp[0] == ',')
	{
 	  /* Next is sponsor (CEN syntax).  */
	  cp[0] = '\0';
	  *sponsor = ++cp;

	  while (cp[0] != '\0' && cp[0] != '_')
	    ++cp;

	  mask |= CEN_SPONSOR;
	}

      if (cp[0] == '_')
	{
 	  /* Next is revision (CEN syntax).  */
	  cp[0] = '\0';
	  *revision = ++cp;

	  mask |= CEN_REVISION;
	}
    }

  /* For CEN syntax values it might be important to have the
     separator character in the file name, not for XPG syntax.  */
  if (syntax == xpg)
    {
      if (*territory != NULL && (*territory)[0] == '\0')
	mask &= ~TERRITORY;

      if (*codeset != NULL && (*codeset)[0] == '\0')
	mask &= ~XPG_CODESET;

      if (*modifier != NULL && (*modifier)[0] == '\0')
	mask &= ~XPG_MODIFIER;
    }

  return mask;
}

/* End of explodename.c */

/* Begin of loadmsgcat.c */

/* Load needed message catalogs.
   Copyright (C) 1995, 1996, 1997, 1998 Free Software Foundation, Inc.  */

#include <fcntl.h>
#include <sys/stat.h>

#if defined STDC_HEADERS || defined _LIBC
#endif

#if defined HAVE_UNISTD_H || defined _LIBC
# include <unistd.h>
#endif

#if (defined HAVE_MMAP && defined HAVE_MUNMAP) || defined _LIBC
# include <sys/mman.h>
#endif

/* Interrupt of loadmsgcat.c */

/* Begin of gettext.h */

/* Internal header for GNU gettext internationalization functions.
   Copyright (C) 1995, 1997 Free Software Foundation, Inc.  */

#include <stdio.h>

#if HAVE_LIMITS_H || _LIBC
# include <limits.h>
#endif

/* The magic number of the GNU message catalog format.  */
#define _MAGIC 0x950412de
#define _MAGIC_SWAPPED 0xde120495

/* Revision number of the currently used .mo (binary) file format.  */
#define MO_REVISION_NUMBER 0

/* The following contortions are an attempt to use the C preprocessor
   to determine an unsigned integral type that is 32 bits wide.  An
   alternative approach is to use autoconf's AC_CHECK_SIZEOF macro, but
   doing that would require that the configure script compile and *run*
   the resulting executable.  Locally running cross-compiled executables
   is usually not possible.  */

#if __STDC__
# define UINT_MAX_32_BITS 4294967295U
#else
# define UINT_MAX_32_BITS 0xFFFFFFFF
#endif

/* If UINT_MAX isn't defined, assume it's a 32-bit type.
   This should be valid for all systems GNU cares about because
   that doesn't include 16-bit systems, and only modern systems
   (that certainly have <limits.h>) have 64+-bit integral types.  */

#ifndef UINT_MAX
# define UINT_MAX UINT_MAX_32_BITS
#endif

#if UINT_MAX == UINT_MAX_32_BITS
typedef unsigned nls_uint32;
#else
# if USHRT_MAX == UINT_MAX_32_BITS
typedef unsigned short nls_uint32;
# else
#  if ULONG_MAX == UINT_MAX_32_BITS
typedef unsigned long nls_uint32;
#  else
  /* The following line is intended to throw an error.  Using #error is
     not portable enough.  */
  "Cannot determine unsigned 32-bit data type."
#  endif
# endif
#endif

/* Header for binary .mo file format.  */
struct mo_file_header
{
  /* The magic number.  */
  nls_uint32 magic;
  /* The revision number of the file format.  */
  nls_uint32 revision;
  /* The number of strings pairs.  */
  nls_uint32 nstrings;
  /* Offset of table with start offsets of original strings.  */
  nls_uint32 orig_tab_offset;
  /* Offset of table with start offsets of translation strings.  */
  nls_uint32 trans_tab_offset;
  /* Size of hashing table.  */
  nls_uint32 hash_tab_size;
  /* Offset of first hashing entry.  */
  nls_uint32 hash_tab_offset;
};

struct string_desc
{
  /* Length of addressed string.  */
  nls_uint32 length;
  /* Offset of string in file.  */
  nls_uint32 offset;
};

/* End of gettext.h */

/* Resume of loadmsgcat.c */

/* Interrupt of loadmsgcat.c */

/* Begin of gettextP.h */

/* Header describing internals of gettext library
   Copyright (C) 1995, 1996, 1997, 1998 Free Software Foundation, Inc.
   Written by Ulrich Drepper <drepper@gnu.ai.mit.edu>, 1995.  */

#ifndef _GETTEXTP_H
#define _GETTEXTP_H

#ifndef PARAMS
# if __STDC__
#  define PARAMS(args) args
# else
#  define PARAMS(args) ()
# endif
#endif

#ifndef internal_function
# define internal_function
#endif

#ifndef W
# define W(flag, data) ((flag) ? SWAP (data) : (data))
#endif

#ifdef _LIBC
# include <byteswap.h>
# define SWAP(i) bswap_32 (i)
#else
static nls_uint32 SWAP PARAMS ((nls_uint32 i));

static inline nls_uint32
SWAP (i)
     nls_uint32 i;
{
  return (i << 24) | ((i & 0xff00) << 8) | ((i >> 8) & 0xff00) | (i >> 24);
}
#endif

struct loaded_domain
{
  const char *data;
  int use_mmap;
  size_t mmap_size;
  int must_swap;
  nls_uint32 nstrings;
  struct string_desc *orig_tab;
  struct string_desc *trans_tab;
  nls_uint32 hash_size;
  nls_uint32 *hash_tab;
};

struct binding
{
  struct binding *next;
  char *domainname;
  char *dirname;
};
/*
static struct loaded_l10nfile *_nl_find_domain PARAMS ((const char *__dirname,
						 char *__locale,
						 const char *__domainname))
     internal_function;
static void _nl_load_domain PARAMS ((struct loaded_l10nfile *__domain))
     internal_function;
static void _nl_unload_domain PARAMS ((struct loaded_domain *__domain))
     internal_function;
*/
#endif /* gettextP.h  */

/* End of gettextP.h */

/* Resume of loadmsgcat.c */

#ifdef _LIBC
/* Rename the non ISO C functions.  This is required by the standard
   because some ISO C functions will require linking with this object
   file and the name space must not be polluted.  */
# define open   __open
# define close  __close
# define read   __read
# define mmap   __mmap
# define munmap __munmap
#endif

/* We need a sign, whether a new catalog was loaded, which can be associated
   with all translations.  This is important if the translations are
   cached by one of GCC's features.  */
int _nl_msg_cat_cntr = 0;

/* Load the message catalogs specified by FILENAME.  If it is no valid
   message catalog do nothing.  */
static void
internal_function
_nl_load_domain (domain_file)
     struct loaded_l10nfile *domain_file;
{
  int fd;
  size_t size;
  struct stat st;
  struct mo_file_header *data = (struct mo_file_header *) -1;
#if (defined HAVE_MMAP && defined HAVE_MUNMAP && !defined DISALLOW_MMAP) \
    || defined _LIBC
  int use_mmap = 0;
#endif
  struct loaded_domain *domain;

  domain_file->decided = 1;
  domain_file->data = NULL;

  /* If the record does not represent a valid locale the FILENAME
     might be NULL.  This can happen when according to the given
     specification the locale file name is different for XPG and CEN
     syntax.  */
  if (domain_file->filename == NULL)
    return;

  /* Try to open the addressed file.  */
  fd = open (domain_file->filename, O_RDONLY);
  if (fd == -1)
    return;

  /* We must know about the size of the file.  */
  if (fstat (fd, &st) != 0
      || (size = (size_t) st.st_size) != st.st_size
      || size < sizeof (struct mo_file_header))
    {
      /* Something went wrong.  */
      close (fd);
      return;
    }

#if (defined HAVE_MMAP && defined HAVE_MUNMAP && !defined DISALLOW_MMAP) \
    || defined _LIBC
  /* Now we are ready to load the file.  If mmap() is available we try
     this first.  If not available or it failed we try to load it.  */
  data = (struct mo_file_header *) mmap (NULL, size, PROT_READ,
					 MAP_PRIVATE, fd, 0);

  if (data != (struct mo_file_header *) -1)
    {
      /* mmap() call was successful.  */
      close (fd);
      use_mmap = 1;
    }
#endif

  /* If the data is not yet available (i.e. mmap'ed) we try to load
     it manually.  */
  if (data == (struct mo_file_header *) -1)
    {
      size_t to_read;
      char *read_ptr;

      data = (struct mo_file_header *) malloc (size);
      if (data == NULL)
	return;

      to_read = size;
      read_ptr = (char *) data;
      do
	{
	  long int nb = (long int) read (fd, read_ptr, to_read);
	  if (nb == -1)
	    {
	      close (fd);
	      return;
	    }

	  read_ptr += nb;
	  to_read -= nb;
	}
      while (to_read > 0);

      close (fd);
    }

  /* Using the magic number we can test whether it really is a message
     catalog file.  */
  if (data->magic != _MAGIC && data->magic != _MAGIC_SWAPPED)
    {
      /* The magic number is wrong: not a message catalog file.  */
#if (defined HAVE_MMAP && defined HAVE_MUNMAP && !defined DISALLOW_MMAP) \
    || defined _LIBC
      if (use_mmap)
	munmap ((caddr_t) data, size);
      else
#endif
	free (data);
      return;
    }

  domain_file->data
    = (struct loaded_domain *) malloc (sizeof (struct loaded_domain));
  if (domain_file->data == NULL)
    return;

  domain = (struct loaded_domain *) domain_file->data;
  domain->data = (char *) data;
#if (defined HAVE_MMAP && defined HAVE_MUNMAP && !defined DISALLOW_MMAP) \
    || defined _LIBC
  domain->use_mmap = use_mmap;
#endif
  domain->mmap_size = size;
  domain->must_swap = data->magic != _MAGIC;

  /* Fill in the information about the available tables.  */
  switch (W (domain->must_swap, data->revision))
    {
    case 0:
      domain->nstrings = W (domain->must_swap, data->nstrings);
      domain->orig_tab = (struct string_desc *)
	((char *) data + W (domain->must_swap, data->orig_tab_offset));
      domain->trans_tab = (struct string_desc *)
	((char *) data + W (domain->must_swap, data->trans_tab_offset));
      domain->hash_size = W (domain->must_swap, data->hash_tab_size);
      domain->hash_tab = (nls_uint32 *)
	((char *) data + W (domain->must_swap, data->hash_tab_offset));
      break;
    default:
      /* This is an illegal revision.  */
#if (defined HAVE_MMAP && defined HAVE_MUNMAP && !defined DISALLOW_MMAP) \
    || defined _LIBC
      if (use_mmap)
	munmap ((caddr_t) data, size);
      else
#endif
	free (data);
      free (domain);
      domain_file->data = NULL;
      return;
    }

  /* Show that one domain is changed.  This might make some cached
     translations invalid.  */
  ++_nl_msg_cat_cntr;
}

#ifdef _LIBC
static void
internal_function
_nl_unload_domain (domain)
     struct loaded_domain *domain;
{
  if (domain->use_mmap)
    munmap ((caddr_t) domain->data, domain->mmap_size);
  else
    free ((void *) domain->data);

  free (domain);
}
#endif

/* End of loadmsgcat.c */

/* Begin of localealias.c */

/* Handle aliases for locale names.
   Copyright (C) 1995, 1996, 1997, 1998 Free Software Foundation, Inc.
   Written by Ulrich Drepper <drepper@gnu.ai.mit.edu>, 1995.  */

#if defined STDC_HEADERS || defined _LIBC
#else
char *getenv ();
# ifdef HAVE_MALLOC_H
#  include <malloc.h>
# else
void free ();
# endif
#endif

#if defined HAVE_STRING_H || defined _LIBC
# ifndef _GNU_SOURCE
#  define _GNU_SOURCE	1
# endif
#else
# ifndef memcpy
#  define memcpy(Dst, Src, Num) bcopy (Src, Dst, Num)
# endif
#endif
#if !HAVE_STRCHR && !defined _LIBC
# ifndef strchr
#  define strchr index
# endif
#endif

#ifdef _LIBC
/* Rename the non ANSI C functions.  This is required by the standard
   because some ANSI C functions will require linking with this object
   file and the name space must not be polluted.  */
# define strcasecmp __strcasecmp

# define mempcpy __mempcpy
# define HAVE_MEMPCPY	1

/* We need locking here since we can be called from different places.  */
# include <bits/libc-lock.h>

__libc_lock_define_initialized (static, lock);
#endif

/* For those loosing systems which don't have `alloca' we have to add
   some additional code emulating it.  */
#ifdef HAVE_ALLOCA
/* Nothing has to be done.  */
# define ADD_BLOCK(list, address) /* nothing */
# define FREE_BLOCKS(list) /* nothing */
#else
struct block_list
{
  void *address;
  struct block_list *next;
};
# define ADD_BLOCK(list, addr)						      \
  do {									      \
    struct block_list *newp = (struct block_list *) malloc (sizeof (*newp));  \
    /* If we cannot get a free block we cannot add the new element to	      \
       the list.  */							      \
    if (newp != NULL) {							      \
      newp->address = (addr);						      \
      newp->next = (list);						      \
      (list) = newp;							      \
    }									      \
  } while (0)
# define FREE_BLOCKS(list)						      \
  do {									      \
    while (list != NULL) {						      \
      struct block_list *old = list;					      \
      list = list->next;						      \
      free (old);							      \
    }									      \
  } while (0)
# undef alloca
# define alloca(size) (malloc (size))
#endif	/* have alloca */

struct alias_map
{
  const char *alias;
  const char *value;
};

static char *string_space = NULL;
static size_t string_space_act = 0;
static size_t string_space_max = 0;
static struct alias_map *map;
static size_t nmap = 0;
static size_t maxmap = 0;

/* Prototypes for local functions.  */
static size_t read_alias_file PARAMS ((const char *fname, int fname_len))
     internal_function;
static void extend_alias_table PARAMS ((void));
static int alias_compare PARAMS ((const struct alias_map *map1,
				  const struct alias_map *map2));

static const char *
_nl_expand_alias (name)
    const char *name;
{
  static const char *locale_alias_path = ALIASPATH;
  struct alias_map *retval;
  const char *result = NULL;
  size_t added;

#ifdef _LIBC
  __libc_lock_lock (lock);
#endif

  do
    {
      struct alias_map item;

      item.alias = name;

      if (nmap > 0)
	retval = (struct alias_map *) bsearch (&item, map, nmap,
					       sizeof (struct alias_map),
					       (int (*) PARAMS ((const void *,
								 const void *))
						) alias_compare);
      else
	retval = NULL;

      /* We really found an alias.  Return the value.  */
      if (retval != NULL)
	{
	  result = retval->value;
	  break;
	}

      /* Perhaps we can find another alias file.  */
      added = 0;
      while (added == 0 && locale_alias_path[0] != '\0')
	{
	  const char *start;

	  while (locale_alias_path[0] == ':')
	    ++locale_alias_path;
	  start = locale_alias_path;

	  while (locale_alias_path[0] != '\0' && locale_alias_path[0] != ':')
	    ++locale_alias_path;

	  if (start < locale_alias_path)
	    added = read_alias_file (start, locale_alias_path - start);
	}
    }
  while (added != 0);

#ifdef _LIBC
  __libc_lock_unlock (lock);
#endif

  return result;
}

static size_t
internal_function
read_alias_file (fname, fname_len)
     const char *fname;
     int fname_len;
{
#ifndef HAVE_ALLOCA
  struct block_list *block_list = NULL;
#endif
  FILE *fp;
  char *full_fname;
  size_t added;
  static const char aliasfile[] = "/locale.alias";

  full_fname = (char *) alloca (fname_len + sizeof aliasfile);
  ADD_BLOCK (block_list, full_fname);
#ifdef HAVE_MEMPCPY
  mempcpy (mempcpy (full_fname, fname, fname_len),
	   aliasfile, sizeof aliasfile);
#else
  memcpy (full_fname, fname, fname_len);
  memcpy (&full_fname[fname_len], aliasfile, sizeof aliasfile);
#endif

  fp = fopen (full_fname, "r");
  if (fp == NULL)
    {
      FREE_BLOCKS (block_list);
      return 0;
    }

  added = 0;
  while (!feof (fp))
    {
      /* It is a reasonable approach to use a fix buffer here because
	 a) we are only interested in the first two fields
	 b) these fields must be usable as file names and so must not
	    be that long
       */
      unsigned char buf[BUFSIZ];
      unsigned char *alias;
      unsigned char *value;
      unsigned char *cp;

      if (fgets ((char *)buf, sizeof buf, fp) == NULL)
	/* EOF reached.  */
	break;

      /* Possibly not the whole line fits into the buffer.  Ignore
	 the rest of the line.  */
      if (strchr ((char *)buf, '\n') == NULL)
	{
	  char altbuf[BUFSIZ];
	  do
	    if (fgets (altbuf, sizeof altbuf, fp) == NULL)
	      /* Make sure the inner loop will be left.  The outer loop
		 will exit at the `feof' test.  */
	      break;
	  while (strchr (altbuf, '\n') == NULL);
	}

      cp = buf;
      /* Ignore leading white space.  */
      while (isspace (cp[0]))
	++cp;

      /* A leading '#' signals a comment line.  */
      if (cp[0] != '\0' && cp[0] != '#')
	{
	  alias = cp++;
	  while (cp[0] != '\0' && !isspace (cp[0]))
	    ++cp;
	  /* Terminate alias name.  */
	  if (cp[0] != '\0')
	    *cp++ = '\0';

	  /* Now look for the beginning of the value.  */
	  while (isspace (cp[0]))
	    ++cp;

	  if (cp[0] != '\0')
	    {
	      size_t alias_len;
	      size_t value_len;

	      value = cp++;
	      while (cp[0] != '\0' && !isspace (cp[0]))
		++cp;
	      /* Terminate value.  */
	      if (cp[0] == '\n')
		{
		  /* This has to be done to make the following test
		     for the end of line possible.  We are looking for
		     the terminating '\n' which do not overwrite here.  */
		  *cp++ = '\0';
		  *cp = '\n';
		}
	      else if (cp[0] != '\0')
		*cp++ = '\0';

	      if (nmap >= maxmap)
		extend_alias_table ();

	      alias_len = strlen ((char *)alias) + 1;
	      value_len = strlen ((char *)value) + 1;

	      if (string_space_act + alias_len + value_len > string_space_max)
		{
		  /* Increase size of memory pool.  */
		  size_t new_size = (string_space_max
				     + (alias_len + value_len > 1024
					? alias_len + value_len : 1024));
		  char *new_pool = (char *) realloc (string_space, new_size);
		  if (new_pool == NULL)
		    {
		      FREE_BLOCKS (block_list);
		      return added;
		    }
		  string_space = new_pool;
		  string_space_max = new_size;
		}

	      map[nmap].alias = memcpy (&string_space[string_space_act],
					alias, alias_len);
	      string_space_act += alias_len;

	      map[nmap].value = memcpy (&string_space[string_space_act],
					value, value_len);
	      string_space_act += value_len;

	      ++nmap;
	      ++added;
	    }
	}
    }

  /* Should we test for ferror()?  I think we have to silently ignore
     errors.  --drepper  */
  fclose (fp);

  if (added > 0)
    qsort (map, nmap, sizeof (struct alias_map),
	   (int (*) PARAMS ((const void *, const void *))) alias_compare);

  FREE_BLOCKS (block_list);
  return added;
}

static void
extend_alias_table ()
{
  size_t new_size;
  struct alias_map *new_map;

  new_size = maxmap == 0 ? 100 : 2 * maxmap;
  new_map = (struct alias_map *) realloc (map, (new_size
						* sizeof (struct alias_map)));
  if (new_map == NULL)
    /* Simply don't extend: we don't have any more core.  */
    return;

  map = new_map;
  maxmap = new_size;
}

#ifdef _LIBC
static void __attribute__ ((unused))
free_mem (void)
{
  if (string_space != NULL)
    free (string_space);
  if (map != NULL)
    free (map);
}
text_set_element (__libc_subfreeres, free_mem);
#endif

static int
alias_compare (map1, map2)
     const struct alias_map *map1;
     const struct alias_map *map2;
{
#if defined _LIBC || defined HAVE_STRCASECMP
  return strcasecmp (map1->alias, map2->alias);
#else
  const unsigned char *p1 = (const unsigned char *) map1->alias;
  const unsigned char *p2 = (const unsigned char *) map2->alias;
  unsigned char c1, c2;

  if (p1 == p2)
    return 0;

  do
    {
      /* I know this seems to be odd but the tolower() function in
	 some systems libc cannot handle nonalpha characters.  */
      c1 = isupper (*p1) ? tolower (*p1) : *p1;
      c2 = isupper (*p2) ? tolower (*p2) : *p2;
      if (c1 == '\0')
	break;
      ++p1;
      ++p2;
    }
  while (c1 == c2);

  return c1 - c2;
#endif
}

/* End of localealias.c */

/* Begin of finddomain.c */

/* Handle list of needed message catalogs
   Copyright (C) 1995, 1996, 1997, 1998 Free Software Foundation, Inc.
   Written by Ulrich Drepper <drepper@gnu.ai.mit.edu>, 1995.  */

#include <errno.h>

#if defined STDC_HEADERS || defined _LIBC
#else
# ifdef HAVE_MALLOC_H
# else
void free ();
# endif
#endif

#if defined HAVE_STRING_H || defined _LIBC
#else
# ifndef memcpy
#  define memcpy(Dst, Src, Num) bcopy (Src, Dst, Num)
# endif
#endif
#if !HAVE_STRCHR && !defined _LIBC
# ifndef strchr
#  define strchr index
# endif
#endif

#if defined HAVE_UNISTD_H || defined _LIBC
#endif

#ifdef _LIBC
# include <libintl.h>
#else
# include "gettext.h"
#endif

/* List of already loaded domains.  */
static struct loaded_l10nfile *_nl_loaded_domains;

/* Return a data structure describing the message catalog described by
   the DOMAINNAME and CATEGORY parameters with respect to the currently
   established bindings.  */
static struct loaded_l10nfile *
internal_function
_nl_find_domain (dirname, locale, domainname)
     const char *dirname;
     char *locale;
     const char *domainname;
{
  struct loaded_l10nfile *retval;
  const char *language;
  const char *modifier;
  const char *territory;
  const char *codeset;
  const char *normalized_codeset;
  const char *special;
  const char *sponsor;
  const char *revision;
  const char *alias_value;
  int mask;

  /* LOCALE can consist of up to four recognized parts for the XPG syntax:

		language[_territory[.codeset]][@modifier]

     and six parts for the CEN syntax:

	language[_territory][+audience][+special][,[sponsor][_revision]]

     Beside the first part all of them are allowed to be missing.  If
     the full specified locale is not found, the less specific one are
     looked for.  The various parts will be stripped off according to
     the following order:
		(1) revision
		(2) sponsor
		(3) special
		(4) codeset
		(5) normalized codeset
		(6) territory
		(7) audience/modifier
   */

  /* If we have already tested for this locale entry there has to
     be one data set in the list of loaded domains.  */
  retval = _nl_make_l10nflist (&_nl_loaded_domains, dirname,
			       strlen (dirname) + 1, 0, locale, NULL, NULL,
			       NULL, NULL, NULL, NULL, NULL, domainname, 0);
  if (retval != NULL)
    {
      /* We know something about this locale.  */
      int cnt;

      if (retval->decided == 0)
	_nl_load_domain (retval);

      if (retval->data != NULL)
	return retval;

      for (cnt = 0; retval->successor[cnt] != NULL; ++cnt)
	{
	  if (retval->successor[cnt]->decided == 0)
	    _nl_load_domain (retval->successor[cnt]);

	  if (retval->successor[cnt]->data != NULL)
	    break;
	}
      return cnt >= 0 ? retval : NULL;
      /* NOTREACHED */
    }

  /* See whether the locale value is an alias.  If yes its value
     *overwrites* the alias name.  No test for the original value is
     done.  */
  alias_value = _nl_expand_alias (locale);
  if (alias_value != NULL)
    {
#if defined _LIBC || defined HAVE_STRDUP
      locale = strdup (alias_value);
      if (locale == NULL)
	return NULL;
#else
      size_t len = strlen (alias_value) + 1;
      locale = (char *) malloc (len);
      if (locale == NULL)
	return NULL;

      memcpy (locale, alias_value, len);
#endif
    }

  /* Now we determine the single parts of the locale name.  First
     look for the language.  Termination symbols are `_' and `@' if
     we use XPG4 style, and `_', `+', and `,' if we use CEN syntax.  */
  mask = _nl_explode_name (locale, &language, &modifier, &territory,
			   &codeset, &normalized_codeset, &special,
			   &sponsor, &revision);

  /* Create all possible locale entries which might be interested in
     generalization.  */
  retval = _nl_make_l10nflist (&_nl_loaded_domains, dirname,
			       strlen (dirname) + 1, mask, language, territory,
			       codeset, normalized_codeset, modifier, special,
			       sponsor, revision, domainname, 1);
  if (retval == NULL)
    /* This means we are out of core.  */
    return NULL;

  if (retval->decided == 0)
    _nl_load_domain (retval);
  if (retval->data == NULL)
    {
      int cnt;
      for (cnt = 0; retval->successor[cnt] != NULL; ++cnt)
	{
	  if (retval->successor[cnt]->decided == 0)
	    _nl_load_domain (retval->successor[cnt]);
	  if (retval->successor[cnt]->data != NULL)
	    break;
	}
    }

  /* The room for an alias was dynamically allocated.  Free it now.  */
  if (alias_value != NULL)
    free (locale);

  return retval;
}

#ifdef _LIBC
static void __attribute__ ((unused))
free_mem (void)
{
  struct loaded_l10nfile *runp = _nl_loaded_domains;

  while (runp != NULL)
    {
      struct loaded_l10nfile *here = runp;
      if (runp->data != NULL)
	_nl_unload_domain ((struct loaded_domain *) runp->data);
      runp = runp->next;
      free (here);
    }
}

text_set_element (__libc_subfreeres, free_mem);
#endif

/* End of finddomain.c */

/* Begin of dcgettext.c */

/* Implementation of the dcgettext(3) function.
   Copyright (C) 1995, 1996, 1997, 1998 Free Software Foundation, Inc.  */

#ifndef errno
extern int errno;
#endif
#ifndef __set_errno
# define __set_errno(val) errno = (val)
#endif

#if defined STDC_HEADERS || defined _LIBC
#else
char *getenv ();
# ifdef HAVE_MALLOC_H
# else
void free ();
# endif
#endif

#if defined HAVE_STRING_H || defined _LIBC
# ifndef _GNU_SOURCE
#  define _GNU_SOURCE	1
# endif
#else
#endif
#if !HAVE_STRCHR && !defined _LIBC
# ifndef strchr
#  define strchr index
# endif
#endif

#if defined HAVE_UNISTD_H || defined _LIBC
#endif

#ifdef _LIBC
#else
#endif

/* Interrupt of dcgettext.c */

/* Begin of hash-string.h */

/* Implements a string hashing function.
   Copyright (C) 1995, 1997 Free Software Foundation, Inc.  */

#ifndef PARAMS
# if __STDC__
#  define PARAMS(Args) Args
# else
#  define PARAMS(Args) ()
# endif
#endif

/* We assume to have `unsigned long int' value with at least 32 bits.  */
#define HASHWORDBITS 32

/* Defines the so called `hashpjw' function by P.J. Weinberger
   [see Aho/Sethi/Ullman, COMPILERS: Principles, Techniques and Tools,
   1986, 1987 Bell Telephone Laboratories, Inc.]  */
static unsigned long hash_string PARAMS ((const char *__str_param));

static inline unsigned long
hash_string (str_param)
     const char *str_param;
{
  unsigned long int hval, g;
  const char *str = str_param;

  /* Compute the hash value for the given string.  */
  hval = 0;
  while (*str != '\0')
    {
      hval <<= 4;
      hval += (unsigned long) *str++;
      g = hval & ((unsigned long) 0xf << (HASHWORDBITS - 4));
      if (g != 0)
	{
	  hval ^= g >> (HASHWORDBITS - 8);
	  hval ^= g;
	}
    }
  return hval;
}

/* End of hash-string.h */

/* Resume of dcgettext.c */

#ifdef _LIBC
/* Rename the non ANSI C functions.  This is required by the standard
   because some ANSI C functions will require linking with this object
   file and the name space must not be polluted.  */
# define getcwd __getcwd
# ifndef stpcpy
#  define stpcpy __stpcpy
# endif
#else
# if !defined HAVE_GETCWD
char *getwd ();
#  define getcwd(buf, max) getwd (buf)
# else
char *getcwd ();
# endif
# ifndef HAVE_STPCPY
static char *stpcpy PARAMS ((char *dest, const char *src));
# endif
#endif

/* Amount to increase buffer size by in each try.  */
#define PATH_INCR 32

/* The following is from pathmax.h.  */
/* Non-POSIX BSD systems might have gcc's limits.h, which doesn't define
   PATH_MAX but might cause redefinition warnings when sys/param.h is
   later included (as on MORE/BSD 4.3).  */
#if defined(_POSIX_VERSION) || (defined(HAVE_LIMITS_H) && !defined(__GNUC__))
#endif

#ifndef _POSIX_PATH_MAX
# define _POSIX_PATH_MAX 255
#endif

#if !defined(PATH_MAX) && defined(_PC_PATH_MAX)
# define PATH_MAX (pathconf ("/", _PC_PATH_MAX) < 1 ? 1024 : pathconf ("/", _PC_PATH_MAX))
#endif

/* Don't include sys/param.h if it already has been.  */
#if defined(HAVE_SYS_PARAM_H) && !defined(PATH_MAX) && !defined(MAXPATHLEN)
# include <sys/param.h>
#endif

#if !defined(PATH_MAX) && defined(MAXPATHLEN)
# define PATH_MAX MAXPATHLEN
#endif

#ifndef PATH_MAX
# define PATH_MAX _POSIX_PATH_MAX
#endif

/* XPG3 defines the result of `setlocale (category, NULL)' as:
   ``Directs `setlocale()' to query `category' and return the current
     setting of `local'.''
   However it does not specify the exact format.  And even worse: POSIX
   defines this not at all.  So we can use this feature only on selected
   system (e.g. those using GNU C Library).  */
#ifdef _LIBC
# define HAVE_LOCALE_NULL
#endif

/* Name of the default domain used for gettext(3) prior any call to
   textdomain(3).  The default value for this is "messages".  */
static const char _nl_default_default_domain[] = "messages";

/* Value used as the default domain for gettext(3).  */
static const char *_nl_current_default_domain = _nl_default_default_domain;

/* Contains the default location of the message catalogs.  */
static const char _nl_default_dirname[] = LOCALEDIR;

/* List with bindings of specific domains created by bindtextdomain()
   calls.  */
static struct binding *_nl_domain_bindings;

/* Prototypes for local functions.  */
static char *find_msg PARAMS ((struct loaded_l10nfile *domain_file,
			       const char *msgid)) internal_function;
static const char *category_to_name PARAMS ((int category)) internal_function;
static const char *guess_category_value PARAMS ((int category,
						 const char *categoryname))
     internal_function;

/* Names for the libintl functions are a problem.  They must not clash
   with existing names and they should follow ANSI C.  But this source
   code is also used in GNU C Library where the names have a __
   prefix.  So we have to make a difference here.  */
#ifdef _LIBC
# define DCGETTEXT __dcgettext
#else
# define DCGETTEXT dcgettext__
#endif

/* Look up MSGID in the DOMAINNAME message catalog for the current CATEGORY
   locale.  */
char *
DCGETTEXT (domainname, msgid, category)
     const char *domainname;
     const char *msgid;
     int category;
{
#ifndef HAVE_ALLOCA
  struct block_list *block_list = NULL;
#endif
  struct loaded_l10nfile *domain;
  struct binding *binding;
  const char *categoryname;
  const char *categoryvalue;
  char *dirname, *xdomainname;
  char *single_locale;
  char *retval;
  int saved_errno = errno;

  /* If no real MSGID is given return NULL.  */
  if (msgid == NULL)
    return NULL;

  /* If DOMAINNAME is NULL, we are interested in the default domain.  If
     CATEGORY is not LC_MESSAGES this might not make much sense but the
     defintion left this undefined.  */
  if (domainname == NULL)
    domainname = _nl_current_default_domain;

  /* First find matching binding.  */
  for (binding = _nl_domain_bindings; binding != NULL; binding = binding->next)
    {
      int compare = strcmp (domainname, binding->domainname);
      if (compare == 0)
	/* We found it!  */
	break;
      if (compare < 0)
	{
	  /* It is not in the list.  */
	  binding = NULL;
	  break;
	}
    }

  if (binding == NULL)
    dirname = (char *) _nl_default_dirname;
  else if (binding->dirname[0] == '/')
    dirname = binding->dirname;
  else
    {
      /* We have a relative path.  Make it absolute now.  */
      size_t dirname_len = strlen (binding->dirname) + 1;
      size_t path_max;
      char *ret;

      path_max = (unsigned) PATH_MAX;
      path_max += 2;		/* The getcwd docs say to do this.  */

      dirname = (char *) alloca (path_max + dirname_len);
      ADD_BLOCK (block_list, dirname);

      __set_errno (0);
      while ((ret = getcwd (dirname, path_max)) == NULL && errno == ERANGE)
	{
	  path_max += PATH_INCR;
	  dirname = (char *) alloca (path_max + dirname_len);
	  ADD_BLOCK (block_list, dirname);
	  __set_errno (0);
	}

      if (ret == NULL)
	{
	  /* We cannot get the current working directory.  Don't signal an
	     error but simply return the default string.  */
	  FREE_BLOCKS (block_list);
	  __set_errno (saved_errno);
	  return (char *) msgid;
	}

      stpcpy (stpcpy (strchr (dirname, '\0'), "/"), binding->dirname);
    }

  /* Now determine the symbolic name of CATEGORY and its value.  */
  categoryname = category_to_name (category);
  categoryvalue = guess_category_value (category, categoryname);

  xdomainname = (char *) alloca (strlen (categoryname)
				 + strlen (domainname) + 5);
  ADD_BLOCK (block_list, xdomainname);

  stpcpy (stpcpy (stpcpy (stpcpy (xdomainname, categoryname), "/"),
		  domainname),
	  ".mo");

  /* Creating working area.  */
  single_locale = (char *) alloca (strlen (categoryvalue) + 1);
  ADD_BLOCK (block_list, single_locale);

  /* Search for the given string.  This is a loop because we perhaps
     got an ordered list of languages to consider for th translation.  */
  while (1)
    {
      /* Make CATEGORYVALUE point to the next element of the list.  */
      while (categoryvalue[0] != '\0' && categoryvalue[0] == ':')
	++categoryvalue;
      if (categoryvalue[0] == '\0')
	{
	  /* The whole contents of CATEGORYVALUE has been searched but
	     no valid entry has been found.  We solve this situation
	     by implicitly appending a "C" entry, i.e. no translation
	     will take place.  */
	  single_locale[0] = 'C';
	  single_locale[1] = '\0';
	}
      else
	{
	  char *cp = single_locale;
	  while (categoryvalue[0] != '\0' && categoryvalue[0] != ':')
	    *cp++ = *categoryvalue++;
	  *cp = '\0';
	}

      /* If the current locale value is C (or POSIX) we don't load a
	 domain.  Return the MSGID.  */
      if (strcmp (single_locale, "C") == 0
	  || strcmp (single_locale, "POSIX") == 0)
	{
	  FREE_BLOCKS (block_list);
	  __set_errno (saved_errno);
	  return (char *) msgid;
	}

      /* Find structure describing the message catalog matching the
	 DOMAINNAME and CATEGORY.  */
      domain = _nl_find_domain (dirname, single_locale, xdomainname);

      if (domain != NULL)
	{
	  retval = find_msg (domain, msgid);

	  if (retval == NULL)
	    {
	      int cnt;

	      for (cnt = 0; domain->successor[cnt] != NULL; ++cnt)
		{
		  retval = find_msg (domain->successor[cnt], msgid);

		  if (retval != NULL)
		    break;
		}
	    }

	  if (retval != NULL)
	    {
	      FREE_BLOCKS (block_list);
	      __set_errno (saved_errno);
	      return retval;
	    }
	}
    }
  /* NOTREACHED */
}

#ifdef _LIBC
/* Alias for function name in GNU C Library.  */
#endif

static char *
internal_function
find_msg (domain_file, msgid)
     struct loaded_l10nfile *domain_file;
     const char *msgid;
{
  size_t top, act, bottom;
  struct loaded_domain *domain;

  if (domain_file->decided == 0)
    _nl_load_domain (domain_file);

  if (domain_file->data == NULL)
    return NULL;

  domain = (struct loaded_domain *) domain_file->data;

  /* Locate the MSGID and its translation.  */
  if (domain->hash_size > 2 && domain->hash_tab != NULL)
    {
      /* Use the hashing table.  */
      nls_uint32 len = strlen (msgid);
      nls_uint32 hash_val = hash_string (msgid);
      nls_uint32 idx = hash_val % domain->hash_size;
      nls_uint32 incr = 1 + (hash_val % (domain->hash_size - 2));
      nls_uint32 nstr = W (domain->must_swap, domain->hash_tab[idx]);

      if (nstr == 0)
	/* Hash table entry is empty.  */
	return NULL;

      if (W (domain->must_swap, domain->orig_tab[nstr - 1].length) == len
	  && strcmp (msgid,
		     domain->data + W (domain->must_swap,
				       domain->orig_tab[nstr - 1].offset)) == 0)
	return (char *) domain->data + W (domain->must_swap,
					  domain->trans_tab[nstr - 1].offset);

      while (1)
	{
	  if (idx >= domain->hash_size - incr)
	    idx -= domain->hash_size - incr;
	  else
	    idx += incr;

	  nstr = W (domain->must_swap, domain->hash_tab[idx]);
	  if (nstr == 0)
	    /* Hash table entry is empty.  */
	    return NULL;

	  if (W (domain->must_swap, domain->orig_tab[nstr - 1].length) == len
	      && strcmp (msgid,
			 domain->data + W (domain->must_swap,
					   domain->orig_tab[nstr - 1].offset))
	         == 0)
	    return (char *) domain->data
	      + W (domain->must_swap, domain->trans_tab[nstr - 1].offset);
	}
      /* NOTREACHED */
    }

  /* Now we try the default method:  binary search in the sorted
     array of messages.  */
  bottom = 0;
  top = domain->nstrings;
  while (bottom < top)
    {
      int cmp_val;

      act = (bottom + top) / 2;
      cmp_val = strcmp (msgid, domain->data
			       + W (domain->must_swap,
				    domain->orig_tab[act].offset));
      if (cmp_val < 0)
	top = act;
      else if (cmp_val > 0)
	bottom = act + 1;
      else
	break;
    }

  /* If an translation is found return this.  */
  return bottom >= top ? NULL : (char *) domain->data
                                + W (domain->must_swap,
				     domain->trans_tab[act].offset);
}

/* Return string representation of locale CATEGORY.  */
static const char *
internal_function
category_to_name (category)
     int category;
{
  const char *retval;

  switch (category)
  {
#ifdef LC_COLLATE
  case LC_COLLATE:
    retval = "LC_COLLATE";
    break;
#endif
#ifdef LC_CTYPE
  case LC_CTYPE:
    retval = "LC_CTYPE";
    break;
#endif
#ifdef LC_MONETARY
  case LC_MONETARY:
    retval = "LC_MONETARY";
    break;
#endif
#ifdef LC_NUMERIC
  case LC_NUMERIC:
    retval = "LC_NUMERIC";
    break;
#endif
#ifdef LC_TIME
  case LC_TIME:
    retval = "LC_TIME";
    break;
#endif
#ifdef LC_MESSAGES
  case LC_MESSAGES:
    retval = "LC_MESSAGES";
    break;
#endif
#ifdef LC_RESPONSE
  case LC_RESPONSE:
    retval = "LC_RESPONSE";
    break;
#endif
#ifdef LC_ALL
  case LC_ALL:
    /* This might not make sense but is perhaps better than any other
       value.  */
    retval = "LC_ALL";
    break;
#endif
  default:
    /* If you have a better idea for a default value let me know.  */
    retval = "LC_XXX";
  }

  return retval;
}

/* Guess value of current locale from value of the environment variables.  */
static const char *
internal_function
guess_category_value (category, categoryname)
     int category;
     const char *categoryname;
{
  const char *retval;

  /* The highest priority value is the `LANGUAGE' environment
     variable.  This is a GNU extension.  */
  retval = getenv ("LANGUAGE");
  if (retval != NULL && retval[0] != '\0')
    return retval;

  /* `LANGUAGE' is not set.  So we have to proceed with the POSIX
     methods of looking to `LC_ALL', `LC_xxx', and `LANG'.  On some
     systems this can be done by the `setlocale' function itself.  */
#if defined HAVE_SETLOCALE && defined HAVE_LC_MESSAGES && defined HAVE_LOCALE_NULL
  return setlocale (category, NULL);
#else
  /* Setting of LC_ALL overwrites all other.  */
  retval = getenv ("LC_ALL");
  if (retval != NULL && retval[0] != '\0')
    return retval;

  /* Next comes the name of the desired category.  */
  retval = getenv (categoryname);
  if (retval != NULL && retval[0] != '\0')
    return retval;

  /* Last possibility is the LANG environment variable.  */
  retval = getenv ("LANG");
  if (retval != NULL && retval[0] != '\0')
    return retval;

  /* We use C as the default domain.  POSIX says this is implementation
     defined.  */
  return "C";
#endif
}

/* We don't want libintl.a to depend on any other library.  So we
   avoid the non-standard function stpcpy.  In GNU C Library this
   function is available, though.  Also allow the symbol HAVE_STPCPY
   to be defined.  */
#if !_LIBC && !HAVE_STPCPY
static char *
stpcpy (dest, src)
     char *dest;
     const char *src;
{
  while ((*dest++ = *src++) != '\0')
    /* Do nothing. */ ;
  return dest - 1;
}
#endif

#ifdef _LIBC
/* If we want to free all resources we have to do some work at
   program's end.  */
static void __attribute__ ((unused))
free_mem (void)
{
  struct binding *runp;

  for (runp = _nl_domain_bindings; runp != NULL; runp = runp->next)
    {
      free (runp->domainname);
      if (runp->dirname != _nl_default_dirname)
	/* Yes, this is a pointer comparison.  */
	free (runp->dirname);
    }

  if (_nl_current_default_domain != _nl_default_default_domain)
    /* Yes, again a pointer comparison.  */
    free ((char *) _nl_current_default_domain);
}

text_set_element (__libc_subfreeres, free_mem);
#endif

/* End of dcgettext.c */

/* Begin of bindtextdom.c */

/* Implementation of the bindtextdomain(3) function
   Copyright (C) 1995, 1996, 1997, 1998 Free Software Foundation, Inc.  */

#if defined STDC_HEADERS || defined _LIBC
#else
# ifdef HAVE_MALLOC_H
# else
void free ();
# endif
#endif

#if defined HAVE_STRING_H || defined _LIBC
#else
# ifndef memcpy
#  define memcpy(Dst, Src, Num) bcopy (Src, Dst, Num)
# endif
#endif

#ifdef _LIBC
#else
#endif

/* Contains the default location of the message catalogs.  */
/* static const char _nl_default_dirname[]; */

/* List with bindings of specific domains.  */
/* static struct binding *_nl_domain_bindings; */

/* Names for the libintl functions are a problem.  They must not clash
   with existing names and they should follow ANSI C.  But this source
   code is also used in GNU C Library where the names have a __
   prefix.  So we have to make a difference here.  */
#ifdef _LIBC
# define BINDTEXTDOMAIN __bindtextdomain
# ifndef strdup
#  define strdup(str) __strdup (str)
# endif
#else
# define BINDTEXTDOMAIN bindtextdomain__
#endif

/* Specify that the DOMAINNAME message catalog will be found
   in DIRNAME rather than in the system locale data base.  */
static char *
BINDTEXTDOMAIN (domainname, dirname)
     const char *domainname;
     const char *dirname;
{
  struct binding *binding;

  /* Some sanity checks.  */
  if (domainname == NULL || domainname[0] == '\0')
    return NULL;

  for (binding = _nl_domain_bindings; binding != NULL; binding = binding->next)
    {
      int compare = strcmp (domainname, binding->domainname);
      if (compare == 0)
	/* We found it!  */
	break;
      if (compare < 0)
	{
	  /* It is not in the list.  */
	  binding = NULL;
	  break;
	}
    }

  if (dirname == NULL)
    /* The current binding has be to returned.  */
    return binding == NULL ? (char *) _nl_default_dirname : binding->dirname;

  if (binding != NULL)
    {
      /* The domain is already bound.  If the new value and the old
	 one are equal we simply do nothing.  Otherwise replace the
	 old binding.  */
      if (strcmp (dirname, binding->dirname) != 0)
	{
	  char *new_dirname;

	  if (strcmp (dirname, _nl_default_dirname) == 0)
	    new_dirname = (char *) _nl_default_dirname;
	  else
	    {
#if defined _LIBC || defined HAVE_STRDUP
	      new_dirname = strdup (dirname);
	      if (new_dirname == NULL)
		return NULL;
#else
	      size_t len = strlen (dirname) + 1;
	      new_dirname = (char *) malloc (len);
	      if (new_dirname == NULL)
		return NULL;

	      memcpy (new_dirname, dirname, len);
#endif
	    }

	  if (binding->dirname != _nl_default_dirname)
	    free (binding->dirname);

	  binding->dirname = new_dirname;
	}
    }
  else
    {
      /* We have to create a new binding.  */
#if !defined _LIBC && !defined HAVE_STRDUP
      size_t len;
#endif
      struct binding *new_binding =
	(struct binding *) malloc (sizeof (*new_binding));

      if (new_binding == NULL)
	return NULL;

#if defined _LIBC || defined HAVE_STRDUP
      new_binding->domainname = strdup (domainname);
      if (new_binding->domainname == NULL)
	return NULL;
#else
      len = strlen (domainname) + 1;
      new_binding->domainname = (char *) malloc (len);
      if (new_binding->domainname == NULL)
	return NULL;
      memcpy (new_binding->domainname, domainname, len);
#endif

      if (strcmp (dirname, _nl_default_dirname) == 0)
	new_binding->dirname = (char *) _nl_default_dirname;
      else
	{
#if defined _LIBC || defined HAVE_STRDUP
	  new_binding->dirname = strdup (dirname);
	  if (new_binding->dirname == NULL)
	    return NULL;
#else
	  len = strlen (dirname) + 1;
	  new_binding->dirname = (char *) malloc (len);
	  if (new_binding->dirname == NULL)
	    return NULL;
	  memcpy (new_binding->dirname, dirname, len);
#endif
	}

      /* Now enqueue it.  */
      if (_nl_domain_bindings == NULL
	  || strcmp (domainname, _nl_domain_bindings->domainname) < 0)
	{
	  new_binding->next = _nl_domain_bindings;
	  _nl_domain_bindings = new_binding;
	}
      else
	{
	  binding = _nl_domain_bindings;
	  while (binding->next != NULL
		 && strcmp (domainname, binding->next->domainname) > 0)
	    binding = binding->next;

	  new_binding->next = binding->next;
	  binding->next = new_binding;
	}

      binding = new_binding;
    }

  return binding->dirname;
}

#ifdef _LIBC
/* Alias for function name in GNU C Library.  */
#endif

/* End of bindtextdom.c */

/* Begin of dgettext.c */

/* Implementation of the dgettext(3) function
   Copyright (C) 1995, 1996, 1997 Free Software Foundation, Inc.  */

#if defined HAVE_LOCALE_H || defined _LIBC
# include <locale.h>
#endif

#ifdef _LIBC
#else
#endif

/* Names for the libintl functions are a problem.  They must not clash
   with existing names and they should follow ANSI C.  But this source
   code is also used in GNU C Library where the names have a __
   prefix.  So we have to make a difference here.  */
#ifdef _LIBC
# define DGETTEXT __dgettext
# define DCGETTEXT __dcgettext
#else
# define DGETTEXT dgettext__
# define DCGETTEXT dcgettext__
#endif

/* Look up MSGID in the DOMAINNAME message catalog of the current
   LC_MESSAGES locale.  */
static char *
DGETTEXT (domainname, msgid)
     const char *domainname;
     const char *msgid;
{
  return DCGETTEXT (domainname, msgid, LC_MESSAGES);
}

#ifdef _LIBC
/* Alias for function name in GNU C Library.  */
#endif

/* End of dgettext.c */

/* Begin of gettext.c */

/* Implementation of gettext(3) function.
   Copyright (C) 1995, 1997 Free Software Foundation, Inc.  */

#ifdef _LIBC
# define __need_NULL
# include <stddef.h>
#else
# ifdef STDC_HEADERS
#  include <stdlib.h>		/* Just for NULL.  */
# else
#  ifdef HAVE_STRING_H
#  else
#   define NULL ((void *) 0)
#  endif
# endif
#endif

#ifdef _LIBC
#else
#endif

/* Names for the libintl functions are a problem.  They must not clash
   with existing names and they should follow ANSI C.  But this source
   code is also used in GNU C Library where the names have a __
   prefix.  So we have to make a difference here.  */
#ifdef _LIBC
# define GETTEXT __gettext
# define DGETTEXT __dgettext
#else
# define GETTEXT gettext__
# define DGETTEXT dgettext__
#endif

/* Look up MSGID in the current default message catalog for the current
   LC_MESSAGES locale.  If not found, returns MSGID itself (the default
   text).  */
static char *
GETTEXT (msgid)
     const char *msgid;
{
  return DGETTEXT (NULL, msgid);
}

#ifdef _LIBC
/* Alias for function name in GNU C Library.  */
#endif

/* End of gettext.c */

/* Begin of textdomain.c */

/* Implementation of the textdomain(3) function.
   Copyright (C) 1995, 1996, 1997, 1998 Free Software Foundation, Inc.
   Written by Ulrich Drepper <drepper@gnu.ai.mit.edu>, 1995.  */

#if defined STDC_HEADERS || defined _LIBC
#endif

#if defined STDC_HEADERS || defined HAVE_STRING_H || defined _LIBC
#else
# ifndef memcpy
#  define memcpy(Dst, Src, Num) bcopy (Src, Dst, Num)
# endif
#endif

#ifdef _LIBC
#else
#endif

/* Name of the default text domain.  */
/* static const char _nl_default_default_domain[]; */

/* Default text domain in which entries for gettext(3) are to be found.  */
/* static const char *_nl_current_default_domain; */

/* Names for the libintl functions are a problem.  They must not clash
   with existing names and they should follow ANSI C.  But this source
   code is also used in GNU C Library where the names have a __
   prefix.  So we have to make a difference here.  */
#ifdef _LIBC
# define TEXTDOMAIN __textdomain
# ifndef strdup
#  define strdup(str) __strdup (str)
# endif
#else
# define TEXTDOMAIN textdomain__
#endif

/* Set the current default message catalog to DOMAINNAME.
   If DOMAINNAME is null, return the current default.
   If DOMAINNAME is "", reset to the default of "messages".  */
static char *
TEXTDOMAIN (domainname)
     const char *domainname;
{
  char *old;

  /* A NULL pointer requests the current setting.  */
  if (domainname == NULL)
    return (char *) _nl_current_default_domain;

  old = (char *) _nl_current_default_domain;

  /* If domain name is the null string set to default domain "messages".  */
  if (domainname[0] == '\0'
      || strcmp (domainname, _nl_default_default_domain) == 0)
    _nl_current_default_domain = _nl_default_default_domain;
  else
    {
      /* If the following malloc fails `_nl_current_default_domain'
	 will be NULL.  This value will be returned and so signals we
	 are out of core.  */
#if defined _LIBC || defined HAVE_STRDUP
      _nl_current_default_domain = strdup (domainname);
#else
      size_t len = strlen (domainname) + 1;
      char *cp = (char *) malloc (len);
      if (cp != NULL)
	memcpy (cp, domainname, len);
      _nl_current_default_domain = cp;
#endif
    }

  if (old != _nl_default_default_domain)
    free (old);

  return (char *) _nl_current_default_domain;
}

#ifdef _LIBC
/* Alias for function name in GNU C Library.  */
#endif

/* End of textdomain.c */

/* Begin of intl-compat.c */

/* intl-compat.c - Stub functions to call gettext functions from GNU gettext
   Library.
   Copyright (C) 1995 Software Foundation, Inc.  */

#undef gettext
#undef dgettext
#undef dcgettext
#undef textdomain
#undef bindtextdomain

char *
bindtextdomain (domainname, dirname)
     const char *domainname;
     const char *dirname;
{
  return bindtextdomain__ (domainname, dirname);
}

char *
dcgettext (domainname, msgid, category)
     const char *domainname;
     const char *msgid;
     int category;
{
  return dcgettext__ (domainname, msgid, category);
}

char *
dgettext (domainname, msgid)
     const char *domainname;
     const char *msgid;
{
  return dgettext__ (domainname, msgid);
}

char *
gettext (msgid)
     const char *msgid;
{
  return gettext__ (msgid);
}

char *
textdomain (domainname)
     const char *domainname;
{
  return textdomain__ (domainname);
}

/* End of intl-compat.c */
