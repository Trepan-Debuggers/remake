.. _remake:

.. toctree::
.. contents::

remake manpage
##############

Synopsis
--------

**remake** [ *options* ] [*target*] ...

Description
-----------

``remake`` remake is forked and enahanced version of GNU Make that
adds improved error reporting, better tracing, profiling and a
debugger.

See GNU Make_ for information on GNU Make and its use.


Options
-------

Below we give options that are specific to ``remake``. For the other options,
please refer to the GNU Make documentation

:-c | --search-parent:

if a Makefile or goal target isn't found in the current directory, ``remake``
will search in the parent directory for a Makefile. On finding a parent the
closest parent directory with a Makefile, ``remake`` will set its current working
directory to the directory where the Makefile was found.

In this respect the short option `-c`, is like `-C` except no
directory need to be specified.

:-! | --post-mortem:

Go into the debugger on an error. This is the
Same as options: `--debugger --debugger-stop=error`

:--profile:

Creates callgrind profile output.
Callgrind output can be used with kcachegrind, callgrind_annotate,
or gprof2dot to analyze data. You can get not only timings, but
a graph of the target dependencies checked

:--targets:

Print a list of explicitly named targets found in read-in makefiles.

:--tasks:

Print a list of explicitly-named targets found in read-in makefiles which
have description comments. A description comment is added by putting
a single comment before the target that starts with `#:`. For example,
for this Makefile:

.. code:: Makefile

    #: This is the main target
    all:
  	@echo all here

    #: Test things
    check:
	@echo check here

    #: Build distribution
    dist:
	@echo dist here

Running `remake --tasks` gives:

.. code:: console

    all                  This is the main target
    check                Test things
    dist                 Build distribution

:-x | --trace:

Print debugging information in addition to normal processing.

If *flags* are omitted, then the behavior is the same as if `--trace=normal`
was specified

*flags* can be one of:

* `normal`: basic tracing and shell tracing; this is the default
* `read`:  for tracing all Makefiles read in,
* `noshell:` `which is like `normal` but shell tracing is disabled
* `full`: for maximum tracing

:-X | --debugger [=type]:

Enter debugger.

If *type* is given it may be one of:

* `normal`: basic tracing and shell tracing; this is the default
* `goal`: for all tracing Makefiles read
* `preaction` like `normal` but shell tracing is disabled
* `full`: for maximum tracing.
* `fatal`: for entering the debugger on a fatal error. The `--post-mortem` option sets this
* `error`: for entering the debugger on an error.

Bugs
----

Since this is derived from GNU Make, it most of its bugs.
See the chapter *Problems and Bugs* in "The GNU Make Manual" .

For ``remake``-specific bugs see https://github.com/rocky/remake/issues.


Authors
-------

GNU Make from which ``remake`` is derived, was written by Richard
Stallman and Roland McGrath, and is currently maintained by Paul
Smith.

However ``remake`` is the brainstorm of Rocky
Bernstein. The help of others though has been, and is, greatly appreciated.
Michael Lord Welles however thought of the name, ``remake``.

Copyright
---------

Copyright \(co 1992-1993, 1996-2020 Free Software Foundation, Inc.
This file is part of "GNU remake" .

GNU Remake is free software; you can redistribute it and/or modify it under the
terms of the GNU General Public License as published by the Free Software
Foundation; either version 3 of the License, or (at your option) any later
version.

GNU Remake is distributed in the hope that it will be useful, but WITHOUT ANY
WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR
A PARTICULAR PURPOSE.  See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License along with
this program.  If not, see http://www.gnu.org/licenses/ .

.. _Make: https://www.gnu.org/software/make/
