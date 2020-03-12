#!/bin/sh
# Run this to generate all the initial makefiles, etc.
# Additional options go to configure.

MAKE=${MAKE:-make}

echo "Rebuilding ./configure with autoreconf..."
autoreconf -f -i
if [ $? -ne 0 ]; then
  echo "autoreconf failed"
  exit $?
fi

# Add our target descriptions to po/Makefile.in.in
patch -p0 < po/Makefile.in.in.patch

./configure --enable-maintainer-mode "$@"
${MAKE} po-update
(cd doc && ${MAKE} stamp-1 stamp-vti)
