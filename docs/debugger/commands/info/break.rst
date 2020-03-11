.. index:: info; breakpoints
.. _info_breakpoints:

List all Breakpoints (`info break`)
-----------------------------------

**info break**

Show status of user-settable breakpoints.

The columns in a line show are as follows:

* The \"Num\" column is the breakpoint number which can be used in a `delete` command.
* The \"Disp\" column contains one of \"keep\", \"del\", the disposition of the breakpoint after it gets hit.
* The \"mask\" at which points within the target that we stop
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
