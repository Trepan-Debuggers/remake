.. index:: break
.. _break:

Set a breakpoint (`break`)
--------------------------

**break** {*target* | *line-number*} [ **all** | **run** | **prereq** | **end** ]*

**break**



Set a breakpoint at a target or line number; also show breakpoints.

With a target name or a line number, set a break before running commands
of that target or line number.  Without argument, list all breakpoints.

For a given target, there are 3 places where one may want to stop at;
that name can be given as a last option. The stopping points are:

- before target prerequisite checking: `prereq`
- after target prerequisite checking but before running commands: `run`
- after target is complete: `end`

Giving `all` will stop in all of the above places. The default behavior is `run`.

If no location specification is given, use the current target.

Examples:
+++++++++

::

   break               # list all breakpoints
   break 10            # Break on line 10 of the Makefile we are
                       # currently stopped at
   break tests         # Break on the "tests" target
   break tests prereq  # Break on the "tests" target before dependency checking is done

.. seealso::

   :ref:`delete <delete>`.
