.. index:: info; breakpoints
.. _info_breakpoints:

List all Breakpoints (`info break`)
-----------------------------------

**info break**

Show status of user-settable breakpoints.

The columns in the output lines are as follows:

* The \"Num\" column is the breakpoint number which can be used in a `delete` command.
* The \"Disp\" column contains one of \"keep\", \"del\"; the disposition of the breakpoint after it gets hit.
* The \"mask\" column indicates at which points of the breakpoint execution is stopped (0x01=prereq, 0x02=run, 0x04=end, 0x07=all).
* The \"Where\" column indicates where the breakpoint is located.

Example:
++++++++

::

   remake<1> info break
   Num Type           Disp Enb Mask Target  Location
     1 breakpoint     keep   y 0x07 help at /tmp/remake/docs/Makefile:12


Show breakpoints.

.. seealso::

   :ref:`break <break>`, :ref:`delete <delete>`
