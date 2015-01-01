[![Build Status](https://travis-ci.org/rocky/remake.png)](https://travis-ci.org/rocky/remake)

Patched GNU Make 4.1 sources to add improved error reporting, tracing,
target listing, and a debugger. See the remake-3-82 branch for a
patched GNU Make 3.82.

Although there's a full debugger here, most of the time I can get by
using no options since normal output is a little more verbose and detailed.
When that isn't enough, I use the *--trace* or *-x* option, e.g:

    remake -x <other make options>

But if you want the full debugger, use *--debugger* or *-X*:

    remake -X <other make options>

To enter the debugger from inside a Makefile, use the built-in function *$(debugger)*. For example here is a Makefile:

    all:
    	$(debugger 'arg not used')
		echo Nothing here, move along

When GNU Make is inside the *all* target, it will make a call to the
debugger. The string after *debugger* is not used, but seems to be
needed to get parsing right.

If there is project that you want a list of "interesting" Makefile
targets, try:

    remake --tasks

If the project has commented its Makefile using remake-friendly comments you may get output like this:

    ChangeLog	# create ChangeLog fom git log via git2cl
    build	# Do what it takes to build software locally
    check	# Run all tests
    clean	# Remove OS- and platform-specific derived files.
    dist	# Create source and binary distribution
    distclean	# Remove all derived files. Like "clean" on steroids.
    install	# Install package
    test	# Same as check

To get a list of *all* targets, interesting or not, use *--targets*
instead of *--tasks*.

To build:

    $ autoreconf -i
    $ ./configure
    $ make update
    $ make && make check && sudo make install

See also https://github.com/rocky/remake/wiki

*Author for debugger portion:* Rocky Bernstein <rocky@gnu.org><br>
[![endorse](https://api.coderwall.com/rocky/endorsecount.png)](https://coderwall.com/rocky)
