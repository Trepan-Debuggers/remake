.. index:: info; program
.. _info_program:

Show Makefile Information (`info program`)
------------------------------------------

**info program**

Show program information and why we are stopped

* Reason the program is stopped.
* The next line to be run

Example:
++++++++

::

     zshdb<1> info program
     Program stopped.
     It stopped after being stepped.
     Next statement to be run is:
     [ "${PS1-}" ]


.. seealso::

   :ref:`info line <info_line>`.
