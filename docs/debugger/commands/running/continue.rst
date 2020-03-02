.. index:: continue
.. _continue:

Continue Program execution (continue)
--------------------------------------

**continue** [ *target* [**all** | **run** | **prereq** | **end** ]* ]

Continue executing debugged Makefile until another breakpoint or
stopping point. If a target is given and valid we set a breakpoint at
that target before continuing.

As with the `break` command, the place in a target is in can be
specified. See :ref:`break <break>` for a list of the meanings of the
target phases.


Example:
++++++++

::

    continue          # Continue execution
    continue dist     # Continue until the "dist" target is reached

.. seealso::

   :ref:`next <next>` :ref:`skip <skip>`, and :ref:`step <step>` provide other ways to progress execution.
