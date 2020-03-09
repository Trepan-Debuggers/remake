Sample Debugger Sessions
========================

.. contents:: :local:

.. toctree::
   :maxdepth: 1


An Extended Debug Session
-------------------------

In this session we will go into the debugger initially using the
`--debugger` or `-X` option. We'll use the Makefile from the source
code from cd-paranoia_

Basic Information when stopped inside Debugger
++++++++++++++++++++++++++++++++++++++++++++++

.. code:: console

    $ remake -X
    Reading makefiles...
    Updating makefiles...
    -> (/tmp/libcdio-paranoia/Makefile:428)
    Makefile: Makefile.in config.status
    remake<0>

The line immediately before the prompt `remake<0>`, we show the the
target name, `Makefile` and its dependencies: `Makefile.in` and
`config.status`.

The line before that has position information
`(/tmp/libcdio-paranoia/Makefile:428)`. But at the beginning of the
line is and arrow made up of two characters, `->`. This indicates that
we have not done prerequisite checking for this target yet.  Later we
will come across other two-character icons like `++`.
See :ref:`icons <icons>` for a complete list.

The zero in the prompt `remake<0>` is the command history number.  If GNU
Readline history support has it increments as we enter commands,
otherwise it stays zero.

For each recursive call to `remake`, we'll add another pair of angle
brackets `<>` around the number.

Some of the information is given in more verbose format using :ref:`info program <info_program>`:

.. code:: console

    remake<0> info program
    Starting directory `/tmp/libcdio-paranoia'
    Program invocation:
	remake/make  -X
    Recursion level: 0
    Line 428 of "/tmp/libcdio-paranoia/Makefile"
    Program stopped before rule-prequisite checking.
    remake<1>

Notice that the prompt has incremented to 1 after entering the a command.

Stepping
++++++++

We can use the :ref:`step, <step>` command to progress a little
in the interpretation or execution of the makefile:

.. code:: console

    remake<1> step
    -> (/tmp/libcdio-paranoia/Makefile:415)
    Makefile.in: Makefile.am m4/ld-version-script.m4 ...
    remake<2> step
    -> (/src/external-vcs/github/rocky/libcdio-paranoia/Makefile:443)
    aclocal.m4: m4/ld-version-script.m4 ...
    remake<3>

I have elided the list of dependencies listed above and substituted ellipses (`...`).

There is a slight difference in the target output seen above and what you will find in the Makefile. Below
I'll list the line as shown above versus what is in the file.

For line 415:

.. code:: makefile

    Makefile.in: Makefile.am m4/ld-version-script.m4 ...
    $(srcdir)/Makefile.in:  $(srcdir)/Makefile.am  $(am__configure_deps)

while line 443:

.. code:: makefile

    aclocal.m4: m4/ld-version-script.m4 ...
    $(ACLOCAL_M4):  $(am__aclocal_m4_deps)

In the debugger, variables have been expanded and file paths have been
canonicalized. Therefore you see `Makefile.in` for `$(srcdir)/Makefile.in` and
`aclocal.m4` for `$(ACLOCAL_M4)`.

Let's recap where `remake` is in the process of running the Makefile.
The first thing that seems to be done is that the `Makefile`
dependencies needs to checked. A dependency of `Makefile` is
`Makefile.in` and that in turn depends on target `aclocal.m4`. We have
now stepped into and stopped at that target. At the `remake<3>` prompt then
before checking for the dependencies of `aclocal.m4`.

You can see this dependency nesting that got us to this state using
the :ref:`backtrace <backtrace>` command:

.. code:: console

    remake<3> backtrace
    =>#0  aclocal.m4 at /tmp/libcdio-paranoia/Makefile:443
      #1  Makefile.in at /tmp/libcdio-paranoia/Makefile:415
      #2  Makefile at /tmp/libcdio-paranoia/Makefile:428
    remake<4>

Stepping through the program can be illuminating as far as what is
going on, especially when the Makefile has been derived in some way,
is the case here. This make file was created via `autotools`.

I had assumed that when I run `make` it looks for a default target and
runs that. But as we see here, the first thing that goes on is to
check to see if the Makefile is being used is itself out of date. If
that is the situation, then the Makefile will get recreated and you
start again.

However while all of this may be interesting, stepping can be a bit
tedious.

In the next section, we talk about :ref:`breakpoints` can get you to
where you want to debug faster. To finish this session though use the
:ref:`quit <quit>` command.

.. code:: console

   remake<4> quit
   remake: That's all, folks...

Breakpoints
+++++++++++

Let's say I am interested in what goes on when `make dist` is run.
Again, I'll invoke the debugger initially.

.. code:: console

    $ remake -X
    Reading makefiles...
    Updating makefiles...
    -> (/tmp/libcdio-paranoia/Makefile:428)
    Makefile: Makefile.in config.status
    remake<0>

Instead of stepping we can set a breakpoint on the `dist` target and
continue running to that point in one command, using :ref:`continue <continue>`.

.. code:: console

    remake<0> continue dist
    Breakpoint 1 on target `dist', mask 0x0f: file Makefile, line 703.
    Updating goal targets...
    -> (/src/external-vcs/github/rocky/libcdio-paranoia/Makefile:703)
    dist:
    remake<1>

