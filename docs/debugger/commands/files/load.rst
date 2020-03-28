.. index:: load
.. _load:

Read and Evaluate Makefile (`load`)
-----------------------------------

**load** *file-glob*

Read in and evaluate GNU Makefile *file-glob*..

*file-glob* should resolve after glob expansion to single GNU
Makefile. Target dependencies are updated after reading in the file.

Here are several possible uses of this command.

In debug sessions you can fix the source code and the run `load` to have the code reread in, to test out the fix.

Another use is to have pecific "debug"-oriented Makefiles that aren't
normally used, but when you want to trace things are avialable.
This is an aspect of aspect-oriented_ programming

.. _aspect-oriented: https://en.wikipedia.org/wiki/Aspect-oriented_programming
