/* Library function for scanning an archive file.
   Copyright (C) 1987, 89, 91, 92, 93, 94, 95 Free Software Foundation, Inc.

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

#include "make.h"

#ifdef HAVE_FCNTL_H
#include <fcntl.h>
#else
#include <sys/file.h>
#endif

#ifndef	NO_ARCHIVES

/* SCO Unix's compiler defines both of these.  */
#ifdef	M_UNIX
#undef	M_XENIX
#endif

/* On the sun386i and in System V rel 3, ar.h defines two different archive
   formats depending upon whether you have defined PORTAR (normal) or PORT5AR
   (System V Release 1).  There is no default, one or the other must be defined
   to have a nonzero value.  */

#if (!defined (PORTAR) || PORTAR == 0) && (!defined (PORT5AR) || PORT5AR == 0)
#undef	PORTAR
#ifdef M_XENIX
/* According to Jim Sievert <jas1@rsvl.unisys.com>, for SCO XENIX defining
   PORTAR to 1 gets the wrong archive format, and defining it to 0 gets the
   right one.  */
#define PORTAR 0
#else
#define PORTAR 1
#endif
#endif

#include <ar.h>

/* Cray's <ar.h> apparently defines this.  */
#ifndef	AR_HDR_SIZE
#define	AR_HDR_SIZE	(sizeof (struct ar_hdr))
#endif

/* Takes three arguments ARCHIVE, FUNCTION and ARG.

   Open the archive named ARCHIVE, find its members one by one,
   and for each one call FUNCTION with the following arguments:
     archive file descriptor for reading the data,
     member name,
     member name might be truncated flag,
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
#else
  int long_name = 0;
#endif
  char *namemap = 0;
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
    register int nread = read (desc, (char *) &fl_header, FL_HSZ);
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

    if (member_offset == 0)
      {
	/* Empty archive.  */
	close (desc);
	return 0;
      }
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
	char name[256];
	int name_len;
	long int dateval;
	int uidval, gidval;
	long int data_offset;
#else
	char namebuf[sizeof member_header.ar_name + 1];
	char *name;
	int is_namemap;		/* Nonzero if this entry maps long names.  */
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
#define	AR_MEMHDR	(AR_HDR_SIZE - sizeof (member_header._ar_name))
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
	  (*function) (desc, name, 0,
		       member_offset, data_offset, eltsize,
		       dateval, uidval, gidval,
		       eltmode, arg);

#else	/* Not AIAMAG.  */
	nread = read (desc, (char *) &member_header, AR_HDR_SIZE);
	if (nread == 0)
	  /* No data left means end of file; that is OK.  */
	  break;

	if (nread != AR_HDR_SIZE
#ifdef ARFMAG
	    || bcmp (member_header.ar_fmag, ARFMAG, 2)
#endif
	    )
	  {
	    (void) close (desc);
	    return -2;
	  }

	name = namebuf;
	bcopy (member_header.ar_name, name, sizeof member_header.ar_name);
	{
	  register char *p = name + sizeof member_header.ar_name;
	  do
	    *p = '\0';
	  while (p > name && *--p == ' ');

#ifndef AIAMAG
	  /* If the member name is "//" or "ARFILENAMES/" this may be
	     a list of file name mappings.  The maximum file name
 	     length supported by the standard archive format is 14
 	     characters.  This member will actually always be the
 	     first or second entry in the archive, but we don't check
 	     that.  */
 	  is_namemap = (!strcmp (name, "//")
			|| !strcmp (name, "ARFILENAMES/"));
#endif	/* Not AIAMAG. */
	  /* On some systems, there is a slash after each member name.  */
	  if (*p == '/')
	    *p = '\0';

#ifndef AIAMAG
 	  /* If the member name starts with a space or a slash, this
 	     is an index into the file name mappings (used by GNU ar).
 	     Otherwise if the member name looks like #1/NUMBER the
 	     real member name appears in the element data (used by
 	     4.4BSD).  */
 	  if (! is_namemap
 	      && (name[0] == ' ' || name[0] == '/')
 	      && namemap != 0)
	    {
	      name = namemap + atoi (name + 1);
	      long_name = 1;
	    }
 	  else if (name[0] == '#'
 		   && name[1] == '1'
 		   && name[2] == '/')
 	    {
 	      int namesize = atoi (name + 3);
 
 	      name = (char *) alloca (namesize + 1);
 	      nread = read (desc, name, namesize);
 	      if (nread != namesize)
 		{
 		  close (desc);
 		  return -2;
 		}
 	      name[namesize] = '\0';

	      long_name = 1;
 	    }
#endif /* Not AIAMAG. */
	}

#ifndef	M_XENIX
	sscanf (member_header.ar_mode, "%o", &eltmode);
	eltsize = atol (member_header.ar_size);
#else	/* Xenix.  */
	eltmode = (unsigned short int) member_header.ar_mode;
	eltsize = member_header.ar_size;
