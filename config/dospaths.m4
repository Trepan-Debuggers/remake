# Test if the system uses DOS-style pathnames (drive specs and backslashes)
# By Paul Smith <psmith@gnu.org>.  Based on dos.m4 by Jim Meyering.

AC_DEFUN([pds_AC_DOS_PATHS],
  [
    AC_CACHE_CHECK([whether system uses MSDOS-style paths], [ac_cv_dos_paths],
      [
        AC_COMPILE_IFELSE([
#if !defined _WIN32 && !defined __WIN32__ && !defined __MSDOS__ && !defined __EMX__
neither MSDOS nor Windows nor OS2
#endif
],
        [ac_cv_dos_paths=yes],
        [ac_cv_dos_paths=no])
      ])

    if test x"$ac_cv_dos_paths" = xyes; then
      AC_DEFINE_UNQUOTED([HAVE_DOS_PATHS], 1,
                         [Define if the system uses DOS-style pathnames.])
    fi
  ])
