/*! Versions of error and fatal with the ability to show call-stack. */
#ifndef REMAKE_DBG_MSG_H
#define REMAKE_DBG_MSG_H
#include "../make.h"

#if defined __STDC__ && __STDC__
void dbg_msg (const char *fmt, ...)
    __attribute__ ((__format__ (__printf__, 1, 2)));
void dbg_errmsg (const char *fmt, ...)
    __attribute__ ((__format__ (__printf__, 1, 2)));
#else
void dbg_msg();
void dbg_errmsg();
#endif

#endif
