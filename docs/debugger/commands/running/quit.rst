.. index:: quit
.. _quit:

Quit (gentle termination)
-------------------------

**quit** [ *exit-code* ]

Exit `remake`. If a numeric argument is given, it will be the exit
status reported back. A status of 77 in a nested make will signals
termination in the parent. So if no numeric argument is given and
MAKELEVEL is 0, then status 0 is set; otherwise it is 77.

The program being debugged is aborted.

.. seealso::

   :ref:`run <run>` restarts the debugged program.
