Entering the Debugger
*********************

.. toctree::
.. contents::


Invoking the Debugger Initially
===============================

The simplest way to debug your program is to call run `remake -X` or
`remake --debugger`.

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

	  debug:
                $(debugger "debug target break")

          bar:
                $(debugger "first bar command")
		@echo hi

	  baz: debug
	        @echo hello again


.. code:: console

          $ remake -f /tmp/foo.Makefile
	  debugger() function caled with parameter "bar called"
	  break
	  :o (/tmp/foo.Makefile:3)
          bar
          remake<0> where
          =>#0  bar at /tmp/foo.Makefile:6
            #1  foo at /tmp/foo.Makefile:1
          remake<0> quit
	  remake: That's all, folks...

          $ remake -f /tmp/foo.Makefile baz
	  debugger() function caled with parameter "debug target break"
	  break
	  :o (/src/external-vcs/github/rocky/remake/tmp/debugger.Makefile:3)
          debug
          remake<0> where
          =>#0  debug at /tmp/foo.Makefile:3
            #1  baz at /tmp/foo.Makefile:10
          remake<0>


Entering the debugger when `remake` encounters an error
=======================================================

This is done by supplying the `--post-mortem` or `-!` option on invocation.

Note that in contrast to the situations above. although you can examine state and evaluating expressions, execution
has terminated. Therefore, some of the execution-specific commands are no longer applicable.
