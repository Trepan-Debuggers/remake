.. contents:: :local:

Features
========

Although debugging GNU Makefiles is a little different than debugging, procedure-oriented
programming languages, this debugger tries similar to other_ trepanning_ debuggers_ and *gdb*
in general. So knowledge gained by learning this is transferable to those
debuggers and vice versa.

Profiling
---------

If you want to know where most of the time goes in building your system with Makefiles,
there is a `--profile` option which times the targets.

This option creates Callgrind Profile Format_ output which can be read
by KCachegrind_, callgrind_annotate_, or gprof2dot_ or other tools that understand this format.

You can get not only timings, but a graph of the target dependencies
checked.

Documenting Makefile Targets
----------------------------

Have you ever wanted `rake tasks` for GNU Make?  That is, you have
some strange `Makefile` and you want to see the interesting targets,
that you can run "make *target-name*" on?

There are two new options added to `remake` to assist this:

* `--tasks`  gives a list of "interesting" targets
* `--targets` gives a list of *all* targets

Before each target in the Makefile, you can give add a one-line comment
describing what the target does, starting the comment with `#:`.

If you do this, when either of these options is shown it will also be shown
with next to the target name.

Here is an example. Consider this `Makefile`:

.. code:: Makefile

    #: This is the main target
    all:
  	echo all here

    #: test things
    check:
	echo check here

    #: Build distribution
    dist:
	echo dist here

Running `remake --tasks` gives:

.. code:: console

    all         # This is the main target
    check       # test things
    dist        # Build distribution

Makefile searching in Parent Directories
----------------------------------------

When the `-c` flag is given (or `--search-parent`), if a Makefile or
goal target isn't found in the current directory, `remake` will search
in the parent directory for a Makefile. On finding a parent the
closest parent directory with a Makefile, `remake` will set its current working
directory to the directory where the Makefile was found.

In this respect the short option `-c`, is like `-C` except no
directory need to be specified.


Improved Execution Tracing
--------------------------

When the `-x` flag is given (or `--trace=normal`), any commands that
are about to be run are shown as seen in the `Makefile` along with
`set -x` tracing when run in a POSIX shell. Also, we override or
rather ignore, any non-echo prefix `@` directive listed at the
beginning of target commands.

If different granularity of tracing is desired the `--trace` option
has other settings. See the relevant parts of this manual for more information.

And, if you the most flexibility in tracing there is a built-in debugger.


Debugger
--------

Features of the debugger:

* Inspect target properties
* See the current target stack
* Set breakpoints
* Set and expand GNU Make variables
* Load in Makefiles
* write a shell script containing the target commands with GNU Make variables expanded away, so the
  shell code can be run (and debugged) outside of make.
* Enter debugger at the outset, call it from inside a Makefile, or enter it upon the first error

.. _pygments:  http://pygments.org
.. _pygments_style:  http://pygments.org/docs/styles/
.. _other: https://www.npmjs.com/package/trepanjs
.. _trepanning: https://pypi.python.org/pypi/trepan2
.. _debuggers: https://metacpan.org/pod/Devel::Trepan
.. _this: http://bashdb.sourceforge.net/pydb/features.html
.. _set_substitute:  https://zshdb.readthedocs.org/en/latest/commands/set/substitute.html
.. _set_style:  https://zshdb.readthedocs.org/en/latest/commands/set/style.html
.. _set_width:  https://zshdb.readthedocs.org/en/latest/commands/set/width.html
.. _eval: https://zshdb.readthedocs.org/en/latest/commands/data/eval.html
.. _step: https://zshdb.readthedocs.org/en/latest/commands/running/step.html
.. _install: http://zshdb.readthedocs.org/en/latest/install.html
.. _Format: https://valgrind.org/docs/manual/cl-format.html
.. _KCachegrind: https://kcachegrind.github.io/html/Home.html
.. _gprof2dot: https://github.com/jrfonseca/gprof2dot
.. _callgrind_annotate: http://man7.org/linux/man-pages/man1/callgrind_annotate.1.html
