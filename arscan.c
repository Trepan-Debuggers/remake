/* Library function for scanning an archive file.
   Copyright (C) 1987, 1989, 1991 Free Software Foundation, Inc.

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2, or (at your option)
   any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.  */

/* On the sun386i and in System V rel 3, ar.h defines two different archive
   formats depending upon whether you have defined PORTAR (normal) or PORT5AR
   (System V Release 1).  There is no default, one or the other must be defined
   to have a nonzero value.  */

#if (defined(sun386) || defined(USGr3) || defined(HPUX) \
     && !defined(PORTAR) && !defined(PORT5AR))
#define PORTAR 1
#endif

#include <ar.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>

#ifdef USG
#include <fcntl.h>
#else
#include <sys/file.h>
#endif

#if	(defined (STDC_HEADERS) || defined (__GNU_LIBRARY__) \
	 || defined (POSIX))
#include <stdlib.h>
#include <string.h>
#define	ANSI_STRING
#else	/* No standard headers.  */

#ifdef	USG

#include <string.h>
#include <memory.h>
#define	ANSI_STRING

#else	/* Not USG.  */
#include <strings.h>

#ifndef	bcmp
extern int bcmp ();
#endif
#ifndef	bzero
extern void bzero ();
#endif
#ifndef	bcopy
extern void bcopy ();
#endif

#endif	/* USG.  */

extern char *malloc (), *realloc ();
extern void free ();

#endif	/* Standard headers.  */

#ifdef	ANSI_STRING
#define	index(s, c)	strchr((s), (c))
#define	rindex(s, c)	strrchr((s), (c))

#define bcmp(s1, s2, n)	memcmp ((s1), (s2), (n))
#define bzero(s, n)	memset ((s), 0, (n))
#define bcopy(s, d, n)	memcpy ((d), (s), (n))
#endif	ANSI_STRING
#undef	ANSI_STRING


#ifndef	AIAMAG
#if	(defined(APOLLO) || defined(HPUX) || defined(hpux) || \
	 (PORTAR == 1 && (defined(USGr3) || defined(u3b2) || defined(sun386))))
#define	AR_NAMELEN	14
#define	AR_TRAILING_SLASH	/* Member names have a trailing slash.  */
#else
#define	AR_NAMELEN	15
#endif
#else	/* AIX.  */
#define	AR_NAMELEN	255
#endif

#if	defined(__GNU_LIBRARY__) || defined(POSIX) || defined(_IBMR2)
#include <unistd.h>
#else
extern int read (), open (), close (), write (), fstat ();
extern long int lseek (), atol ();
extern int atoi ();
#endif

/* Takes three arguments ARCHIVE, FUNCTION and ARG.

   Open the archive named ARCHIVE, find its members one by one,
   and for each one call FUNCTION with the following arguments:
     archive file descriptor for reading the data,
     member name,
     member header position in file,
     member data position in file,
     member data size,
     member date,
     member uid,
     member gid,
     member protection mode,
     ARG.

   The descriptor is poised to read the data of the member
   when FUNCTION is called.  It does not matter how much
   data FUNCTION reads.

   If FUNCTION returns nonzero, we immediately return
   what FUNCTION returned.

   Returns -1 if archive does not exist,
   Returns -2 if archive has invalid format.
   Returns 0 if have scanned successfully.  */

