.. index:: info; target
.. _info_targets:

Show Targets found in Makefiles (`info targets`)
------------------------------------------------

**info targets** [ **names** | **positions** | **tasks** | **all** ]

Show the explicitly-named targets found in read Makefiles.

Suboptions are as follows:
* `names`: shows target names,
* `positions`: shows the location in the Makefile
* `all`:shows names and location
* `tasks`: shows target name if it has commands associated with it

Example:
++++++++

::

     remake<1> info targets
        	.C
        	.C.o
        	...
     Makefile:15:
        	.PHONY
        	.S
		...


.. seealso::

   :ref:`info tasks <info_tasks>`, :ref:`target <target>`, and :ref:`info target <info_target>`.
