.. index:: help
.. _help:

Command Documentation (help)
----------------------------

**help** [ *command* [ *subcommand* ]

Get help for a debugger command or subcommand.

Without an argument, print the list of available debugger commands.

When an argument is given, it is first checked to see if it is command
name.

Some commands like `info`, `set`, and `show` can accept an
additional subcommand to give help just about that particular
subcommand. For example `help set basename` give help about the
`basename` subcommand of `set`.
