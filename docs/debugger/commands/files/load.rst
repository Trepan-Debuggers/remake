.. index:: load
.. _load:

Read and Evaluate Makefile (load)
---------------------------------

**load** *Makefile*

Read in and evaluate *Makefile*.

Note that dependencies are updated after reading the file in.

Here are several possible uses of this command.

In debug sessions you can fix the source code and the run `load` to have the code reread in, to test out the fix.

Another use is to have pecific "debug"-oriented Makefiles that aren't
normally used, but when you want to trace things are avialable.
This is an aspect of aspect-oriented_ programming

.. _aspect-oriented: https://en.wikipedia.org/wiki/Aspect-oriented_programming
