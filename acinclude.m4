dnl acinclude.m4 -- Extra macros needed for GNU make.
dnl
dnl Automake will incorporate this into its generated aclocal.m4.

dnl ---------------------------------------------------------------------------
dnl Got this from the lynx 2.8 distribution.
dnl by T.E.Dickey <dickey@clark.net>
dnl and Jim Spath <jspath@mail.bcpl.lib.md.us>
dnl and Philippe De Muyter <phdm@macqel.be>
dnl
dnl Created: 1997/1/28
dnl Updated: 1997/12/23
dnl ---------------------------------------------------------------------------
dnl After checking for functions in the default $LIBS, make a further check
dnl for the functions that are netlib-related (these aren't always in the
dnl libc, etc., and have to be handled specially because there are conflicting
dnl and broken implementations.
dnl Common library requirements (in order):
dnl	-lresolv -lsocket -lnsl
dnl	-lnsl -lsocket
dnl	-lsocket
dnl	-lbsd
AC_DEFUN([CF_NETLIBS],[
cf_test_netlibs=no
AC_MSG_CHECKING(for network libraries)
AC_CACHE_VAL(cf_cv_netlibs,[
AC_MSG_RESULT(working...)
cf_cv_netlibs=""
cf_test_netlibs=yes
AC_CHECK_FUNCS(gethostname,,[
	CF_RECHECK_FUNC(gethostname,nsl,cf_cv_netlibs,[
		CF_RECHECK_FUNC(gethostname,socket,cf_cv_netlibs)])])
#
# FIXME:  sequent needs this library (i.e., -lsocket -linet -lnsl), but
# I don't know the entrypoints - 97/7/22 TD
AC_CHECK_LIB(inet,main,cf_cv_netlibs="-linet $cf_cv_netlibs")
#
if test "$ac_cv_func_lsocket" != no ; then
AC_CHECK_FUNCS(socket,,[
	CF_RECHECK_FUNC(socket,socket,cf_cv_netlibs,[
		CF_RECHECK_FUNC(socket,bsd,cf_cv_netlibs)])])
fi
#
AC_CHECK_FUNCS(gethostbyname,,[
	CF_RECHECK_FUNC(gethostbyname,nsl,cf_cv_netlibs)])
#
AC_CHECK_FUNCS(strcasecmp,,[
	CF_RECHECK_FUNC(strcasecmp,resolv,cf_cv_netlibs)])
])
LIBS="$LIBS $cf_cv_netlibs"
test $cf_test_netlibs = no && echo "$cf_cv_netlibs" >&AC_FD_MSG
])dnl
dnl ---------------------------------------------------------------------------
dnl Re-check on a function to see if we can pick it up by adding a library.
dnl	$1 = function to check
dnl	$2 = library to check in
dnl	$3 = environment to update (e.g., $LIBS)
dnl	$4 = what to do if this fails
dnl
dnl This uses 'unset' if the shell happens to support it, but leaves the
dnl configuration variable set to 'unknown' if not.  This is a little better
dnl than the normal autoconf test, which gives misleading results if a test
dnl for the function is made (e.g., with AC_CHECK_FUNC) after this macro is
dnl used (autoconf does not distinguish between a null token and one that is
dnl set to 'no').
AC_DEFUN([CF_RECHECK_FUNC],[
AC_CHECK_LIB($2,$1,[
	CF_UPPER(cf_tr_func,$1)
	AC_DEFINE_UNQUOTED(HAVE_$cf_tr_func)
	ac_cv_func_$1=yes
	$3="-l$2 [$]$3"],[
	ac_cv_func_$1=unknown
	unset ac_cv_func_$1 2>/dev/null
	$4],
	[[$]$3])
])dnl
dnl ---------------------------------------------------------------------------
dnl Make an uppercase version of a variable
dnl $1=uppercase($2)
AC_DEFUN([CF_UPPER],
[
changequote(,)dnl
$1=`echo $2 | tr '[a-z]' '[A-Z]'`
changequote([,])dnl
])dnl

dnl ---------------------------------------------------------------------------
dnl Got this from the GNU fileutils 3.16r distribution
dnl by Paul Eggert <eggert@twinsun.com>
dnl ---------------------------------------------------------------------------

dnl The problem is that the default compilation flags in Solaris 2.6 won't
dnl let programs access large files;  you need to tell the compiler that
dnl you actually want your programs to work on large files.  For more
dnl details about this brain damage please see:
dnl http://www.sas.com/standards/large.file/x_open.20Mar96.html

AC_DEFUN(AC_LFS,
[dnl
  # If available, prefer support for large files unless the user specified
  # one of the CPPFLAGS, LDFLAGS, or LIBS variables.
  AC_MSG_CHECKING(whether large file support needs explicit enabling)
  ac_getconfs=''
  ac_result=yes
  ac_set=''
  ac_shellvars='CPPFLAGS LDFLAGS LIBS'
  for ac_shellvar in $ac_shellvars; do
    case $ac_shellvar in
      CPPFLAGS) ac_lfsvar=LFS_CFLAGS ;;
      *) ac_lfsvar=LFS_$ac_shellvar ;;
    esac
    eval test '"${'$ac_shellvar'+set}"' = set && ac_set=$ac_shellvar
    (getconf $ac_lfsvar) >/dev/null 2>&1 || { ac_result=no; break; }
    ac_getconf=`getconf $ac_lfsvar`
    ac_getconfs=$ac_getconfs$ac_getconf
    eval ac_test_$ac_shellvar=\$ac_getconf
  done
  case "$ac_result$ac_getconfs" in
    yes) ac_result=no ;;
  esac
  case "$ac_result$ac_set" in
    yes?*) ac_result="yes, but $ac_set is already set, so use its settings"
  esac
  AC_MSG_RESULT($ac_result)
  case $ac_result in
    yes)
      for ac_shellvar in $ac_shellvars; do
	eval $ac_shellvar=\$ac_test_$ac_shellvar
      done ;;
  esac
])


