.. index:: stack
.. _stack:

Examining the Target Stack (`backtrace`, `frame`, `up`, `down`)
===============================================================

The target stack is made up of targets linked by a dependency
from one target on the next.  The debugger assigns numbers to target frames counting
from zero for the innermost or currently executing target.

At any time the debugger identifies one target as the "selected"
target.  When the program being debugged stops, the debugger selects
the innermost targets.  The commands below can be used to select other
targets in the target stack by number.


.. toctree::
   :maxdepth: 1

   stack/backtrace
   stack/frame
   stack/up
   stack/down
