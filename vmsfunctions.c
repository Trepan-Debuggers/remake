/* vmsfunctions.c */

#define KDEBUG 0

#include <stdio.h>
#include <ctype.h>
#include "make.h"
#ifdef __DECC
#include <starlet.h>
#endif
#include <descrip.h>
#include <rms.h>
#include <iodef.h>
#include <atrdef.h>
#include <fibdef.h>
#include "vmsdir.h"

#if __VMS_VER < 70000000

DIR *
opendir (dspec)
     char *dspec;
{
  static struct FAB *dfab;
  struct NAM *dnam;
  char *searchspec;

  dfab = (struct FAB *) xmalloc (sizeof (struct FAB));
  if (! dfab)
    {
      printf ("Error mallocing for FAB\n");
      return (NULL);
    }

  dnam = (struct NAM *) xmalloc (sizeof (struct NAM));
  if (! dnam)
    {
      printf ("Error mallocing for NAM\n");
      free (dfab);
      return (NULL);
    }

  searchspec = (char *) xmalloc (MAXNAMLEN + 1);
  if (! searchspec)
    {
      printf ("Error mallocing for searchspec\n");
      free (dfab);
      free (dnam);
      return (NULL);
    }

  sprintf (searchspec, "%s*.*;", dspec);

  *dfab = cc$rms_fab;
  dfab->fab$l_fna = searchspec;
  dfab->fab$b_fns = strlen (searchspec);
  dfab->fab$l_nam = dnam;

  *dnam = cc$rms_nam;
  dnam->nam$l_esa = searchspec;
  dnam->nam$b_ess = MAXNAMLEN;

  if (! (sys$parse (dfab) & 1))
    {
      free (dfab);
      free (dnam);
      free (searchspec);
      return (NULL);
    }

  return (dfab);
}

#define uppercasify(str) \
  do \
    { \
      char *tmp; \
      for (tmp = (str); *tmp != '\0'; tmp++) \
        if (islower (*tmp)) \
          *tmp = toupper (*tmp); \
    } \
  while (0)

struct direct *
readdir (dfd)
     DIR * dfd;
{
  static struct direct *dentry;
  static char resultspec[MAXNAMLEN + 1];
  int i;

  dentry = (struct direct *) xmalloc (sizeof (struct direct));
  if (! dentry)
    {
      printf ("Error mallocing for direct\n");
      return (NULL);
    }

  dfd->fab$l_nam->nam$l_rsa = resultspec;
  dfd->fab$l_nam->nam$b_rss = MAXNAMLEN;

  if (debug_flag)
    printf (".");

  if (!((i = sys$search (dfd)) & 1))
    {
      if (debug_flag)
	printf ("sys$search failed with %d\n", i);
      free (dentry);
      return (NULL);
    }

  dentry->d_off = 0;
  if (dfd->fab$l_nam->nam$w_fid == 0)
    dentry->d_fileno = 1;
  else
    dentry->d_fileno = dfd->fab$l_nam->nam$w_fid[0]
      + dfd->fab$l_nam->nam$w_fid[1] << 16;
  dentry->d_reclen = sizeof (struct direct);
#if 0
  if (!strcmp(dfd->fab$l_nam->nam$l_type, ".DIR"))
    dentry->d_namlen = dfd->fab$l_nam->nam$b_name;
  else
#endif
  dentry->d_namlen = dfd->fab$l_nam->nam$b_name + dfd->fab$l_nam->nam$b_type;
  strncpy (dentry->d_name, dfd->fab$l_nam->nam$l_name, dentry->d_namlen);
  dentry->d_name[dentry->d_namlen] = '\0';
  uppercasify (dentry->d_name);
#if 0
  uvUnFixRCSSeparator(dentry->d_name);
#endif

  return (dentry);
}

closedir (dfd)
     DIR *dfd;
{
  if (dfd)
    {
      if (dfd->fab$l_nam)
	free (dfd->fab$l_nam->nam$l_esa);
      free (dfd->fab$l_nam);
      free (dfd);
    }
}
#endif /* compiled for OpenVMS prior to V7.x */

char *
getwd (cwd)
     char *cwd;
{
  static char buf[512];

  if (cwd)
    return (getcwd (cwd, 512));
  else
    return (getcwd (buf, 512));
}

int
vms_stat (name, buf)
     char *name;
     struct stat *buf;
{
  int status;
  int i;

