.. index:: expand
.. _expand:

Expand (Print a string expanded)
--------------------------------

**expand** *string*

Expands the string *string* given using `remake`'s internal
variables. The expansion would be the same as if the string were given
as a command inside the target.

Example:
++++++++

::


     remake<0> expand MAKE
     (origin default) MAKE := /tmp/remake/src/./make


     /tmp/remake/src/Makefile:264: Makefile.in

     remake<1> print MAKE  # note the difference with the print
     (origin default) MAKE = $(MAKE_COMMAND)

     remake<2> expand $(MAKE)   # Note using $( ) doesn't matter here...
     /tmp/remake/src/./make     # except in output format - no origin info

     /tmp/remake/src/Makefile:264: Makefile.in

     remake<2> p COMPILE
     Makefile:104 (origin: makefile) COMPILE := $(CC) $(DEFS) $(DEFAULT_INCLUDES)

     /tmp/remake/src/Makefile:264: Makefile.in
     remake<10> @b{x compile starts: $(CC) $(DEFS) $(DEFAULT_INCLUDES)}
     compile starts: gcc -DLOCALEDIR=\"\" -DLIBDIR=\"/usr/local/lib\" -DINCLUDEDIR=\"/usr/local/include\" -DHAVE_CONFIG_H -I. -I..

.. seealso::

   :ref:`print <print>`, `info variables <info-variables>`.
