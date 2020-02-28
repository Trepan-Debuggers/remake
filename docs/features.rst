.. contents:: :local:

Features
========

Although debugging GNU Makefiles is a little different than debugging, procedure-oriented
programming langauges, this debugger tries similar to other_ trepanning_ debuggers_ and *gdb*
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

If you want to know what targets can be run, there are two new options:

* `--targets` gives a list of targets
* `--tasks`  gives a list of "interesting" targets

Before each target in the Makefile, you can give add a one-line comment
describing what the target does, starting the comment with `#:`.

If you do this, when either of these options is shown it will also be shown
with next to the target name. Here is an example:

.. code:: Makefile

	  #: Run Python pytests
	  check-short:
	       py.test pytest


Improved Execution Tracing
--------------------------

When the `-x` flag is given (or `--trace=normal`), any commands that
are about to be run are shown as seen in the `Makefile` along with
`set -x` tracing when run in a POSIX shell. ALso, we override or
rather ignore, any non-echo prefix `@` directive listed at the
beginnig of target commands.

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
* write a shell script containng the target commands with GNU Make variables expanded away, so the
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