  static struct FAB Fab;
  static struct NAM Nam;
  static struct fibdef Fib;	/* short fib */
  static struct dsc$descriptor FibDesc =
  { sizeof (Fib), DSC$K_DTYPE_Z, DSC$K_CLASS_S, (char *) &Fib };
  static struct dsc$descriptor_s DevDesc =
  { 0, DSC$K_DTYPE_T, DSC$K_CLASS_S, &Nam.nam$t_dvi[1] };
  static char EName[NAM$C_MAXRSS];
  static char RName[NAM$C_MAXRSS];
  static struct dsc$descriptor_s FileName =
  { 0, DSC$K_DTYPE_T, DSC$K_CLASS_S, 0 };
  static struct dsc$descriptor_s string =
  { 0, DSC$K_DTYPE_T, DSC$K_CLASS_S, 0 };
  static unsigned long Rdate[2];
  static unsigned long Cdate[2];
  static struct atrdef Atr[] =
  {
#if defined(VAX)
    /* Revision date */
    { sizeof (Rdate), ATR$C_REVDATE, (unsigned int) &Rdate[0] },
    /* Creation date */
    { sizeof (Cdate), ATR$C_CREDATE, (unsigned int) &Cdate[0] },
#else
    /* Revision date */
    { sizeof (Rdate), ATR$C_REVDATE, &Rdate[0] },
    /* Creation date */
    { sizeof (Cdate), ATR$C_CREDATE, &Cdate[0]},
#endif
    { 0, 0, 0 }
  };
  static short int DevChan;
  static short int iosb[4];

  name = vmsify (name, 0);

  /* initialize RMS structures, we need a NAM to retrieve the FID */
  Fab = cc$rms_fab;
  Fab.fab$l_fna = name;		/* name of file */
  Fab.fab$b_fns = strlen (name);
  Fab.fab$l_nam = &Nam;		/* FAB has an associated NAM */

  Nam = cc$rms_nam;
  Nam.nam$l_esa = EName;	/* expanded filename */
  Nam.nam$b_ess = sizeof (EName);
  Nam.nam$l_rsa = RName;	/* resultant filename */
  Nam.nam$b_rss = sizeof (RName);

  /* do $PARSE and $SEARCH here */
  status = sys$parse (&Fab);
  if (!(status & 1))
    return -1;

  DevDesc.dsc$w_length = Nam.nam$t_dvi[0];
  status = sys$assign (&DevDesc, &DevChan, 0, 0);
  if (!(status & 1))
    return -1;

  FileName.dsc$a_pointer = Nam.nam$l_name;
  FileName.dsc$w_length = Nam.nam$b_name + Nam.nam$b_type + Nam.nam$b_ver;

  /* Initialize the FIB */
  for (i = 0; i < 3; i++)
    {
#if __DECC
      Fib.fib$w_fid[i] = Nam.nam$w_fid[i];
      Fib.fib$w_did[i] = Nam.nam$w_did[i];
#else
      Fib.fib$r_fid_overlay.fib$w_fid[i] = Nam.nam$w_fid[i];
      Fib.fib$r_did_overlay.fib$w_did[i] = Nam.nam$w_did[i];
#endif
    }

  status = sys$qiow (0, DevChan, IO$_ACCESS, &iosb, 0, 0,
		     &FibDesc, &FileName, 0, 0, &Atr, 0);
  sys$dassgn (DevChan);
  if (!(status & 1))
    return -1;
  status = iosb[0];
  if (!(status & 1))
    return -1;

  status = stat (name, buf);
  if (status)
    return -1;

  buf->st_mtime = ((Rdate[0] >> 24) & 0xff) + ((Rdate[1] << 8) & 0xffffff00);
  buf->st_ctime = ((Cdate[0] >> 24) & 0xff) + ((Cdate[1] << 8) & 0xffffff00);

  return 0;
}

char *
cvt_time (tval)
     unsigned long tval;
{
  static long int date[2];
  static char str[27];
  static struct dsc$descriptor date_str =
  { 26, DSC$K_DTYPE_T, DSC$K_CLASS_S, str };

  date[0] = (tval & 0xff) << 24;
  date[1] = ((tval >> 8) & 0xffffff);

  if ((date[0] == 0) && (date[1] == 0))
    return ("never");

  sys$asctim (0, &date_str, date, 0);
  str[26] = '\0';

  return (str);
}
