Here we have note remake changes. For changes to GNU Make see its NEWS file.

Version 4.2.1+dbg-1.5
======================

All of the below changes are backports from 4.3+dbg-1.5

New and Changed Features
-------------------------

* [`--search parent`](https://remake.readthedocs.io/en/latest/features.html#searching-for-a-makefile-in-parent-directories) -- I really like this one.
* [`--tasks`](https://remake.readthedocs.io/en/latest/features.html#listing-and-documenting-makefile-targets) -- the definition of a "tasks" has been simplified.
  Now, it is simply a target that has a description comment (`#:`) before it.
  After many years of using this myself, I highly encourage people to start using description comments more.
  Output from running `remake --tasks` is nicer because we use spaces to align columns rather than tabs.

Debugger Changes
----------------

* Commands with file expansion now use `glob()`, not `word_expand()` (Thomas did this too).
  Previously [`source`](https://remake.readthedocs.io/en/latest/debugger/commands/support/source.html) used to POSIX.1-2008 `wordexp()`, but this is not available on BSD-ish systems.`
  glob()` is more general, and GNU make ships with its own `glob()` function when none is provided in the underlying OS library.
* [`load`](https://remake.readthedocs.io/en/latest/debugger/commands/files/load.html#read-and-evaluate-makefile-load) command added `eval` command removed.
  `eval` never worked and it attempted to be the same thing as `load`; `load` is the _gdb_ name.
* `$(debugger)` function fixed.
   This function gave an `virtual memory exhausted` on exit. This has been fixed. The required parameter for this function, a tag name, is now shown on entry
* `info tasks` has been added. It is basically the same thing as `remake --tasks`
* Description lines are now shown on in [`info targets`](https://remake.readthedocs.io/en/latest/debugger/commands/info/target.html) and   [`list`](https://remake.readthedocs.io/en/latest/debugger/commands/files/list.html) commands
* The debugger now disallows any ["running" command](https://remake.readthedocs.io/en/latest/debugger/commands/running.html) inside post-mortem debugging
* Help as shown inside the debugger has been greatly expanded and more closely matches the sphinx docs.
   We now show in help text the short command name and any aliases attached to the command

New and Changed Features
-------------------------

* [`--search parent`](https://remake.readthedocs.io/en/latest/features.html#searching-for-a-makefile-in-parent-directories) -- I really like this one.
* [`--tasks`](https://remake.readthedocs.io/en/latest/features.html#listing-and-documenting-makefile-targets) -- the definition of a "tasks" has been simplified.
  Now, it is simply a target that has a description comment (`#:`) before it.
  After many years of using this myself, I highly encourage people to start using description comments more.
  Output from running `remake --tasks` is nicer because we use spaces to align columns rather than tabs.

Debugger Changes
----------------

* Commands with file expansion now use `glob()`, not `word_expand()` (Thomas did this too).
  Previously [`source`](https://remake.readthedocs.io/en/latest/debugger/commands/support/source.html) used to POSIX.1-2008 `wordexp()`, but this is not available on BSD-ish systems.`
  glob()` is more general, and GNU make ships with its own `glob()` function when none is provided in the underlying OS library.
* [`load`](https://remake.readthedocs.io/en/latest/debugger/commands/files/load.html#read-and-evaluate-makefile-load) command added `eval` command removed.
  `eval` never worked and it attempted to be the same thing as `load`; `load` is the _gdb_ name.
* `$(debugger)` function fixed.
   This function gave an `virtual memory exhausted` on exit. This has been fixed. The required parameter for this function, a tag name, is now shown on entry
* `info tasks` has been added. It is basically the same thing as `remake --tasks`
* Description lines are now shown on in [`info targets`](https://remake.readthedocs.io/en/latest/debugger/commands/info/target.html) and   [`list`](https://remake.readthedocs.io/en/latest/debugger/commands/files/list.html) commands
* The debugger now disallows any ["running" command](https://remake.readthedocs.io/en/latest/debugger/commands/running.html) inside post-mortem debugging
* Help as shown inside the debugger has been greatly expanded and more closely matches the sphinx docs.
   We now show in help text the short command name and any aliases attached to the command


Version 4.2.1+dbg-1.4 (2017-11-21)
==================================

Two small but important changes...

* `gnumake.h` -> `gnuremake.h` so we don't conflict with GNU make
* Propagate `--post-mortem` flag (now short option `-!`) in recursive make

Revise documentation to include recent --post-mortem and profiling changes.

Version 4.2.1+dbg-1.3 (2017-09-17)
===================================

* Add the `--post-mortem` option

Version 4.2.1+dbg-1.2 (2017-08-26)
===================================

* Add more information in debugger `write` command: the complete target name and dependencies   Format is a little nicer
* Remove more VMS, AMIGA and XENIX code which complicates things


Version 4.2.1+dbg-1.1 (2017-08-22)
======================

* Sync up with GNU Make 4.2.1 see `NEWS` for changes

Version 4.1+dbg-1.1 (2015-06-23)
================================

* Fix SEGV in debugger `set` command
* Add debugger `setqx` which is like `setq` (set a varable) but expands the value before it assigns

Version 4.1+dbg-1.0 (2015-06-3)  Marilyn release
==============================================

* Add `--profile`

This creates callgrind profile output for a given run the callgrind output can be used with `kcachegrind`, `callgrind_annotate`,
or `gprof2dot` to analyze data. You can get not only timings, but  a graph of the target dependencies checked.

Version 4.1+dbg-0.91 2015-01-1
===============================

* Code now based on GNU Make 4.1
* add remake `-verbosity` option. Can be used to suppress version information;
  version info only shown at level 0.
* More targets should have file locations now

Version 3.82+dbg-0.9 2012-10-3
===============================

* More aggressive on showing `--target` descriptions
* `-X` and `-x` are boolean options only use `--trace` and `--debugger` to supply an optional parameter
* GNU readline headers are no longer optional
* small packaging changes

Version 3.82+dbg-0.8 2011-10-27
================================

* Add remake `--tasks` descriptions
* Fix indentation bug in `--trace`
* Give process id in output line when `--jobs` value _n_ != 1
* More correct target depth calculation

Version 3.82+dbg-0.7 (2011-06-12) Fleetwood release
===================================================

* Add `--tasks` (akin to `rake --tasks`) option to list interesting targets; `--targets` is similar
* In debugger, allow negative frame numbers to count from the other end
* Output changes to make it easier for front-ends like the emacs-dbgr
* builds better on OS's with various versions of GNU readline

Version 3.82+dbg-0.6 2011-04-10
===============================

* Allow debugger breakpoints on line numbers; `info lines` shows the lines.
* debugger `list` also allows line numbers now.
* Remove duplicate stops in stepping and breakpoints
* Help and error message text greatly improved
* Documentation updated
* `break`/`continue` *target* run fixed
* `info files` more user friendly
* Correct order-only target display
* Recover from a fatal error when inside the debugger

Version 3.82+dbg-0.5 (2011-04-01)
=================================

* Track updating goal targets and more targets in general
* We track more target locations earlier and show that in tracing
* Add debugger `source` command
* Add `--no-readline` option to disable use of GNU Readline
* variable names are expanded on "break" and "continue" target names
* bug in history count fixed.
* Update manual to reflect the various changes.
* Don't unstall reremake

Version 3.82+dbg-0.4 2011-03-25
================================

* Completely revise using GNU make 3.82 as a base. Much
  cleaner code, and should support all of GNU make's OS's.
* Stepping now stops before each line of code is run.
* Add debugger `edit` command
* allow target name on `continue` command
* breakpoints can now stop after prerequisite checking and after
  remaking a target.
* Show line number in break command
* Change semantics of debugger "finish" command. Is now
  more similar to _gdb_, i.e is a "step out" up to a higher-level target
* Add back in command separator lines `##<<<<` and `##>>>>`
* Add stopping-event icons; `--`, `++`, `->`, `<-`, `||`, `!!`
* Added support for making debugger when GNU Readline is not available
* Start debugger tests via RSpec (in Ruby).
* Add debugger `eval` command
* Start to revise documentation for current release.
* Remove emacs debugger. Please use [realgud](https://github.com/realgud) code, available from ELPA or MELPA
* `mdb<>` prompt is now `remake<>`
* The usual doc fixes after a release

Version 3.81+dbg-0.3 (2010-05-01)
================================

* Bug fixes
* Compiles on Solaris

Version 3.81+dbg-0.2 (2009-01-10) - Sam Woodward Release
========================================================

* Bug in trying to fake file information for a .PHONY target
* Some merge cleanup. Add debugger-specific documentation (from 3.80+dbg-0.61)

Version 3.81+dbg-0.1 2008-10-27
===============================

* Merge in code from 3.80+dbg-0.63cvs.
* Add debugger `info rules` and `info vpath`

Version 3.80+dbg-0.63cvs 2006-04-01
====================================

* Add a debugger `list`d" command. Basically same as `target x depends commands` for now. `list -` shows the parent.
* Help commands now show short usage.
* Ctrl-d leaves debugger
* Add GNU Emacs regression test and allow space in filename for MS Windows

Version 3.80+dbg-0.62 2007-02-27
================================

* Changes to make more Debian compatibile.
* Track some GNU Make 3.81 changes, which include a bugfix
  and making more strings "const".
* Miscellaneous bugfixes

Version 3.80+dbg-0.61 2006-03-24
==================================

* Add comint shell tracking taken from python's `pdbtrack`
* Add gdb's `cd` and `pwd` commands.
* "write" command now adds "cd <directory name>" as a comment.
* `configure` options:
   `--with-default-shell` to specify the default value
     for SHELL (useful on say Solaris),
   `--with-shell-trace` to specify the option to turn on tracing, e.g. (-x).
* Add `configure` option `--with-shell-trace` to specify how to turn on
  tracing when running shell commands.
* Add remake command option `--trace=normal`.
* run wasn't reading any (optional) arguments supplied.
* Guard against dereferencing NULL pointers.
* Some 3.81 features and bugfixes have been incorporated. At present not everything is there.
* More doxygen documentation and docuementation improvements. Note "debugger options" in Manual page.

Version 3.80+dbg-0.60 2005-12-25
=================================

* On a fatal error we now show command arguments used to invoke program.
* tracing now sets shell tracing (+x) when invoking commands
* The default behavior on error is to show call trace. To turn off use `--no-extended-errors`. Recently added `--extended-errors` and `-E` (not in standard GNU Make) have been removed.
* debugger command `quit` can now terminate several levels of nested make (if _remake_  was called directly and not via a shell string).
* prompt now shows target and prerequisites.
* Fix bugs and memory leaks involving improper use of memory can weak casting. (shown in the vpath regression test dash-I on cygwin).
* Reorganize help command. Is more ddd-friendly (and more gdb like).
* More explicit free's of memory at termination.
* Give line numbers and target names on `--just-print`. Print delimiters
  around the commands.
* More debug tracing funnelled through `db_level`. Debugger command `show debug` shows the debug mask in a more understandable way.
* Incorporate patches from GNU/Linux make 3.80.
* Incorporate new functions from GNU Make 3.81: _abspath_, _realpath_, _lastword_
* Expand documentation.
* NOTE: the prompt has changed from `makedb` to `mdb` as has the GNU Emacs Lisp command name to run the debugger. This change was made to  avoid confusion between the Make debugger and a program which creates a DBM file. If you were using ddd-3.3.12-test.tar.gz you
  will need to get a newer version.

Version 3.80+dbg-0.5 2005-12-10
===============================

* Errors in debugger command `eval` no longer cause termination.
* Separation of debugger `step` and `next` commands. The default step command granularity is now "Next". "Step" stepping is more fine grained.
* debugger `target` command improvements:
   - sets file variables when showing is requested
   - add "target expand" to list target commands with "make" variables
     expanded.
   - allow @ or $@ as a target name
* Add debugger `info makefiles` to show what Makefiles have been read in.
* Add debugger `info breakpoints`
* Breakpoints can now be listed and deleted by number - like _gdb_.
  This also helps `ddd` and should pave the way for more sophisticated kinds of breaking/watching.
* Add a number of debugger command aliases (e.g. `backtrace`, `bt`, `!!`, `L`) and via an internal alias mechanism
* Remove hash stats on printing variables -- generally not wanted for users (rather than GNU Make maintainers)
* Add explicit memory free and reduce reading uninitialized values.
* Revise testing mechanism for a more modern use of Perl. Can specify test program file name (`./scripts/variables/special`) test name  (variables/special). The former is works better with command completion. Document some more regression tests.
* Make build work outside of source tree. `make distcheck` doesn't bomb. It doesn't run the regression tests though :-(
* More documentation and doxygenization.
* Remove AMIGA support :-(

Version 3.80+dbg-0.4 2005-12-10
===============================

* Add debugger commadn `eval!`
* Add ability to stop before reading Makefiles - may be useful in debugging macros and variables
* `--debugger=preread`. We can now trace makefile reading activity with `-d r` or via `--trace = {read,full}`
* debugger commadn `where` command expanded to include above new Makefile read stack.
* Add call tracing.
* Fix bugs in showing automatic variables
* Line number in Makefile commands is more accurate
* More ddd (gdb/bashdb) compatible:
  - `help set` added
  - `help show` works like the others
  - `info locals` added
  - `show command` (history) added
  - file/line reporting more like bash/perl
* write file takes basename of target file (well, at least for POSIX systems)
* Update documentation quite a bit
* Start to remove VMS support

Version 3.80+dbg-0.3
====================

* tracing adds GNU Make "basic" debugging. If debugging, then we also enter the debugger read loop. Hopefully this adds more granularity but not the diarrhea that "make -d" gives. To reduce this, "next" could be used to skip over remaking targets that don't have commands.
* print and examine now show origin status
* print/examine work for automatic variables. examine still has problems with automatic variables in strings though.
* Allow abbreviations of debugger command attributes. As a result some attributes were renamed slighly, e.g. `vars` -> `variables`, `deps` ->`depends`, `cmds` -> `commands`. But note that since substrings are allowed, `command` and `com`, and even `c` is the same as `commands`.
* Make option `--trace` (`-x`) overrules using the `--silent` option and not echoing `@` commands.
* debugger `list` command renamed to `target`. A future `list` command will look more like _gdb_s.
* fixed compilation issue(s) on systems that have readline, but do not have termcap.
* fixed failure of enter_debugger to exit on an empty line from readline.
* install program as `remake` by default (not `make`).
* misc bug fixes and compilations fixes for various OS's


Version 3.80+dbg-0.2
======================

* Add ability to print specified attributes of a target, e.g. "time", "deps", "vars"...
* Improve reporting target locations.
* Improve target stack tracking.
* Add "write" command - ability to write target commands to a file with `MAKE` variables expanded, or show that inside a debugger session.
* Separate info from show. Make these act more like they do in _gdb_
* Stay in debugger when a Makefile exits.
* First debugger documentation.

Version 3.80+dbg-0.1
=====================

Fork from version 3.80

* New options:
  `--extended-errors` (`-E`) call stack reporting, extended error reporting
  `--trace` (`-x`) gives tracing information before a command is run.
* Line numbers of commands in a target as they execute
* `--debugger` enters debugger
* GNU Emacs debug interface `makedb.el` patched into `gud.el`

Debugger commands are listed by _gdb_ category below.

*execution:*

* `step`
* `next`
* `continue`
* `skip`
* `restart`
* `quit`
* `exit`

*breakpoints:*

* `break`
* `target`
* `delete`
* `target`

*data:*

* `print`
* `variable`
* `examine`
* `string`
* `up`
* `down`
* `frame`
* `set`
* `setq`
* `where`

*debugger:*

* `show`
* `info`
* `trace`
* `help`