dnl ---------------------------------------------------------------------------
dnl From Paul Eggert <eggert@twinsun.com>

dnl Define HAVE_INTTYPES_H if <inttypes.h> exists,
dnl doesn't clash with <sys/types.h>, and declares uintmax_t.

AC_DEFUN(jm_AC_HEADER_INTTYPES_H,
[
  if test x = y; then
    dnl This code is deliberately never run via ./configure.
    dnl FIXME: this is a gross hack to make autoheader put an entry
    dnl for `HAVE_INTTYPES_H' in config.h.in.
    AC_CHECK_FUNCS(INTTYPES_H)
  fi
  AC_CACHE_CHECK([for inttypes.h], jm_ac_cv_header_inttypes_h,
  [AC_TRY_COMPILE(
    [#include <sys/types.h>
#include <inttypes.h>],
    [uintmax_t i = (uintmax_t) -1;],
    jm_ac_cv_header_inttypes_h=yes,
    jm_ac_cv_header_inttypes_h=no)])
  if test $jm_ac_cv_header_inttypes_h = yes; then
    ac_kludge=HAVE_INTTYPES_H
    AC_DEFINE_UNQUOTED($ac_kludge)
  fi
])


dnl ---------------------------------------------------------------------------
dnl From Paul Eggert <eggert@twinsun.com>

AC_DEFUN(AC_STRUCT_ST_MTIM_NSEC,
 [AC_CACHE_CHECK([for nanoseconds member of struct stat.st_mtim],
   ac_cv_struct_st_mtim_nsec,
   [ac_save_CPPFLAGS="$CPPFLAGS"
    ac_cv_struct_st_mtim_nsec=no
    # tv_nsec -- the usual case
    # _tv_nsec -- Solaris 2.6, if
    #	(defined _XOPEN_SOURCE && _XOPEN_SOURCE_EXTENDED == 1
    #	 && !defined __EXTENSIONS__)
    # st__tim.tv_nsec -- UnixWare 2.1.2
    for ac_val in tv_nsec _tv_nsec st__tim.tv_nsec; do
      CPPFLAGS="$ac_save_CPPFLAGS -DST_MTIM_NSEC=$ac_val"
      AC_TRY_COMPILE([#include <sys/types.h>
#include <sys/stat.h>], [struct stat s; s.st_mtim.ST_MTIM_NSEC;],
        [ac_cv_struct_st_mtim_nsec=$ac_val; break])
    done
    CPPFLAGS="$ac_save_CPPFLAGS"])

  if test $ac_cv_struct_st_mtim_nsec != no; then
    AC_DEFINE_UNQUOTED(ST_MTIM_NSEC, $ac_cv_struct_st_mtim_nsec)
  fi
 ]
)

dnl ---------------------------------------------------------------------------
dnl From Paul Eggert <eggert@twinsun.com>

dnl Define uintmax_t to `unsigned long' or `unsigned long long'
dnl if <inttypes.h> does not exist.

AC_DEFUN(jm_AC_TYPE_UINTMAX_T,
[
  AC_REQUIRE([jm_AC_HEADER_INTTYPES_H])
  if test $jm_ac_cv_header_inttypes_h = no; then
    AC_CACHE_CHECK([for unsigned long long], ac_cv_type_unsigned_long_long,
    [AC_TRY_COMPILE([],
      [unsigned long long i = (unsigned long long) -1;],
      ac_cv_type_unsigned_long_long=yes,
      ac_cv_type_unsigned_long_long=no)])
    if test $ac_cv_type_unsigned_long_long = yes; then
      AC_DEFINE(uintmax_t, unsigned long long)
    else
      AC_DEFINE(uintmax_t, unsigned long)
    fi
  fi
])


dnl ---------------------------------------------------------------------------
dnl From Steve Robbins <steve@nyongwa.montreal.qc.ca>

dnl From a proposed change made on the autoconf list on 2 Feb 1999
dnl http://sourceware.cygnus.com/ml/autoconf/1999-02/msg00001.html
dnl Patch for AIX 3.2 by Lars Hecking <lhecking@nmrc.ucc.ie> on 17 May 1999

AC_DEFUN(AC_FUNC_SELECT,
[AC_CHECK_FUNCS(select)
if test "$ac_cv_func_select" = yes; then
  AC_CHECK_HEADERS(unistd.h sys/types.h sys/time.h sys/select.h sys/socket.h)
  AC_MSG_CHECKING([argument types of select()])
  AC_CACHE_VAL(ac_cv_type_fd_set_size_t,dnl
    [AC_CACHE_VAL(ac_cv_type_fd_set,dnl
      [for ac_cv_type_fd_set in 'fd_set' 'int' 'void'; do
        for ac_cv_type_fd_set_size_t in 'int' 'size_t' 'unsigned long' 'unsigned'; do
          for ac_type_timeval in 'struct timeval' 'const struct timeval'; do
            AC_TRY_COMPILE(dnl
[#ifdef HAVE_SYS_TIME_H
#include <sys/time.h>
#endif
#ifdef HAVE_SYS_TYPES_H
#include <sys/types.h>
#endif
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif
#ifdef HAVE_SYS_SELECT_H
#include <sys/select.h>
#endif
#ifdef HAVE_SYS_SOCKET_H
#include <sys/socket.h>
#endif],
[extern select ($ac_cv_type_fd_set_size_t,
 $ac_cv_type_fd_set *,  $ac_cv_type_fd_set *, $ac_cv_type_fd_set *,
 $ac_type_timeval *);],
[ac_found=yes ; break 3],ac_found=no)
          done
        done
      done
    ])dnl AC_CACHE_VAL
  ])dnl AC_CACHE_VAL
  if test "$ac_found" = no; then
    AC_MSG_ERROR([can't determine argument types])
  fi

  AC_MSG_RESULT([select($ac_cv_type_fd_set_size_t,$ac_cv_type_fd_set *,...)])
  AC_DEFINE_UNQUOTED(fd_set_size_t, $ac_cv_type_fd_set_size_t)
  ac_cast=
  if test "$ac_cv_type_fd_set" != fd_set; then
    # Arguments 2-4 are not fd_set.  Some weirdo systems use fd_set type for
    # FD_SET macros, but insist that you cast the argument to select.  I don't
    # understand why that might be, but it means we cannot define fd_set.
    AC_EGREP_CPP(dnl
changequote(<<,>>)dnl
<<(^|[^a-zA-Z_0-9])fd_set[^a-zA-Z_0-9]>>dnl
changequote([,]),dnl
[#ifdef HAVE_SYS_TIME_H
#include <sys/time.h>
#endif
#ifdef HAVE_SYS_TYPES_H
#include <sys/types.h>
#endif
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif
#ifdef HAVE_SYS_SELECT_H
#include <sys/select.h>
#endif
#ifdef HAVE_SYS_SOCKET_H
#include <sys/socket.h>
#endif],dnl
    # We found fd_set type in a header, need special cast
    ac_cast="($ac_cv_type_fd_set *)",dnl
    # No fd_set type; it is safe to define it
    AC_DEFINE_UNQUOTED(fd_set,$ac_cv_type_fd_set))
  fi
  AC_DEFINE_UNQUOTED(SELECT_FD_SET_CAST,$ac_cast)
fi
])
