.. index:: step
.. _step:

Step into target (`step`)
-------------------------

**step** [ *count* ]

Step execution until the next target is encountered.

Stepping is like `next` but it is more fine-grained. However we
still don't stop at targets for which there is no rule.

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
