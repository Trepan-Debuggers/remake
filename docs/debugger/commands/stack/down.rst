.. index:: down
.. _down:

Relative Motion towards a more-recent Target (`down`)
-----------------------------------------------------

**down** [ *count* ]

Select and print the target this one caused to be examined.

If *count* is given then select that many targets down; the default is 1.

When you enter the debugger this command doesn't make a lot of sense
because you are at the most-recently frame. However if you issue
`down` and `frame` commands, this can change.

.. seealso::

   :ref:`up <up>` and :ref:`frame <frame>`.
