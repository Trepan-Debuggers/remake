.. index:: frame
.. _frame:

Absolute Target Stack Positioning (`frame`)
-------------------------------------------

**frame** [ *number* ]

Change the current target to target *number* if specified, or the
current target, 0, if no target number specified.


Examples:
+++++++++

::

   frame     # Set current frame at the current stopping point
   frame 0   # Same as above
   frame 1   # Move to frame 1. Same as: frame 0; up

.. seealso::

   :ref:`down <down>`, :ref:`up <up>`, :ref:`backtrace <backtrace>`