long int
ar_scan (archive, function, arg)
     char *archive;
     long int (*function) ();
     long int arg;
{
#ifdef AIAMAG
  FL_HDR fl_header;
#endif
  register int desc = open (archive, O_RDONLY, 0);
  if (desc < 0)
    return -1;
#ifdef SARMAG
  {
    char buf[SARMAG];
    register int nread = read (desc, buf, SARMAG);
    if (nread != SARMAG || bcmp (buf, ARMAG, SARMAG))
      {
	(void) close (desc);
	return -2;
      }
  }
#else
#ifdef AIAMAG
  {
    register int nread = read (desc, &fl_header, FL_HSZ);
    if (nread != FL_HSZ || bcmp (fl_header.fl_magic, AIAMAG, SAIAMAG))
      {
	(void) close (desc);
	return -2;
      }
  }
#else
  {
#ifndef M_XENIX
    int buf;
#else
    unsigned short int buf;
#endif
    register int nread = read(desc, &buf, sizeof (buf));
    if (nread != sizeof (buf) || buf != ARMAG)
      {
	(void) close (desc);
	return -2;
      }
  }
#endif
#endif

  /* Now find the members one by one.  */
  {
#ifdef SARMAG
    register long int member_offset = SARMAG;
#else
#ifdef AIAMAG
    long int member_offset;
    long int last_member_offset;

    sscanf (fl_header.fl_fstmoff, "%12ld", &member_offset);
    sscanf (fl_header.fl_lstmoff, "%12ld", &last_member_offset);
#else
#ifndef	M_XENIX
    register long int member_offset = sizeof (int);
#else	/* Xenix.  */
    register long int member_offset = sizeof (unsigned short int);
#endif	/* Not Xenix.  */
#endif
#endif

    while (1)
      {
	register int nread;
	struct ar_hdr member_header;
#ifdef AIAMAG
	char name[AR_NAMELEN + 1];
	int name_len;
	long int dateval;
	int uidval, gidval;
	long int data_offset;
#else
	char name[sizeof member_header.ar_name + 1];
#endif
	long int eltsize;
	int eltmode;
	long int fnval;

	if (lseek (desc, member_offset, 0) < 0)
	  {
	    (void) close (desc);
	    return -2;
	  }

#ifdef AIAMAG
#define	AR_MEMHDR	\
	(sizeof (member_header) - sizeof (member_header._ar_name))
	nread = read (desc, (char *) &member_header, AR_MEMHDR);

	if (nread != AR_MEMHDR)
	  {
	    (void) close (desc);
	    return -2;
	  }

	sscanf (member_header.ar_namlen, "%4d", &name_len);
	nread = read (desc, name, name_len);

	if (nread != name_len)
	  {
	    (void) close (desc);
	    return -2;
	  }
	
	name[name_len] = 0;

	sscanf (member_header.ar_date, "%12ld", &dateval);
	sscanf (member_header.ar_uid, "%12d", &uidval);
	sscanf (member_header.ar_gid, "%12d", &gidval);
	sscanf (member_header.ar_mode, "%12o", &eltmode);
	sscanf (member_header.ar_size, "%12ld", &eltsize);

	if ((data_offset = member_offset + AR_MEMHDR + name_len + 2) % 2)
	    ++data_offset;

	fnval =
	  (*function) (desc, name, member_offset, data_offset, eltsize,
		       dateval, uidval, gidval,
		       eltmode, arg);

#else
	nread = read (desc, (char *) &member_header, sizeof (struct ar_hdr));
	if (nread == 0)
	  /* No data left means end of file; that is OK.  */
	  break;

	if (nread != sizeof (member_header)
#ifdef ARFMAG
	    || bcmp (member_header.ar_fmag, ARFMAG, 2)
#endif
	    )
	  {
	    (void) close (desc);
	    return -2;
	  }

	bcopy (member_header.ar_name, name, sizeof member_header.ar_name);
	{
	  register char *p = name + sizeof member_header.ar_name;
	  while (p > name && *--p == ' ')
	    *p = '\0';
#ifdef	AR_TRAILING_SLASH
	  if (*p == '/')
	    *p = '\0';
#endif
	}

#ifndef	M_XENIX
	sscanf (member_header.ar_mode, "%o", &eltmode);
	eltsize = atol (member_header.ar_size);
#else	/* Xenix.  */
	eltmode = (unsigned short int) member_header.ar_mode;
	eltsize = member_header.ar_size;
#endif	/* Not Xenix.  */

	fnval =
	  (*function) (desc, name, member_offset,
		       member_offset + sizeof (member_header), eltsize,
#ifndef	M_XENIX
		       atol (member_header.ar_date),
		       atoi (member_header.ar_uid),
		       atoi (member_header.ar_gid),
#else	/* Xenix.  */
		       member_header.ar_date,
		       member_header.ar_uid,
		       member_header.ar_gid,
#endif	/* Not Xenix.  */
		       eltmode, arg);

#endif  /* Not AIAMAG */

	if (fnval)
	  {
	    (void) close (desc);
	    return fnval;
	  }

#ifdef AIAMAG
	if (member_offset == last_member_offset) /* end of chain? */
	    break;

	sscanf (member_header.ar_nxtmem, "%12ld", &member_offset);

	if (lseek (desc, member_offset, 0) != member_offset)
	  {
	    (void) close (desc);
	    return -2;
	  }
#else
	member_offset += sizeof (member_header) + eltsize;
	if (member_offset & 1) member_offset++;
#endif
      }
  }

  close (desc);
  return 0;
}

