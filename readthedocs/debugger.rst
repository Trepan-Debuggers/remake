.. contents:: :local:

The Remake Debugger
===================

When there problems in running GNU make, most of the time I can figure out what's wrong by switching to ``remake`` and looking at its call stack and extended error information.

When that is not sufficient, the ``--trace`` or ``-x`` option many times will fill in the gaps.

However there are situations when it is helpful to go deeper. So here and there is a full-fledged debugger built into ``remake``.

``remake`` can do four main kinds of things (plus other things in support of these) to help you catch bugs in the act:

* *examine* and query things: See the state of variables, see how they got expanded, where targets are defined, and look a the the state of targets
* *stop* at specified places such as targets, or when there is an error. In conjunction with this you can:
* *change* the internal state of things as though the Makefile were written differently
* *experiment* with Makefile code fragments possibly correcting the effects of one bug and go on to discover another.

Although you can use the ``remake`` debugger to debug Makefiles, it can also be used just as a front-end for learning more about Makefiles and
how GNU ``make`` or ``remake`` processes a Makefile.

.. toctree::
   :maxdepth: 2
   :caption: Contents:

   debugger/entry
   debugger/sessions
   debugger/syntax
   debugger/commands
