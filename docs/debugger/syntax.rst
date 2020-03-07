.. _syntax_command:

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