/* Return nonzero iff NAME matches MEM.  If NAME is longer than
   sizeof (struct ar_hdr.ar_name), MEM may be the truncated version.  */

int
ar_name_equal (name, mem)
     char *name, *mem;
{
  char *p;

  p = rindex (name, '/');
  if (p != 0)
    name = p + 1;

#ifndef	APOLLO

  if (!strncmp (name, mem, AR_NAMELEN))
    return 1;

  if (!strncmp (name, mem, AR_NAMELEN - 2))
    {
      unsigned int namelen, memlen;

      namelen = strlen (name);
      memlen = strlen (mem);

      if (memlen == AR_NAMELEN
	  && mem[AR_NAMELEN - 2] == '.' && mem[AR_NAMELEN - 1] == 'o'
	  && name[namelen - 2] == '.' && name[namelen -1] == 'o')
	return 1;
    }
  return 0;

#else	/* APOLLO.  */
  return !strcmp (name, mem);
#endif
}

/* ARGSUSED */
static long int
ar_member_pos (desc, mem, hdrpos, datapos, size, date, uid, gid, mode, name)
     int desc;
     char *mem;
     long int hdrpos, datapos, size, date;
     int uid, gid, mode;
     char *name;
{
  if (!ar_name_equal (name, mem))
    return 0;
  return hdrpos;
}

/* Set date of member MEMNAME in archive ARNAME to current time.
   Returns 0 if successful,
   -1 if file ARNAME does not exist,
   -2 if not a valid archive,
   -3 if other random system call error (including file read-only),
   1 if valid but member MEMNAME does not exist.  */

int
ar_member_touch (arname, memname)
     char *arname, *memname;
{
  register long int pos = ar_scan (arname, ar_member_pos, (long int) memname);
  register int fd;
  struct ar_hdr ar_hdr;
  register int i;
  extern int errno;
  struct stat statbuf;

  if (pos < 0)
    return (int) pos;
  if (!pos)
    return 1;

  fd = open (arname, O_RDWR, 0666);
  if (fd < 0)
    return -3;
  /* Read in this member's header */
  if (lseek (fd, pos, 0) < 0)
    goto lose;
  if (sizeof ar_hdr != read (fd, (char *) &ar_hdr, sizeof ar_hdr))
    goto lose;
  /* Write back the header, thus touching the archive file.  */
  if (lseek (fd, pos, 0) < 0)
    goto lose;
  if (sizeof ar_hdr != write (fd, (char *) &ar_hdr, sizeof ar_hdr))
    goto lose;
  /* The file's mtime is the time we we want.  */
  fstat (fd, &statbuf);
#if defined(ARFMAG) || defined(AIAMAG)
  /* Advance member's time to that time */
  for (i = 0; i < sizeof ar_hdr.ar_date; i++)
    ar_hdr.ar_date[i] = ' ';
  sprintf (ar_hdr.ar_date, "%ld", (long int) statbuf.st_mtime);
#ifdef AIAMAG
  ar_hdr.ar_date[strlen(ar_hdr.ar_date)] = ' ';
#endif
#else
  ar_hdr.ar_date = statbuf.st_mtime;
#endif
  /* Write back this member's header */
  if (lseek (fd, pos, 0) < 0)
    goto lose;
  if (sizeof ar_hdr != write (fd, (char *) &ar_hdr, sizeof ar_hdr))
    goto lose;
  close (fd);
  return 0;

 lose:
  i = errno;
  close (fd);
  errno = i;
  return -3;
}

#ifdef TEST

long int
describe_member (desc, name, hdrpos, datapos, size, date, uid, gid, mode)
     int desc;
     char *name;
     long int hdrpos, datapos, size, date;
     int uid, gid, mode;
{
  extern char *ctime ();

  printf ("Member %s: %ld bytes at %ld (%ld).\n", name, size, hdrpos, datapos);
  printf ("  Date %s", ctime (&date));
  printf ("  uid = %d, gid = %d, mode = 0%o.\n", uid, gid, mode);

  return 0;
}

main (argc, argv)
     int argc;
     char **argv;
{
  ar_scan (argv[1], describe_member);
  return 0;
}

#endif	/* TEST.  */
