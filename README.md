Patched GNU Make 3.82 sources to add improved error reporting, tracing,
and a debugger. 

Although there's a full debugger here. Most of the time I can get by
using the --trace or -x option, e.g

    remake -x <other make options>

But if you want the full debugger, use --debugger or -X:

    remake -X <other make options>

To build:

    $ autoreconf -i
    $ ./configure
    $ make -k update
    $ make && make check && sudo make install

See README.CVS for more detail regarding the above and for 
GNU Make CVS source instructions.

See also https://github.com/rocky/remake/wiki
