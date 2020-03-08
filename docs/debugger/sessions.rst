Sample Debugger Sessions
========================

Debugging Debug Session
------------------------

oLet's go into the debugger initially. To do this, use the `--debugger`
or `-X` option. We'll use the Makefile from the source code of
this distribution.

*to be continued...*

Debugging Make Variables
-------------------------

*to be continued...*


Debugging Shell Commands
------------------------

Now consider the following sample Makefile `test2.mk`:

.. code:: Makefile

    PACKAGE=make

    all: $(PACKAGE).txt

    $(PACKAGE).txt: ../doc/remake.texi
  	makeinfo --no-headers $< > $@

Running this entering the debugger initially:

.. code:: console

    $ remake -X -f test2.mk
    ...
    Reading makefiles...
    updating makefiles....
    Updating goal targets....
      /tmp/remake/src/test2.mk:3	File `all' does not exist.

    -> (/tmp/test2.mk:5)
    make.txt: ../doc/remake.texi

We could use the `target` command to show information about
the current target, but that returns lots if information. So let us instead
narrow the information to just the automatic variables that get set. The
following commands do this are all mean the same thing: `target make.txt variables`,
`target @ variables`, and `info locals`.

.. code:: console

    @ := all
    % :=
    * :=
    + := make.txt
    | :=
    < := all
    ^ := make.txt
    ? :=

There is a `target` option to list just the shell commands of the
target:

.. code::

    remake<2> target make.txt commands

    make.txt:
    #  commands to execute (from `test2.mk', line 6):
	makeinfo --no-headers $< > $@

We can see a full expansion of the command that is about to be run:

.. code::

    remake<5> target @ expand

    #  commands to execute (from `test2.mk', line 6):
    	makeinfo --no-headers $< > $@

    #  commands to execute (from `test2.mk', line 6):
    	-makeinfo --no-headers ../doc/remake.texi > make.txt

Now if we want to write out commands as a shell script which
we might want to execute, we can use the :ref:`write <write>`
command:

.. code:: console

    (/tmp/remake/src/test2.mk:6): make.txt
    remake<6> write
    File "/tmp/make.txt.sh" written.

We can issue a shell command `cat -n /tmp/make.txt.sh` to see what
was written. See :ref:`shell <shell>`.

.. code:: console

    remake<7> shell cat -n /tmp/make.txt.sh
    #!/bin/sh
    # cd /tmp/remake/src/
    #/tmp/remake/src/test2.mk:5
    makeinfo --no-headers ../doc/remake.texi > make.txt


If you issue step commands, the debugger runs the each command and
stops. In this way, you can inspect the result of running that
particular shell command and decide to continue or not.

.. code:: console

    remake<8> step

      Must remake target `make.txt'.
    Invoking recipe from test2.mk:6 to update target `make.txt'.
    ##>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
    makeinfo --no-headers ../doc/remake.texi > make.txt
    ##<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<

    ++ (/tmp/test2.mk:5)

Notice that we've shown the expansion automatically. One subtle
difference in the above output, is that we only show the *single*
shell command that is about to be run when there are several
commands. In our example though, there is only one command; so there is
no a difference.

The `++` icon means that we are about to run that code.

.. code:: console

    make.txt
    remake<9> @b{step}
      Successfully remade target file `make.txt'.

    <- (/tmp/test2.mk:5)
    make.txt
    remake<10>

We ran the code, and are still at target `make.txt`. The `<-`
icon means that have finished with this target and are about to return.

If you are at a target and want to continue to the end of the target you
can use the command `finish` which is the same as `finish 0`.
