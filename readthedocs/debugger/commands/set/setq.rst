.. index:: set; setq
.. _setq:

Set GNU Make Variable without Expansion (`setq`)
------------------------------------------------

**setq** *basename* *value*


Set GNU Make variable *variable* to *value*.

In contrast to `setqx`, variable definitions inside *value* are *not*
expanded before assignment occurs.

.. seealso::

   :ref:`setqx <setq>`, :ref:`set <set>`.
