/* Name of this package (needed by automake) */
#undef PACKAGE

/* Version of this package (needed by automake) */
#undef VERSION

/* Define to the name of the SCCS `get' command.  */
#undef SCCS_GET

/* Define this if the SCCS `get' command understands the `-G<file>' option.  */
#undef SCCS_GET_MINUS_G

/* Define this to enable job server support in GNU make.  */
#undef MAKE_JOBSERVER

/* Define to be the nanoseconds member of struct stat's st_mtim,
   if it exists.  */
#undef ST_MTIM_NSEC

/* Define this if the C library defines the variable `sys_siglist'.  */
#undef HAVE_SYS_SIGLIST

/* Define this if the C library defines the variable `_sys_siglist'.  */
#undef HAVE__SYS_SIGLIST

/* Define this if you have the `union wait' type in <sys/wait.h>.  */
#undef HAVE_UNION_WAIT

/* Define to `unsigned long' or `unsigned long long'
   if <inttypes.h> doesn't define.  */
#undef uintmax_t

/* These are for AC_FUNC_SELECT */

/* Define if the system doesn't provide fd_set.  */
#undef fd_set

/* Define the type of the first arg to select().  */
#undef fd_set_size_t

/* Define this if select() args need to be cast away from fd_set (HP-UX).  */
#undef SELECT_FD_SET_CAST
