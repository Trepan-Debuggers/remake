.. _syntax_command:
.. _icons:

Debugger Command Syntax
=======================

Command names and arguments are separated with spaces like POSIX shell
syntax. Parenthesis around the arguments and commas between them are
not used. If the character of a line starts with `#`,
the command is ignored. (Actually, what is going on here is that it is
a "comment" commant.)

Within a single command, tokens are then white-space split. Again,
this process disregards quotes or symbols that have meaning in GNU Make.
Some commands like :ref:`expand <expand>`, have access to the untokenized
string entered after the command name.

Resolving a command name involves possibly 2 steps. Some steps may be
omitted depending on early success or some debugger settings:

1. The leading token is next looked up in the debugger alias table and
the name may be substituted there.

2. After the above, The leading token is looked up a table of debugger
commands. If an exact match is found, the command name and arguments
are dispatched to that command.

GNU Readline is used to read commands, so it's capabilities are
available, such as `vi` or `emacs` editing.

Event Icons
-----------

In the debugger, before showing position information there is a two-character event icon.

For example, in his line:

.. code:: console

   !! (/tmp/project/errors.Makefile:1)
   ^^ event icon is here

The `!!` indicates an error occurred and we have gone into post-mortem debugging.

Here is a list of event icons:

.. index:: icons; event

.. list-table::
   :header-rows: 1

   * - Icon
     - Event
   * - `->`
     - Stopped before checking target prerequisites.
   * - `..`
     - Stopped after checking target prerequisites.
   * - `<-`
     - Stopped after running target commands.
   * - rd
     - About to read a Makefile
   * - `!!`
     - Error encountered and `--post-mortem flag given`. In post-mortem debugging.
   * - `- -`
     - Ran a debugger step of a Makefile target and it's not one of the above.
   * - `++`
     - Ran a debugger step in a POSIX command and it's not one of the above.
   * - :o
     - A call to the debugger using the $(debugger) function in the Makefile
   * - `||`
     - Finished making the goal target
