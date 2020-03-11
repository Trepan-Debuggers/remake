.. index:: backtrace
.. _backtrace:

Show Target Stack (`backtrace`)
-------------------------------

**backtrace** [*count*]

Print target stack or Makefile target stack with the most recent frame first.
An argument specifies the maximum amount of entries to show.

An arrow at the beginning of a line indicates the 'current frame'. The
current frame determines the context used for many debugger commands
such as expression evaluation or source-line listing.


Examples:
+++++++++

::

   backtrace    # Print a full stack trace
   backtrace 2  # Print only the top two entries

.. seealso::

   :ref:`up <up>`, :ref:`down <down>` and :ref:`frame <frame>`.
