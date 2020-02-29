.. index:: print
.. _print:

Print (print variable info)
---------------------------

**print** [*variable*]

Expand `remake` variables.

Variable names should *not* be preceded with a dollar sign.

Note however that a more versatile print command is `examine` which
can print arbitrary string expands which of course includes variable.

If you omit *variable*, the last expression again is displayed again..

Example:
++++++++

::

    remake<0> print SHELL
    Makefile:168 (origin: makefile) SHELL = /bin/sh

    /tmp/remake/Makefile:243: Makefile.in
    remake<1> print $MAKE   # don't use $
    Can't find variable $MAKE

    /tmp/remake/Makefile:243: Makefile.in
    remake<1> print shell   # note case is significant
    Can't find variable shell

.. seealso::

   :ref:`expand <expand>`.
