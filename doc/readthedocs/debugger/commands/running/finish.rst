.. index:: finish
.. _finish:

Step out (`finish`)
-------------------

**finish** [ *count* ]

Run to the completion of the target, which is also known as "step out".

With no arguments, `remake` runs to the end of the target. Any prerequisite
checking and building that needs to occur is done and any shell commands
that occur get run.

This is analogous to "step out" in programming-language debuggers.

When *count* is a positive number, run `finish` *count* additional times.
The default value is 0.

.. seealso::

   :ref:`skip <step>`, :ref:`continue <continue>`, and
   :ref:`next <next>` provide other ways to progress execution.
