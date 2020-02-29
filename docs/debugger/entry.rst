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

Another possibility then is to add statements into your program to call
the debugger at the spot in the program you want. To do this, you source
`zshdb/dbg-trace.sh` from where wherever it appears on your filesystem.
This needs to be done only once.

After that you call `_Dbg_debugger`.

Here is an Example:

.. code:: console

    source path-to-zshdb/zshdb/dbg-trace.sh
    # work, work, work.
    # ... some zsh code

    _Dbg_debugger
    # start debugging here


Since `_Dbg_debugger` a function call, it can be nested inside some sort of
conditional statement allowing one to be very precise about the
conditions you want to debug under. And until first call to `_Dbg_debugger`,
there is no debugger overhead.

Note that `_Dbg_debugger` causes the statement *after* the call to be stopped at.

Entering the debugger when `remake` encounters an error
=======================================================

This is done by supplying the `--post-mortem` or `-!` option on invocation.

Note that in contrast to the situations above. although you can examine state and evaluating expressions, execution
has terminated. Therefore, some of the execution-specific commands are no longer applicable.
