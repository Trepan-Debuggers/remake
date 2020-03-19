.. index:: next
.. _next:

Step over (`next`)
------------------

**next** [ *count* ]

Continue processing your Makefile until control reaches the next
interesting target, then stop and return control to the debugger.

Argument *count* means do this *count* times or until there's another reason to stop.

.. seealso::

   If you want more fine-grained stepping use :ref:`skip <step>`.

   If you want to not stop at any of targets the current target depends
   on, but instead run until after this target is remade, :ref:`finish <finish>`.

   :ref:`skip <skip>`, and :ref:`continue <continue>`, provide other ways to progress execution.
