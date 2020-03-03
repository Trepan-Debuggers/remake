.. index:: step
.. _step:

Step into (step)
----------------

**step** [ *count* ]

Step execution until another stopping point is reached.

Argument *count* means do this *count* times (or until there's another
reason to stop.

Examples:
+++++++++

::

    step        # step 1 event, *any* event
    step 1      # same as above
    step 2      # same as: step; step

.. seealso::

   :ref:`next <next>` command. :ref:`skip <skip>`, :ref:`continue <continue>`, and :ref:`finish <finish>` provide other ways to progress execution.
