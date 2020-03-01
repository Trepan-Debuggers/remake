.. index:: break
.. _break:

Set a breakpoint (break)
------------------------

**break** {*target*|*line-number*} [**all**|**run**|**prereq**|**end**]*

**break**



Set a breakpoint at a target or line number; also show breakpoints.

With a target name or a line number, set a break before running commands
of that target or line number.  Without argument, list all breakpoints.

There are 3 place where one may want to stop at and that name can
be given as a last option. The stopping points are:

- before prerequisite checking `prereq`
- after prerequisite checking but before running commands `run`
- after target is complete: `end`

If no location specification is given, use the current target.

Examples:
+++++++++

::

   break              # list all breakpoints
   break 10           # Break on line 10 of the Makefile we are
                      # currently stopped at
   break tests        # Break on the "tests" target

.. seealso::

   :ref:`delete <delete>`.
