.. index:: info; tasks
.. _info_tasks:

Show Targets with Descriptions (`info tasks`)
---------------------------------------------

**info tasks**

Show the targets that have descriptions.

A Description comment is a single line before a target that starts `#:`


Example:
++++++++

.. code:: Makefile

    #: This is the main target
    all:
  	@echo all here

    #: Test things
    check:
	@echo check here

    #: Build distribution
    dist:
	@echo dist here


.. code::

    remake<0> info tasks

    all                  This is the main target
    check                Test things
    dist                 Build distribution


.. seealso::

   :ref:`info target <info_target>`.
