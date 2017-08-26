#!/bin/bash
if (( $# != 1 )) ; then
    echo "Usage $0 callgrind-file"
fi
callgrind_file=$1
if [[ ! -r $callgrind_file ]] ; then
    echo "Can't find callgrind-file $callgrind_file"
fi
png_file=${callgrind_file}.png
gprof2dot -f callgrind $callgrind_file | dot -Tpng -o $png_file
rc=$?
if (( $rc == 0 )) ; then
    echo "created $png_file"
fi
exit $rc
