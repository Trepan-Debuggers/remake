Patched GNU Make 3.82 sources to add improved error reporting, tracing,
and a debugger. 

    $ autoreconf -i
    $ ./configure
    $ make -k update
    $ make && make check && sudo make install

See README.CVS for more detail regarding the above and for 
GNU Make CVS source instructions.

See also https://github.com/rocky/remake/wiki
