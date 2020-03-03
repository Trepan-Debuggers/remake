Entering the Debugger
*********************

.. toctree::
.. contents::


Invoking the Debugger Initially
===============================

The simplest way to debug your program is to call run `remake -X`. Give
the name of your Makefile:

Example Debugger Sessions
-------------------------

In this example we'll use the Makefile from libcdio-paranoia

.. code:: console

    $  remake --debugger
    ...
    Reading makefiles...
    Updating makefiles....

    -> (/tmp/remake/Makefile:1505)
    Makefile: Makefile.in config.status
    remake<0>

*To be continued...*


Calling the debugger within the Makefile
========================================

Sometimes it is not feasible to invoke the program from the debugger.
Although the debugger tries to set things up to make it look like your
program is called, sometimes the differences matter. Also the debugger
adds overhead and slows down your program.

Another possibility then is to a function call into your Makefile to call
the debugger at the spot you want to stop at.

Here is an Example:

.. code:: Makefile

          foo: bar

          bar:
                $(debugger "bar called")


.. code:: console

          ./make -f /tmp/foo.Makefile
          :o (/tmp/foo.Makefile:3)
          bar
          remake<0> where
          =>#0  bar at /tmp/foo.Makefile:3
            #1  foo at /tmp/foo.Makefile:1
          foo: bar
          remake<0>

Entering the debugger when `remake` encounters an error
=======================================================

This is done by supplying the `--post-mortem` or `-!` option on invocation.

Note that in contrast to the situations above. although you can examine state and evaluating expressions, execution
has terminated. Therefore, some of the execution-specific commands are no longer applicable.
