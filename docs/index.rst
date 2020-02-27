.. remake documentation master file.
   You can adapt this file completely to your liking, but it should at least
   contain the root `toctree` directive.

remake - GNU Make with comprehensible tracing, profiling, extended error messages, and a debugger
=================================================================================================

`remake` is a fork of and extends GNU Make_. It adds, profiling,
comprehensible tracing, extended error messages and a debugger

Although debugging GNU Makefiles is a little different than debugging, procedure-oriented
programming langauges, this debugger tries similar to other_ trepanning_ debuggers_ and *gdb*
in general. So knowledge gained by learning this is transferable to those
debuggers and vice versa.

An Emacs interface is available via realgud_.

.. toctree::
   :maxdepth: 2
   :caption: Contents:

   manpage


Indices and tables
==================


* :ref:`genindex`
* :ref:`search`

.. _realgud: https://github.com/realgud/realgud
.. _other: https://www.npmjs.com/package/trepan-ni
.. _trepanning: https://pypi.python.org/pypi/trepan3k
.. _debuggers: https://metacpan.org/pod/Devel::Trepan
.. _Make: https://www.gnu.org/software/make/
