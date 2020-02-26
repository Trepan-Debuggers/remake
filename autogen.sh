#!/bin/sh
# Run this to generate all the initial makefiles, etc.
# Additional options go to configure.

# honor MAKE variable if set otherwise set it to `make'
MAKE=${MAKE:-make}

echo "Rebuilding ./configure with autoreconf..."
autoreconf -f -i
if [ $? -ne 0 ]; then
  echo "autoreconf failed"
  exit $?
fi

./configure --enable-maintainer-mode "$@"
${MAKE} po-update
(cd doc && ${MAKE} stamp-1 stamp-vti)
