.. index:: list
.. _list:

List Makefile target (`list`)
-----------------------------

**list** [ *target* ]

**list** *line-number* | **-**

List target dependencies and commands for *target* or *line-number*

Without a target name or line number, use the current target.
A target name of `-` will use the parent target on the target stack.

Examples:
+++++++++

::

    remake<0> list
    /tmp/remake/tests/spec/example/simple.Makefile:2
    all:
    #  recipe to execute (from '/tmp/remake/tests/spec/example/simple.Makefile', line 3):
	echo all here

    remake<1> list -
    ** We don't seem to have a parent target.

.. seealso::

   ref:`target <target>`, :ref:`edit <edit>`.
