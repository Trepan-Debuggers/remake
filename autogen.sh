#!/bin/sh
# Run this to generate all the initial makefiles, etc.
# Additional options go to configure.

# honor MAKE variable if set otherwise set it to `make'
MAKE=${MAKE:-make}

echo "Rebuilding ./configure with autoreconf..."
autoreconf -f -i || { rc=$?; echo "autoreconf failed"; exit $rc; }

# Add our target descriptions to po/Makefile.in.in
patch -p0 < po/Makefile.in.in.patch

./configure --enable-maintainer-mode "$@"
${MAKE} po-update
(cd doc && ${MAKE} stamp-1 stamp-vti)