#endif	/* Not Xenix.  */

	fnval =
	  (*function) (desc, name, ! long_name, member_offset,
		       member_offset + AR_HDR_SIZE, eltsize,
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

#endif  /* AIAMAG.  */

	if (fnval)
	  {
	    (void) close (desc);
	    return fnval;
	  }

#ifdef AIAMAG
	if (member_offset == last_member_offset)
	  /* End of the chain.  */
	  break;

	sscanf (member_header.ar_nxtmem, "%12ld", &member_offset);

	if (lseek (desc, member_offset, 0) != member_offset)
	  {
	    (void) close (desc);
	    return -2;
	  }
#else

 	/* If this member maps archive names, we must read it in.  The
 	   name map will always precede any members whose names must
 	   be mapped.  */
	if (is_namemap)
 	  {
 	    char *clear;
 	    char *limit;

 	    namemap = (char *) alloca (eltsize);
 	    nread = read (desc, namemap, eltsize);
 	    if (nread != eltsize)
 	      {
 		(void) close (desc);
 		return -2;
 	      }
 
 	    /* The names are separated by newlines.  Some formats have
 	       a trailing slash.  Null terminate the strings for
 	       convenience.  */
 	    limit = namemap + eltsize;
 	    for (clear = namemap; clear < limit; clear++)
 	      {
 		if (*clear == '\n')
 		  {
 		    *clear = '\0';
 		    if (clear[-1] == '/')
 		      clear[-1] = '\0';
 		  }
 	      }
 
	    is_namemap = 0;
 	  }

	member_offset += AR_HDR_SIZE + eltsize;
	if (member_offset % 2 != 0)
	  member_offset++;
#endif
      }
  }

  close (desc);
  return 0;
}

/* Return nonzero iff NAME matches MEM.
   If TRUNCATED is nonzero, MEM may be truncated to
   sizeof (struct ar_hdr.ar_name) - 1.  */

int
ar_name_equal (name, mem, truncated)
     char *name, *mem;
     int truncated;
{
  char *p;

  p = rindex (name, '/');
  if (p != 0)
    name = p + 1;

  /* We no longer use this kludge, since we
     now support long archive member names.  */

#if 0 && !defined (AIAMAG) && !defined (APOLLO)

  {
    /* `reallylongname.o' matches `reallylongnam.o'.
       If member names have a trailing slash, that's `reallylongna.o'.  */

    struct ar_hdr h;
    unsigned int max = sizeof (h.ar_name);
    unsigned int namelen, memlen;

    if (strncmp (name, mem, max - 3))
      return 0;

    namelen = strlen (name);
    memlen = strlen (mem);

    if (namelen > memlen && memlen >= max - 1
	&& name[namelen - 2] == '.' && name[namelen - 1] == 'o'
	&& mem[memlen - 2] == '.' && mem[memlen - 1] == 'o')
      return 1;

    if (namelen != memlen)
      return 0;

    return (namelen < max - 3 || !strcmp (name + max - 3, mem + max - 3));
  }

#else	/* AIX or APOLLO.  */

  if (truncated)
    {
#ifdef AIAMAG
      /* TRUNCATED should never be set on this system.  */
      abort ();
#else
      struct ar_hdr hdr;
      return !strncmp (name, mem,
		       sizeof (hdr.ar_name) - 
#if !defined (__hpux) && !defined (cray)
		       1
#else
		       2
#endif /* !__hpux && !cray */
		       );
#endif
    }

  return !strcmp (name, mem);

#endif
}

/* ARGSUSED */
static long int
ar_member_pos (desc, mem, truncated,
	       hdrpos, datapos, size, date, uid, gid, mode, name)
     int desc;
     char *mem;
     int truncated;
     long int hdrpos, datapos, size, date;
     int uid, gid, mode;
     char *name;
{
  if (!ar_name_equal (name, mem, truncated))
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
  if (AR_HDR_SIZE != read (fd, (char *) &ar_hdr, AR_HDR_SIZE))
    goto lose;
  /* Write back the header, thus touching the archive file.  */
  if (lseek (fd, pos, 0) < 0)
    goto lose;
  if (AR_HDR_SIZE != write (fd, (char *) &ar_hdr, AR_HDR_SIZE))
    goto lose;
  /* The file's mtime is the time we we want.  */
#ifdef EINTR
  while (fstat (fd, &statbuf) < 0 && errno == EINTR);
#else
  fstat (fd, &statbuf);
#endif
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
  if (AR_HDR_SIZE != write (fd, (char *) &ar_hdr, AR_HDR_SIZE))
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
describe_member (desc, name, truncated,
		 hdrpos, datapos, size, date, uid, gid, mode)
     int desc;
     char *name;
     int truncated;
     long int hdrpos, datapos, size, date;
     int uid, gid, mode;
{
  extern char *ctime ();

  printf ("Member `%s'%s: %ld bytes at %ld (%ld).\n",
	  name, truncated ? " (name might be truncated)" : "",
	  size, hdrpos, datapos);
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

#endif	/* NO_ARCHIVES.  */