Now when I issue a `step`, I will step into the commands associated with the `dist` target:

.. code:: console

   remake<1> step
   File 'dist' does not exist.
   Must remake target 'dist'.
   Makefile:704: target 'dist' does not exist
   ##>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
   remake  dist-bzip2 dist-gzip am__post_remove_distdir='@:'
   ##<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
   ++ (/src/external-vcs/github/rocky/libcdio-paranoia/Makefile:703)
   dist
   remake<2>

Notice that the event icon above is `++` which means I am stepping shell commands, here those associated with the Make target `dist`.
Above the line with the event icon in between the two chevrons is the command that is *about* to be run.


Debugging Make Variables
-------------------------

In the above session we have seen that output has variables expanded. You can query any GNU Make variable
that has been set in the program without variables inside expanded using the :ref:`print <print>` command.

.. code:: console

    remake<2> print MAKE
    (origin default) MAKE = $(MAKE_COMMAND)

The `(origin default)` means this is a built-in definition. The other
kind of print which does full expansion of the variables is called
`expand` or `x`. Here is an example

.. code:: console

    remake<3> expand MAKE
    (origin default) MAKE := remake

Note that in printing expanded values we use `:=` while non-expanded
values we use `=` This output matches the semantics of these
assignment operators.

In fact, `expand` doesn't need a variable name, it will work with a
string. So I could type `x This is $(MAKE)` or `x $(MAKE) $(DIST_TARGETS)`
For the latter, I get:

.. code:: console

    remake<4> x $(MAKE) $(DIST_TARGETS)
    remake dist-bzip2 dist-gzip

No location identification is given here since what I put in isn't a
variable. Also note that for `expand` I add the dollar sign and
parenthesis when there is other stuff. If you just want information
about the variable you can leave that off.

However for `print` you *never* add the dollar sign; printing only
prints *variables* not strings.

I change values too using either the :ref:`set <set>`, :ref:`set <setq>` or
:ref:`setqx <setqx>` commands. Let's see the difference between `set`
and `setq`:

.. code:: console

    remake<5> set MAKE $(MAKE_COMMAND)
    Variable MAKE now has value 'remake'
    remake<6>  setq MAKE $(MAKE_COMMAND)
    Variable MAKE now has value '$(MAKE_COMMAND)'

So with `set`, the value in the expression `$(MAKE_COMMAND)` is
expanded before the variable definition is assigned. With `setq` the
internal variables are kept unexpanded. Which you use or want is up to
you.

Note the irregular syntax of `set` and `setq`. Don't put an equal sign
between the variable and the expression. That is, `set MAKE = $(MAKE_COMMAND)` gives:

.. code:: console

    remake<7> set MAKE = $(MAKE_COMMAND)
    Variable MAKE now has value '= remake'

which is probably not what you want.  You can optionally put in the the
word "variable" when using `set` and "variable" is ignored. But
it won't be if you use `setq`.


Debugging POSIX Shell Commands
-------------------------------

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

.. code:: makefile

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

.. code:: console

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

Post-Mortem Debug Session
-------------------------

In this session we'll go into the debugger on encountering an error. For this the `--post-mortem`
or `-!` option is used. We'll use the Makefile from the source code of
this distribution.

*to be continued...*

.. _cd-paranoia: https://github.com/rocky/libcdio-paranoia
