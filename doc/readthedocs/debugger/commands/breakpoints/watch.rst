.. index:: watch
.. _watch:

Add a command watchpoint (`watch`)
-------------------------------------

**watch** *regex*

Add a "command watch" breakpoint that triggers before a command that is about to be executed matches the given regex.
An argument is the regex.

Example:
++++++++

::

    # 'watch' if something tries to delete "precious" directory
    watch rm -[rf] precious
    # Alternative regex
    watch \brm\b.+precious

.. seealso::

   :ref:`delete <delete>`, :ref:`break <break>`.
