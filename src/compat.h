/* src/compat.h - compatibility helpers (mempcpy fallback) */

#ifndef SRC_COMPAT_H
#define SRC_COMPAT_H

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include <string.h>
#include <stddef.h>

#ifndef HAVE_MEMPCPY
static inline void *
mempcpy(void *dest, const void *src, size_t n)
{
    memcpy(dest, src, n);
    return (char *) dest + n;
}
#endif

#endif /* SRC_COMPAT_H */
