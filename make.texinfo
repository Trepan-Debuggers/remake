\input texinfo
@setfilename make.info
@synindex vr fn

@ifinfo
This file documents the GNU Make utility.

Copyright (C) 1988 Richard M. Stallman.

Permission is granted to make and distribute verbatim copies of
this manual provided the copyright notice and this permission notice
are preserved on all copies.

@ignore
Permission is granted to process this file through Tex and print the
results, provided the printed document carries copying permission
notice identical to this one except for the removal of this paragraph
(this paragraph not being relevant to the printed manual).

@end ignore
Permission is granted to copy and distribute modified versions of this
manual under the conditions for verbatim copying, provided that the entire
resulting derived work is distributed under the terms of a permission
notice identical to this one.

Permission is granted to copy and distribute translations of this manual
into another language, under the above conditions for modified versions.
@end ifinfo
@c
@setchapternewpage odd
@settitle Make

@titlepage
@sp 6
@center @titlefont{GNU Make}
@sp 1
@center A Program for Directing Recompilation
@sp 2
@center February 1988
@sp 5
@center Richard M. Stallman
@page
@vskip 0pt plus 1filll
Copyright @copyright{} 1988 Richard M. Stallman.

Permission is granted to make and distribute verbatim copies of
this manual provided the copyright notice and this permission notice
are preserved on all copies.

Permission is granted to copy and distribute modified versions of this
manual under the conditions for verbatim copying, provided that the entire
resulting derived work is distributed under the terms of a permission
notice identical to this one.

Permission is granted to copy and distribute translations of this manual
into another language, under the above conditions for modified versions.
@end titlepage
@page

@node Top, Simple,, (DIR)
@chapter Overview of @code{make}

The purpose of the @code{make} utility is to determine automatically which
pieces of a large program need to be recompiled, and issue the commands to
recompile them.  This manual describes the GNU implementation of
@code{make}.

Our examples show C programs, since they are most common, but you can use
@code{make} with any programming language whose compiler can be run with a
shell command.  In fact, @code{make} is not limited to programs.  You can
use it to describe any task where some files must be updated automatically
from others whenever the others change.

To prepare to use @code{make}, you must write a file called the
@dfn{makefile} that describes the relationships among files in your
program, and the states the commands for updating each file.  In a program,
typically the executable file is updated from object files, which are in
turn made by compiling source files.

Once a suitable makefile exists, each time you change some source files, this
simple shell command:

@example
make
@end example

@noindent
suffices to perform all necessary recompilations.  The @code{make} program
uses the makefile data base and the last-modification times of the files to
decide which of the files need to be updated.  For each of those files, it
issues the commands recorded in the data base.

@iftex
Command arguments to @code{make} can be used to control which files should
be recompiled, or how.  @xref{Running}.
@end iftex

GNU @code{make} was implemented by Richard Stallman and Roland McGrath.

@menu
* Simple::	A simple example explained.
* Makefiles::	The data base contains rules and variable definitions.
* Rules::	A rule says how and when to remake one file.
* Commands::    A rule contains shell commands that say how to remake.
* Variables::	A variable holds a text string for substitution into rules.
* Conditionals::Makefiles that do one thing or another depending on
		 variable values.
* Functions::   Functions can do text-processing within @code{make}.

* Running::     How to run @code{make}; how you can adjust the way
                 @code{make} uses the makefile.

* Implicit::	Implicit rules take over if the makefile doesn't say
		 how a file is to be remade.
* Archives::    How to use @code{make} to update archive files.
* Missing::     Features of other @code{make}s not supported by GNU @code{make}.
* Concept Index::Index of cross-references to where concepts are discussed.
* Name Index::  Index of cross-references for names of @code{make}'s
                 variables, functions, special targets and directives.
@end menu

@node Simple,,Top, Top
@section Simple Example of @code{make}

Suppose we have a text editor consisting of eight C source files and three
header files.  We need a makefile to tell @code{make} how to compile and
link the editor.  Assume that all the C files include @file{defs.h}, but
only those defining editing commands include @file{commands.h} and only low
level files that change the editor buffer include @file{buffer.h}.

To recompile the editor, each changed C source file must be recompiled.  If
a header file has changed, in order to be safe each C source file that
includes the header file must be recompiled.  Each compilation produces an
object file corresponding to the source file.  Finally, if any source file
has been recompiled, all the object files, whether newly made or saved from
previous compilations, must be linked together to produce the new
executable editor.

Here is a straightforward makefile that describes these criteria and says
how to compile and link when the time comes:

@example
edit : main.o kbd.o commands.o display.o \
              insert.o search.o files.o utils.o
        cc -o edit main.o kbd.o commands.o display.o \
              insert.o search.o files.o utils.o

main.o : main.c defs.h
        cc -c main.c
kbd.o : kbd.c defs.h command.h
        cc -c kbd.c
commands.o : command.c defs.h command.h
        cc -c commands.c
display.o : display.c defs.h buffer.h
        cc -c display.c
insert.o : insert.c defs.h buffer.h
        cc -c insert.c
search.o : search.c defs.h buffer.h
        cc -c search.c
files.o : files.c defs.h buffer.h command.h
        cc -c files.c
utils.o : utils.c defs.h
        cc -c utils.c
@end example

We split each long line into two lines using a backslash-newline; this is
like using one long line, but is easier to read.

Each file that is generated by a program---that is to say, each file except
for source files---is the @dfn{target} of a @dfn{rule} (@pxref{Rules}).
(In this example, these are the object files such as @file{main.o},
@file{kbd.o}, etc., and the executable file @file{edit}.)  The target
appears at the beginning of a line, followed by a colon.

After the colon come the target's @dfn{dependencies}: all the files that
are used as input when the target file is updated.  A target file needs to
be recompiled or relinked if any of its dependencies changes.  In addition,
any dependencies that are themselves automatically generated should be
updated first.  In this example, @file{edit} depends on each of the
eight object files; the object file @file{main.o} depends on the source
file @file{main.c} and on the header file @file{defs.h}.

By default, @code{make} starts with the first rule (not counting rules
whose target names start with @samp{.}).  This is called the @dfn{default
goal}.  Therefore, we put the rule for the executable program @file{edit}
first.  The other rules are processed because their targets appear as
dependencies in the goal.

After each line containing a target and dependencies come one or more lines
of shell commands that say how to update the target file.  These lines
start with a tab to tell @code{make} that they are command lines.
But @code{make} does not know anything about how the commands work.  It is up
to you to supply commands that will update the target file properly.  All
@code{make} does is execute the commands you have specified when the target
file needs to be updated.

@subsection How @code{make} Processes This Makefile

After reading the makefile, @code{make} begins its real work by processing
the first rule, the one for relinking @file{edit}; but before it can fully
process this rule, it must process the rules for the files @file{edit}
depends on: all the object files.  Each of these files is processed
according to its own rule.  These rules say to update the @samp{.o} file by
compiling its source file.  The recompilation must be done if the source
file, or any of the header files named as dependencies, is more recent than
the object file, or if the object file does not exist.

Before recompiling an object file, @code{make} considers updating its
dependencies, the source file and header files.  This makefile does not
specify anything to be done for them---the @samp{.c} and @samp{.h} files
are not the targets of any rules---so nothing needs to be done.  But
automatically generated C programs, such as made by Yacc, would be updated
by their own rules at this time.

After recompiling whichever object files need it, @code{make} can now
decide whether to relink @file{edit}.  This must be done if the file
@file{edit} does not exist, or if any of the object files are newer than
it.  If an object file was just recompiled, it is now newer than
@file{edit}, so @file{edit} will be relinked.

@subsection Variables Make Makefiles Simpler

In our example, we had to list all the object files twice in the rule for
@file{edit} (repeated here):

@example
edit : main.o kbd.o commands.o display.o \
              insert.o search.o files.o utils.o
        cc -o edit main.o kbd.o commands.o display.o \
              insert.o search.o files.o utils.o
@end example

@vindex objects
Such duplication is error-prone; if a new object file is added to the
system, we might add it to one list and forget the other.  We can eliminate
the risk and simplify the makefile by using a @dfn{variable}.  Variables
allow a text string to be defined once and substituted in multiple places
later (@pxref{Variables}).

It's standard practice for every makefile to have a variable named
@code{objects}, @code{OBJECTS} or @code{OBJ} which is a list of all object
file names.  We would define such a variable @code{objects} with a line
like this in the makefile:

@example
objects = main.o kbd.o commands.o display.o \
              insert.o search.o files.o utils.o
@end example

@noindent
Then, each place we want to put a list of the object file names, we can
substitute the variable's value by writing @samp{$(objects)}
(@pxref{Variables}).  Here is how the rule for @code{edit} looks as a
result:

@example
edit : $(objects)
        cc -o edit $(objects)
@end example

@subsection Letting @code{make} Deduce the Commands

It is not necessary to spell out the commands for compiling the individual
C source files, because @code{make} can figure them out: it has an
@dfn{implicit rule} for updating a @samp{.o} file from a correspondingly
named @samp{.c} file using a @samp{cc -c} command.  For example, it will
use the command @samp{cc -c main.c -o main.o} to compile @file{main.c} into
@file{main.o}.  We can therefore omit the commands from the rules for the
object files.  @xref{Implicit}.@refill

When a @samp{.c} file is used automatically in this way, it is also
automatically added to the list of dependencies.  We can therefore omit the
@samp{.c} files from the dependencies, provided we omit the commands.

Here is the entire example, with both of these changes, and a variable
@code{objects} as suggested above:

@example
objects =  main.o kbd.o commands.o display.o \
 insert.o search.o files.o utils.o

edit : $(objects)
        cc -o edit $(objects)

main.o : defs.h
kbd.o : defs.h command.h
commands.o : defs.h command.h
display.o : defs.h buffer.h
insert.o : defs.h buffer.h
search.o : defs.h buffer.h
files.o : defs.h buffer.h command.h
utils.o : defs.h
@end example

@noindent
This is how we would write the makefile in actual practice.

@subsection Another Style of Makefile

Since the rules for the object files specify only dependencies, no
commands, one can alternatively combine them by dependency instead of by
target.  Here is what it looks like:

@example
objects =  main.o kbd.o commands.o display.o \
 insert.o search.o files.o utils.o

edit : $(objects)
        cc -o edit $(objects)

$(objects): defs.h
kbd.o commands.o files.o : command.h
display.o insert.o search.o files.o : buffer.h
@end example

@noindent
Here @file{defs.h} is given as a dependency of all the object files;
@file{commands.h} and @file{buffer.h} are dependencies of the specific
object files listed for them.

Whether this is better is a matter of taste: it is more compact, but some
people dislike it because they find it clearer to put all the information
about each target in one place.

@node Makefiles, Rules, Simple, Top
@chapter Writing Makefiles

@cindex makefile
The information that tells @code{make} how to recompile a system comes from
reading a data base called the @dfn{makefile}.

@menu
* Contents: Makefile Contents.   Overview of what you put in a makefile.
* Names: Makefile Names.         Where @code{make} finds the makefile.
* Include::                      How one makefile can use another makefile.
@end menu

@node Makefile Contents, Makefile Names, Makefiles, Makefiles
@section What Makefiles Contain

Makefiles contain four kinds of things: @dfn{rules}, @dfn{variable
definitions}, @dfn{directives} and @dfn{comments}.  Rules, variables and
directives are described at length in later chapters.@refill

@itemize @bullet
@item
A rule says when and how to remake one or more files, called the rule's
@dfn{targets}.  It lists the other files that the targets @dfn{depend on},
and may also give commands to use to create or update the targets.
@xref{Rules}.

@item
A variable definition is a line that specifies a text string value for
a @dfn{variable} that can be substituted into the text later.  The
simple makefile example (@pxref{Simple}) shows a variable definition
for @code{objects} as a list of all object files.  @xref{Variables},
for full details.

@item
A directive is a command for @code{make} to do something special while
reading the makefile.  These include:

@itemize @bullet
@item
Reading another makefile (@pxref{Include}).

@item
Deciding (based on the values of variables) whether to use or ignore a
part of the makefile (@pxref{Conditionals}).

@item
Defining a variable from a verbatim string containing multiple lines
(@pxref{Defining}).
@end itemize

@item
@cindex comments
@samp{#} in a line of a makefile starts a comment.  It and the rest of
the line are ignored.  Comments may appear on any of the lines in the
makefile, except within a @code{define} directive, and perhaps within
commands (where the shell decides what is a comment).  A line
containing just a comment (with perhaps spaces before it) is
effectively blank, and is ignored.
@end itemize

@node Makefile Names, Include, Makefile Contents, Makefiles
@section What Name to Give Your Makefile

By default, when @code{make} looks for the makefile, it tries the names
@file{./makefile} or @file{./Makefile} in that order.  So normally you call
your makefile by one of these two names, and @code{make} finds it
automatically.  We recommend @file{Makefile} because it appears prominently
near the beginning of a directory listing.

If @code{make} finds neither of these two names, it does not use any
makefile.  Then you must specify a goal with a command argument, and
@code{make} will attempt to figure out how to remake it using only its
built-in implicit rules.@refill

If you want to use a nonstandard name for your makefile, you can specify
the makefile name with the @samp{-f} option.  The arguments @samp{-f
@var{name}} tell @code{make} to read the file @var{name} as the makefile.
If you use more than one @samp{-f} option, you can specify several
makefiles.  All the makefiles are effectively concatenated in the order
specified.  The default makefile names @file{./makefile} and
@file{./Makefile} are not used if you specify @samp{-f}.@refill

@vindex MAKEFILES
If the environment variable @code{MAKEFILES} is defined, @code{make}
considers its value as a list of names (separated by whitespace) of
additional makefiles to be read before the others.  This works much like
the @code{include} directive: various directories are searched for those
files and the default goal is never taken from them.  @xref{Include}.  In
addition, it is not an error if the files listed in @code{MAKEFILES} are
not found.

The main use of @code{MAKEFILES} is in communication between recursive
invocations of @code{make} (@pxref{Recursion}).  It usually isn't
desirable to set the environment variable before a top-level invocation
of @code{make}, because it is usually better not to mess with a makefile
from outside.

Some users are tempted to set @code{MAKEFILES} in the environment
automatically on login, and program makefiles to expect this to be done.
This is a very bad idea, because such makefiles will fail to work if run by
anyone else.  It is much better to write explicit @code{include} directives
in the makefiles.

@node Include,, Makefile Names, Makefiles
@section Including Other Makefiles

@findex include
The @code{include} directive tells @code{make} to suspend reading the
current makefile and read another makefile before continuing.  The
directive is a line in the makefile that looks like this:

@example
include @var{filename}
@end example

Extra spaces are allowed and ignored at the beginning of the line, but a
tab is not allowed.  (If the line begins with a tab, it will be considered
a command line.)  Whitespace is required between @code{include} and
@var{filename}; extra whitespace is ignored there and at the end of the
directive.  A comment starting with @samp{#} is allowed at the end of the
line.

Reading of the containing makefile is temporarily suspended while the file
@var{filename} is read as a makefile.  When that is finished, @code{make}
goes on with reading the makefile in which the directive appears.

The default goal target is never taken from an included makefile
(@pxref{Goals}).

One occasion for using @code{include} directives is when several programs,
handled by individual makefiles in various directories, need to use a
common set of variable definitions (@pxref{Setting}) or pattern rules
(@pxref{Pattern Rules}).

If the specified name does not start with a slash, and the file is not
found in the current directory, several other directories are searched.
First, any directories you have specified with the @samp{-I} option are
searched (@pxref{Options}).  Then the following directories (if they
exist) are searched, in this order: @file{/usr/gnu/include},
@file{/usr/local/include}, @file{/usr/include}.@refill

@node Rules, Commands, Makefiles, Top
@chapter Writing Rules

@cindex rule
@cindex target
@cindex dependency
A @dfn{rule} appears in the makefile and says when and how to remake
certain files, called the rule's @dfn{targets} (usually only one per rule).
It lists the other files that are the @dfn{dependencies} of the target, and
@dfn{commands} to use to create or update the target.

The order of rules is not significant, except for determining the
@dfn{default goal}: the target for @code{make} to consider, if you do not
otherwise specify one.  The default goal comes from the first rule (not
counting included makefiles) whose target does not start with a period.
Therefore, the first rule is normally one for compiling the entire program
or all the programs described by the makefile.  @xref{Goals}.

@menu
* Rule Example::        An explained example of a rule.
* Rule Syntax::	        General syntax of rules, with explanation.

* Wildcards::	        Using wildcard characters like `*' in file names.
* Directory Search::    Searching other directories for source files.

* Phony Targets::       Using a target that isn't a real file's name.
* Special Targets::     Targets with special built-in meanings.
* Empty Targets::       Real files that are empty--only the date matters.
* Multiple Targets::    When it is useful to have several targets in a rule.
* Multiple Rules::      Using several rules with the same target.
* Double-Colon::        Special kind of rule allowing
                          several independent rules for one target.
* Commands::            Special features and details of how commands
                         in a rule are executed.
@end menu

@ifinfo
@node Rule Example, Rule Syntax, Rules, Rules
@isubsection Rule Example

Here is an example of a rule:

@example
foo.o : foo.c defs.h       # module for twiddling the frobs
        cc -c -g foo.c
@end example

Its target is @file{foo.o} and its dependencies are @file{foo.c} and
@file{defs.h}.  It has one command, which is @samp{cc -c -g foo.c}.
The command line starts with a tab to identify it as a command.

This rule says two things:

@itemize @bullet
@item
How to decide whether @file{foo.o} is out of date: it is out of date
if it does not exist, or if either @file{foo.c} or @file{defs.h} is
more recent than it.

@item
How to update the file @file{foo.o}: by running @code{cc} as stated.
The command does not explicitly mention @file{defs.h}, but we presume
that @file{foo.c} includes it, and that that is why @file{defs.h} was
added to the dependencies.
@end itemize
@end ifinfo

@node Rule Syntax, Wildcards, Rule Example, Rules
@section Rule Syntax

In general, a rule looks like this:

@example
@var{targets} : @var{dependencies}
        @var{command}
        @var{command}
        ...
@end example

@noindent
or like this:

@example
@var{targets} : @var{dependencies} ; @var{command}
        @var{command}
        @var{command}
        ...
@end example

The @var{targets} are file names, separated by spaces.  Wild card
characters may be used (@pxref{Wildcards}) and a name of the form
@file{@var{a}(@var{m})} represents member @var{m} in archive file @var{a}
(@pxref{Archive Members}).  Usually there is only one target per rule, but
occasionally there is a reason to have more (@pxref{Multiple Targets}).

The @var{command} lines start with a tab character.  The first command may
appear on the line after the dependencies, with a tab character, or may
appear on the same line, with a semicolon.  Either way, the effect is the
same.  @xref{Commands}.

Because dollar signs are used to start variable references, if you really
want a dollar sign in the rule you must write two of them (@samp{$$}).
@xref{Variables}.  A long line may be split by inserting a backslash
followed by a newline, but this is not required, as there is no limit on
the length of a line.

A rule tells @code{make} two things: when the targets are out of date,
and how to update them when necessary.

The criterion for being out of date is specified in terms of the
@var{dependencies}, which consist of file names separated by spaces.
(Wildcards and archive members are allowed here too.)  A target is out of
date if it does not exist or if it is older than any of the dependencies
(by comparison of last-modification times).  The idea is that the contents
of the target file are computed based on information in the dependencies,
so if any of the dependencies changes the contents of the existing target
file are no longer necessarily valid.

How to remake is specified by @var{commands}.  These are lines to be
executed by the shell (normally @samp{sh}), but with some extra features
(@pxref{Commands}).

@node Wildcards, Directory Search, Rule Syntax, Rules
@section Using Wildcards Characters in File Names
@cindex wildcard
@cindex file name

A single file name can specify many files using @dfn{wildcard characters}.
The wildcard characters in @code{make} are @samp{*}, @samp{?} and
@samp{[@dots{}]}, the same as in the Bourne shell.  For example, @file{*.c}
specifies a list of all the files (in the working directory) whose names
end in @samp{.c}.@refill

Wildcard expansion happens automatically in targets, in dependencies, and
in commands.  In other contexts, wildcard expansion happens only if you
request it explicitly with the @code{wildcard} function.

The special significance of a wildcard character can be turned off by
preceding it with a backslash.  Thus, @file{foo\*bar} would refer to a
specific file whose name consists of @samp{foo}, an asterisk, and
@samp{bar}.@refill

@menu
* Examples: Wildcard Examples.    Some simple examples.
* Pitfall: Wildcard Pitfall.      @code{*.o} won't do what you want!
* Function: Wildcard Function.
       How to do wildcard expansion when defining a variable
       using the function @code{wildcard}.
@end menu

@node Wildcard Examples, Wildcard Function, Wildcards, Wildcards
@subsection Wildcard Examples

Wildcards can be used in the commands of a rule.  For example, here is a
rule to delete all the object files:

@example
clean:
        rm -f *.o
@end example

Wildcards are also useful in the dependencies of a rule.  With the
following rule in the makefile, @samp{make print} will print all the
@samp{.c} files that have changed since the last time you printed them:

@example
print: *.c
        lpr -p $?
        touch print
@end example

@noindent
This rule uses @file{print} as an empty target file; @pxref{Empty Targets}.

Wildcard expansion does not happen when you define a variable.  Thus, if
you write this:

@example
objects=*.o
@end example

@noindent
then the value of the variable @code{objects} is the actual string
@samp{*.o}.  However, if you use the value of @code{objects} in a target,
dependency or command, wildcard expansion will take place at that time.

@node Wildcard Pitfall, Wildcard Function, Wildcard Examples, Wildcards
@subsection Pitfalls of Using Wildcards

Now here is an example of a naive way of using wildcard expansion, that
does not do what you would intend.  Suppose you would like to say that the
executable file @file{foo} is made from all the object files in the
directory, and you write this:

@example
objects=*.o

foo : $(objects)
        cc -o foo $(CFLAGS) $(objects)
@end example

@noindent
The value of @code{objects} is the actual string @samp{*.o}.  Wildcard
expansion happens in the rule for @file{foo}, so that each @emph{existing}
@samp{.o} file becomes a dependency of @file{foo} and will be recompiled if
necessary.

But what if you delete all the @samp{.o} files?  Then @samp{*.o} will
expand into @emph{nothing}.  The target @file{foo} will have no
dependencies and would be remade by linking no object files.  This is not
what you want!

Actually you can use wildcard expansion for this purpose, but you need more
sophisticated techniques, including the @code{wildcard} function and string
substitution.
@ifinfo
@xref{Wildcard Function}.
@end ifinfo
@iftex
These are described in the following section.
@end iftex

@node Wildcard Function,, Wildcard Pitfall, Wildcards
@subsection The Function @code{wildcard}
@findex wildcard

Wildcard expansion happens automatically in rules.  But wildcard expansion
does not normally take place when a variable is set, or inside the
arguments of a function.  If you want to do wildcard expansion in such
places, you need to use the @code{wildcard} function, like this:

@example
$(wildcard @var{pattern})
@end example

This string, used anywhere in a makefile, is replaced by a space-separated
list of names of existing files that match the pattern @var{pattern}.

One use of the @code{wildcard} function is to get a list of all the C source
files in a directory, like this:

@example
$(wildcard *.c)
@end example

We can change the list of C source files into a list of object files by
substituting @samp{.o} for @samp{.c} in the result, like this:

@example
$(subst .c,.o,$(wildcard *.c))
@end example

Here we have used another function, @code{subst} (@pxref{Text Functions}).

Thus, a makefile to compile all C source files in the directory and then
link them together could be written as follows:

@example
objects=$(subst .c,.o,$(wildcard *.c))

foo : $(objects)
        cc -o foo $(LDFLAGS) $(objects)
@end example

@noindent
(This takes advantage of the implicit rule for compiling C programs, so
there is no need to write explicit rules for compiling the files.)

@node Directory Search, Phony Targets, Wildcards, Rules
@section Searching Directories for Dependencies
@vindex VPATH
@cindex directory search

For large systems, it is often desirable to put sources in a separate
directory from the binaries.  The @code{VPATH} feature makes this easier.

The value of the variable @code{VPATH} is a list of directories which
@code{make} should search (in the order specified) for dependency files.
The directory names are separated by colons.  For example:

@example
VPATH = src:../headers
@end example

@noindent
specifies a path containing two directories, @file{src} and @file{../headers}.

When a file listed as a dependency does not exist in the current directory,
the directories listed in @code{VPATH} are searched for a file with that
name.  If a file is found in one of them, that file becomes the dependency.
Rules may then specify the names of source files as if they all existed in
the current directory.

Using the value of @code{VPATH} set in the previous example, a rule like this:

@example
foo.o : foo.c
@end example

@noindent
is interpreted as if it were written like this:

@example
foo.o : src/foo.c
@end example

@noindent
assuming the file @file{foo.c} does not exist in the current directory but
is found in the directory @file{src}.

But what about the rule's commands?  The @code{VPATH} directory search
cannot change the commands; they will execute as written.  You need to
write the commands so that they will use the file names that @code{make}
finds.  This is done with the @dfn{automatic variables} such as @samp{$^}
(@pxref{Automatic}).  For instance, the value of @samp{$^} is a list of all
the dependencies of the rule, including the names of the directories in
which they were found, and the value of @samp{$@@} is the target.  Thus:

@example
foo.o : foo.c
        cc -c $(CFLAGS) $^ -o $@@
@end example

@noindent
The variable @code{CFLAGS} exists so you can specify flags for C
compilation by changing its value; we use it here for consistency so it
will affect all C compilations uniformly.  (@pxref{Implicit Variables}).

Often the dependencies include header files as well, which you don't want
to mention in the commands.  The function @code{firstword} can be used to
extract just the first dependency from the entire list, as shown here
(@pxref{Filename Functions}):

@example
foo.o : foo.c defs.h hack.h
        cc -c $(CFLAGS) $(firstword $^) -o $@@
@end example

@noindent
Here the value of @samp{$^} is something like @samp{src/foo.c
../headers/defs.h hack.h}, from which @samp{$(firstword $^)} extracts just
@samp{src/foo.c}.@refill

@subsection Directory Search and Implicit Rules

The search through the directories in @code{VPATH} happens also during
consideration of implicit rules (@pxref{Implicit}).

For example, when a file @file{foo.o} has no explicit rule, @code{make}
considers implicit rules, such as to compile @file{foo.c} if that file
exists.  If such a file is lacking in the current directory, the
directories in @code{VPATH} are searched for it.  If @file{foo.c} exists
(or is mentioned in the makefile) in any of the directories, the implicit
rule for C compilation is applicable.

The commands of all the built-in implicit rules normally use automatic
variables as a matter of necessity; consequently they will use the file
names found by directory search with no extra effort.

@node Phony Targets, Empty Targets, Directory Search, Rules
@section Phony Targets

A phony target is one that is not really the name of a file.
It is only a name for some commands to be executed when explicitly
requested.

If you write a rule whose commands will not create the target file, the
commands will be executed every time the target comes up for remaking.
Here is an example:

@example
clean:
        rm *.o temp
@end example

@noindent
Because the @code{rm} command does not create a file named @file{clean},
probably no such file will ever exist.  Therefore, the @code{rm} command
will be executed every time you say @samp{make clean}.

@findex .PHONY
The phony target will cease to work if anything ever does create a file
named @file{clean} in this directory.  Since there are no dependencies, the
@file{clean} would be considered up-to-date and its commands would not be
executed.  To avoid this problem, you can explicitly declare the target to
be phony, using the special target @code{.PHONY} (@pxref{Special Targets})
as follows:

@example
.PHONY : clean
@end example

@noindent
Once this is done, @code{make} will run the commands regardless of whether
there is a file named @file{clean}.

A phony target should not be a dependency of a real target file; strange
things can result from that.  As long as you don't do that, the phony
target commands will be executed only when the phony target is a goal
(@pxref{Goals}).

Phony targets can have dependencies.  When one directory contains multiple
programs, it is most convenient to describe all of the programs in one
makefile @file{./Makefile}.  Since the target remade by default will be the
first one in the makefile, it is common to make this a phony target named
@samp{all} and give it, as dependencies, all the individual programs.  For
example:

@example
all : prog1 prog2 prog3
.PHONY : all

prog1 : prog1.o utils.o
        cc -o prog1 prog1.o utils.o

prog2 : prog2.o
        cc -o prog2 prog2.o

prog3 : prog3.o sort.o utils.o
        cc -o prog3 prog3.o sort.o utils.o
@end example

@noindent
Now you can say @code{make} to remake all three programs, or specify
as arguments the ones to remake (as in @samp{make prog1 prog3}).

When one phony target is a dependency of another, it serves as a subroutine
of the other.  For example, here @samp{make cleanall} will delete the
object files, the difference files, and the file @file{program}:

@example
cleanall : cleanobj cleandiff
        rm program

cleanobj :
        rm *.o

cleandiff :
        rm *.diff
@end example

@node Empty Targets, Special Targets, Phony Targets, Rules
@section Empty Target Files to Record Events
@cindex empty target

The @dfn{empty target} is a variant of the phony target; it is used to hold
commands for an action that you request explicitly from time to time.
Unlike a phony target, this target file can really exist; but the file's
contents do not matter, and usually are empty.

The purpose of the empty target file is to record, with its
last-modification-time, when the rule's commands were last executed.  It
does so because one of the commands is a @code{touch} command to update the
target file.

The empty target file must have some dependencies.  When you ask to remake
the empty target, the commands are executed if any dependency is more
recent than the target; in other words, if a dependency has changed since
the last time you remade the target.  Here is an example:

@example
print: foo.c bar.c
        lpr -p $?
        touch print
@end example

@noindent
With this rule, @samp{make print} will execute the @code{lpr} command if
either source file has changed since the last @samp{make print}.  The
automatic variable @samp{$?} is used to print only those files that have
changed (@pxref{Automatic}).

@node Special Targets, Multiple Targets, Empty Targets, Rules
@section Special Built-in Target Names
@cindex special targets

Certain names have special meanings if they appear as targets.

@table @code
@item .PHONY
The dependencies of the special target @code{.PHONY} are considered to
be phony targets.  When it is time to consider such a target,
@code{make} will run its commands unconditionally, regardless of
whether a file with that name exists or what its date is.  @xref{Phony
Targets}.

@item .SUFFIXES
The dependencies of the special target @code{.SUFFIXES} are the list
of suffixes to be used in checking for suffix rules (@pxref{Suffix
Rules}).

@item .DEFAULT
The commands specified for @code{.DEFAULT} are used for any target for
which no other commands are known (either explicitly or through an
implicit rule).  If @code{.DEFAULT} commands are specified, every
nonexistent file mentioned as a dependency will have these commands
executed on its behalf.  @xref{Search Algorithm}.

@item .PRECIOUS
The targets which @code{.PRECIOUS} depends on are given this special
treatment: if @code{make} is killed or interrupted during the
execution of their commands, the target is not deleted.
@xref{Interrupts}.

@item .IGNORE
Simply by being mentioned as a target, @code{.IGNORE} says to ignore
errors in execution of commands.  The dependencies and commands for
@code{.IGNORE} are not meaningful.

@samp{.IGNORE} exists for historical compatibility.  Since
@code{.IGNORE} affects every command in the makefile, it is not very
useful; we recommend you use the more selective ways to ignore errors
in specific commands (@pxref{Errors}).

@item .SILENT
Simply by being mentioned as a target, @code{.SILENT} says not to
print commands before executing them.  The dependencies and commands
for @code{.SILENT} are not meaningful.

@samp{.SILENT} exists for historical compatibility.  We recommend you
use the more selective ways to silence specific commands
(@pxref{Echoing}).
@end table

An entire class of special targets have names made of the concatenation of
two implicit rule suffixes (two members of the list of dependencies of
@code{.SUFFIXES}).  Such special targets are suffix rules, an obsolete way
of defining implicit rules (but a way still widely used).  In principle,
any target name could be special in this way if you break it in two and add
both pieces to the suffix list.  In practice, suffixes normally begin with
@samp{.}, so these special target names also begin with @samp{.}.
@xref{Suffix Rules}.

@node Multiple Targets, Multiple Rules, Special Targets, Rules
@section Multiple Targets in a Rule

A rule with multiple targets is equivalent to writing many rules, each with
one target, and all identical aside from that.  This is useful in two cases.

@itemize @bullet
@item
You want just dependencies, no commands.  For example:

@example
kbd.o commands.o files.o: command.h
@end example

@noindent
gives an additional dependency to each of the three object files
mentioned.

@item
Identical commands work for all the targets.  The automatic variable
@samp{$@@} can be used to substitute the target to be remade into the
commands (@pxref{Automatic}).  For example:

@example
bigoutput littleoutput : text.g
        generate text.g -$(subst output,,$@@) > $@@
@end example

@noindent
is equivalent to

@example
bigoutput : text.g
        generate text.g -big > bigoutput
littleoutput : text.g
        generate text.g -little > littleoutput
@end example

@noindent
Here we assume the hypothetical program @code{generate} makes two
types of output, one if given @samp{-big} and one if given
@samp{-little}.@refill
@end itemize

@node Multiple Rules, Double-Colon, Multiple Targets, Rules
@section Multiple Rules for One Target

One file can be the target of several rules if at most one rule has commands.
The other rules can only have dependencies.  All the dependencies mentioned
in all the rules are merged into one list of dependencies for the target.
If the target is older than any dependency from any rule, the commands are
executed.

An extra rule with just dependencies can be used to give a few extra
dependencies to many files at once.  For example, one usually has a
variable named @code{objects} containing a list of all the compiler output
files in the system being made.  An easy way to say that all of them must
be recompiled if @file{config.h} changes is to write

@example
objects = foo.o bar.o
foo.o : defs.h
bar.o : defs.h test.h
$(objects) : config.h
@end example

This could be inserted or taken out without changing the rules that really
say how to make the object files, making it a convenient form to use if
you wish to add the additional dependency intermittently.

Another wrinkle is that the additional dependencies could be specified with
a variable that you could set with a command argument to @code{make}
(@pxref{Overriding}).  For example,

@example
extradeps=
$(objects) : $(extradeps)
@end example

@noindent
means that the command @samp{make extradeps=foo.h} will consider
@file{foo.h} as a dependency of each object file, but plain @samp{make}
will not.

If none of the explicit rules for a target has commands, then @code{make}
searches for an applicable implicit rule to find some commands.
@xref{Implicit}.

@node Double-Colon,, Multiple Rules, Rules
@section Double-Colon Rules
@cindex double-colon rule

@dfn{Double-colon} rules are rules written with @samp{::} instead of
@samp{:} after the target names.  They are handled differently from
ordinary rules when the same target appears in more than one rule.

When a target appears in multiple rules, all the rules must be the same
type: all ordinary, or all double-colon.  If they are double-colon, each of
them is independent of the others.  Each double-colon rule's commands are
executed if the target is older than any dependencies of that rule.  This
can result in executing none, any or all of the double-colon rules.

The double-colon rules for a target are executed in the order they appear
in the makefile.  However, the cases where double-colon rules really make
sense are those where the order of executing the commands would not matter.

Each double-colon rule should specify commands; if it does not, an
implicit rule will be used if one applies.  @xref{Implicit}.

@node Commands, Variables, Rules, Top
@chapter Writing the Commands in Rules
@cindex command (in rules)

The commands of a rule consist of shell command lines to be executed one by
one.  Each command line must start with a tab, except that the first
command line may be attached to the target-and-dependencies line with a
semicolon in between.  Blank lines and lines of just comments may appear
among the command lines; they are ignored.

Users use many different shell programs, but commands in makefiles are
always interpreted by @file{/bin/sh} unless the makefile specifies otherwise.

Whether comments can be written on command lines, and what syntax they use,
is under the control of the shell that is in use.  If it is @file{/bin/sh},
a @samp{#} at the start of a word starts a comment.

@menu
* Echoing::       Normally commands are echoed before execution,
                    but you can control this in several ways.
* Execution::     How commands are executed.
* Errors::	  What happens after an error in command execution.
		   How to ignore errors in certain commands.
* Interrupts::	  If a command is interrupted or killed,
		   the target may be deleted.
* Recursion::	  Invoking @code{make} from commands in makefiles.
* Sequences::     Defining canned sequences of commands.
@end menu

@node Echoing, Execution, Commands, Commands
@section Command Echoing

@cindex echoing (of commands)
@cindex silent operation
@cindex @@ (in commands)
@cindex -n
Normally @code{make} prints each command line before it is executed.  We
call this @dfn{echoing} because it gives the appearance that you are typing
the commands yourself.

When a line starts with @samp{@@}, it is normally not echoed.  The
@samp{@@} is discarded before the command is passed to the shell.  Typically
you would use this for a command whose only effect is to print something,
such as an @code{echo} command.

When @code{make} is given the flag @samp{-n}, echoing is all that happens,
no execution.  @xref{Options}.  In this case and only this case, even the
commands starting with @samp{@@} are printed.  This flag is useful for
finding out which commands @code{make} thinks are necessary without
actually doing them.

@cindex -s
@findex .SILENT
The @samp{-s} flag to @code{make} prevents all echoing, as if all commands
started with @samp{@@}.  A rule in the makefile for the special target
@code{.SILENT} has the same effect (@pxref{Special Targets}).
@code{.SILENT} is essentially obsolete since @samp{@@} is more
general.@refill

@node Execution, Errors, Echoing, Commands
@section Command Execution
@cindex execution
@cindex shell

When it is time to execute commands to update a target, they are executed
one at a time by making a new subshell for each line.  (In practice,
@code{make} may take shortcuts that do not affect the results.)

This implies that shell commands such as @code{cd} that set variables local
to each process will not affect the following command lines.  If you want
to use @code{cd} to affect the next command, put the two on a single line
with a semicolon between them.  Then @code{make} will consider them a
single command and pass them, together, to a shell which will execute them
in sequence.  For example:

@example
foo : bar/lose
        cd bar; gobble lose > ../foo
@end example

If you would like to split a single shell command into multiple lines of
text, you must use a backslash at the end of all but the last subline.
Such a sequence of lines is combined into a single line, by deleting the
backslash-newline sequences, before passing it to the shell.  Thus, the
following is equivalent to the preceding example:

@group
@example
foo : bar/lose
        cd bar;  \
        gobble lose > ../foo
@end example
@end group

@vindex SHELL
@vindex SHFLAGS
The program used as the shell is taken from the variable @code{SHELL}.  By
default, the program @file{/bin/sh} is used.

Unlike most variables, the variable @code{SHELL} will not be set from the
environment, except in a recursive @code{make}.  This is because the
environment variable @code{SHELL} is used to specify your personal choice
of shell program for interactive use.  It would be very bad for personal
choices like this to affect the functioning of makefiles.
@xref{Environment}.

The value of the variable @code{SHFLAGS} is used as additional command
arguments to give to the shell each time it is run.  By default, the
value is empty.  This variable also is not set from the environment
except in recursive use of @code{make}.

@node Errors, Interrupts, Execution, Commands
@section Errors in Commands

@cindex error (in commands)
After each shell command returns, @code{make} looks at its exit status.
If the command completed successfully, the next command line is executed in
a new shell, or after the last command line the rule is finished.

If there is an error (the exit status is nonzero), @code{make} gives up on
the current rule, and perhaps on all rules.

Sometimes it does not matter whether a command fails.  For example, you
may use the @code{mkdir} command to insure that a directory exists.  If
the directory already exists, @code{mkdir} will report an error, but you
probably want @code{make} to continue regardless.

@cindex - (in commands)
To ignore errors in a command line, write a @samp{-} at the beginning of
the line's text (after the initial tab).  The @samp{-} is discarded before
the command is passed to the shell for execution.

@cindex -i
@findex .IGNORE
When @code{make} is run with the @samp{-i} flag, errors are ignored in
all commands of all rules.  A rule in the makefile for the special target
@code{.IGNORE} has the same effect.  These ways of ignoring errors are
obsolete because @samp{-} is more general.

When errors are to be ignored, because of either a @samp{-} or the
@samp{-i} flag, @code{make} treats an error return just like success.

@cindex -k
When an error happens that @code{make} has not been told to ignore,
it implies that the current target cannot be correctly remade, and neither
can any other that depends on it either directly or indirectly.  No further
commands will be executed for these targets, since their preconditions
have not been achieved.

Normally @code{make} gives up immediately in this circumstance, returning a
nonzero status.  However, if the @samp{-k} flag is specified, @code{make}
continues to consider the other dependencies of the pending targets,
remaking them if necessary, before it gives up and returns nonzero status.
For example, after an error in compiling one object file, @samp{make -k}
will continue compiling other object files even though it already knows
that linking them will be impossible.  @xref{Options}.

The usual behavior assumes that your purpose is to get the specified
targets up to date; once @code{make} learns that this is impossible, it
might as well report the failure immediately.  @samp{-k} says that the real
purpose is to test as much as possible of the changes made in the program,
perhaps to find several independent problems so that you can correct them
all before the next attempt to compile.  This is why Emacs's @code{compile}
command passes the @samp{-k} flag by default.

@node Interrupts, Recursion, Errors, Commands
@section Interrupting or Killing @code{make}
@cindex interrupt
@cindex signal
@cindex deletion of target files

If @code{make} gets a fatal signal while a command is executing, it may
delete the target file that the command was supposed to update.  This is
done if the target file's date has changed since @code{make} first checked it.

The purpose of deleting the target is to make sure that it is remade from
scratch when @code{make} is next run.  Otherwise, a partially written file
could appear to be valid, since it is more recent than the dependencies.

@findex .PRECIOUS
You can prevent the deletion of a target file in this way by making the
special target @code{.PRECIOUS} depend on it.  Before remaking a target,
@code{make} checks to see whether it appears on the dependencies of
@code{.PRECIOUS}, and thereby decides whether the target should be
deleted if a signal happens.  Some reasons why you might do this are
that the target is updated in some atomic fashion or exists only to
record a date/time (its contents do not matter) or will cause trouble
if it ever fails to exist.

@node Recursion, Sequences, Interrupts, Commands
@section Recursive Use of @code{make}
@cindex recursion
@vindex MAKE

Recursive use of @code{make} means using @code{make} as a command in a
makefile.  This technique is useful when you want separate makefiles for
various subsystems that compose a larger system.  For example, suppose you
have a subdirectory @file{subdir} which has its own makefile, and you would
like the containing directory's makefile to run @code{make} on the
subdirectory.

You can do it by writing this:

@example
subsystem:
        cd subdir; $(MAKE)
@end example

@noindent
or, equivalently, this (@pxref{Options}):

@example
subsystem:
        $(MAKE) -c subdir
@end example

That's all you have to write in the makefile to cause the sub-@code{make}
to be run if you do @code{make subsystem}, but there are other things you
should know about how this does its job and how the sub-@code{make} relates
to the top-level @code{make}.

The commands above use the variable @code{MAKE}, whose value is the file
name with which @code{make} was invoked.  If this file name was
@file{/bin/make}, then the command executed is @samp{cd subdir; /bin/make}.
If you use a special version of @code{make} to run the top-level makefile,
the same special version will be executed for recursive invocations.  Also,
any arguments that define variable values are added to @code{MAKE}, so the
sub-@code{make} gets them too.  Thus, if you do @samp{make CFLAGS=-O}, so
that all C-compilations will be optimized, the sub-@code{make} is run with
@samp{cd subdir; /bin/make CFLAGS=-O}.

The flag options you give to the top-level @code{make} are passed down
to the sub-@code{make} automatically, through the variable @code{MAKEFLAGS}
as described below.

All the other variable values of the top-level @code{make} are passed to
the sub-@code{make} through the environment.  These variables are defined
in the sub-@code{make} as defaults, but do not override what is specified
in the sub-@code{make}'s makefile.  The way this works is that @code{make}
adds each variable and its value to the environment for running each
command.  (Variables whose names start with non-alphanumeric characters are
left out.)  The sub-@code{make}, in turn, uses the environment to
initialize its table of variable values.  @xref{Environment}.

@vindex MAKELEVEL
As a special feature, the variable @code{MAKELEVEL} is changed when it is
passed down from level to level.  This variable's value is a string which
is the depth of the level as a decimal number.  The value is @samp{0} for
the top-level @code{make}; @samp{1} for a sub-@code{make}, @samp{2} for a
sub-sub-@code{make}, and so on.  The incrementation happens when
@code{make} sets up the environment for a command.@refill

The main use of @code{MAKELEVEL} is to test it in a conditional directive
(@pxref{Conditionals}); this way you can write a makefile that behaves one
way if run recursively and another way if run directly by you.

@vindex MAKEFLAGS
Flags such as @samp{-s} and @samp{-k} are passed automatically to the
sub-@code{make} through the variable @code{MAKEFLAGS}.  This variable is
set up automatically by @code{make} to contain the flag letters that
@code{make} received.  Thus, if you do @samp{make -ks} then
@code{MAKEFLAGS} gets the value @samp{ks}.

As a consequence, every sub-@code{make} gets a value for @code{MAKEFLAGS}
in its environment.  In response, it takes the flags from that value and
processes them as if they had been given as arguments.  @xref{Options}.

The options @samp{-c}, @samp{-d}, @samp{-f}, @samp{-I}, @samp{-o}, and
@samp{-p} are not put into @code{MAKEFLAGS}; these options are not
passed down.@refill

If you don't want to pass the other the flags down, you must change the
value of @code{MAKEFLAGS}, like this:

@example
subsystem:
        cd subdir; $(MAKE) MAKEFLAGS=
@end example

@vindex MFLAGS
A similar variable @code{MFLAGS} exists also, for historical compatibility.
It has the same value as @code{MAKEFLAGS} except that a hyphen is added at
the beginning if it is not empty.  @code{MFLAGS} was traditionally used
explicitly in the recursive @code{make} command, like this:

@example
subsystem:
        cd subdir; $(MAKE) $(MFLAGS)
@end example

@noindent
but now @code{MAKEFLAGS} makes this usage redundant.

What about @samp{make -t}?  (@xref{Instead of Execution}.)  Following the
usual definition of @samp{-t}, this would create a file named
@file{subsystem}.  What you really want it to do is run @samp{cd subdir;
make -t}; but that would require executing the command, and @samp{-t} says
not to execute commands.@refill

The paradox is resolved by a special @code{make} feature: whenever a
command uses the variable @code{MAKE}, the flags @samp{-t}, @samp{-n} or
@samp{-q} do not apply to that rule.  The commands of that rule are
executed normally despite the presence of a flag that causes most
commands not to be run.  These flags are passed along via
@code{MAKEFLAGS}, so your request to touch the files, or print the
commands, is propagated to the subsystem.

@vindex MAKEFILES
If the environment variable @code{MAKEFILES} is defined, @code{make}
considers its value as a list of names (separated by whitespace) of
additional makefiles to be read before the others.  This works much like
the @code{include} directive: various directories are searched for those
files and the default goal is never taken from them.  @xref{Include}.  In
addition, it is not an error if the files listed in @code{MAKEFILES} are
not found.

The main use of @code{MAKEFILES} is with recursive invocation of @code{make}.
The outer @code{make} can set @code{MAKEFILES} to influence recursive
@code{make} levels.

@node Sequences,, Recursion, Commands
@section Defining Canned Command Sequences
@cindex sequences of commands

When the same sequence of commands is useful in making various targets, you
can define it as a canned sequence with the @code{define} directive, and
refer to the canned sequence from the rules for those targets.  The canned
sequence is actually a variable, so the name must not conflict with other
variable names.

Here is an example of defining a canned sequence of commands:

@example
define run-yacc
yacc $(firstword $^)
mv y.tab.c $@@
endef
@end example

@noindent
Here @code{run-yacc} is the name of the variable being defined;
@code{endef} marks the end of the definition; the lines in between are the
commands.  The @code{define} directive does not expand variable references
and function calls in the canned sequence; the @samp{$} characters,
parentheses, variable names, and so on, all become part of the value of the
variable you are defining.  @xref{Defining}, for a complete explanation of
@code{define}.

The first command in this example runs Yacc on the first dependency (of
whichever rule uses the canned sequence).  The output file from Yacc is
always named @file{y.tab.c}.  The second command moves the output to the
rule's target file name.

To use the canned sequence, substitute the variable into the commands of a
rule.  You can substitute it like any other variable (@pxref{Reference}).
But usually substitution alone is not enough, because the commands in a
canned sequence typically contain variable references that should be
expanded each time the canned sequence is used.  To make this work, you
need to use the @code{expand} function when you substitute the sequence
(@pxref{Expand Function}).  Here is how it looks:

@example
foo.c : foo.y
        $(expand $(run-yacc))
@end example

@noindent
The @code{expand} function will substitute @samp{foo.y} for the variable
@samp{$^} when it occurs in @code{run-yacc}'s value, and @samp{foo.c} for
@samp{$@@}.@refill

This is a realistic example, but this particular one is not needed in
practice because @code{make} has an implicit rule to figure out these
commands based on the file names involved.  @xref{Implicit}.

@node Variables, Conditionals, Commands, Top
@chapter How to Use Variables
@cindex variable
@cindex value

A @dfn{variable} is a name defined within @code{make} to represent a string
of text, called the variable's @dfn{value}.  These values can be
substituted by explicit request into targets, dependencies, commands and
other parts of the makefile.

Variables can represent lists of file names, options to pass to compilers,
programs to run, directories to look in for source files, directories to
write output in, or anything else you can imagine.

A variable name may be any sequence characters not containing @samp{:},
@samp{#}, @samp{=}, tab characters or leading or trailing spaces.  However,
variable names containing nonalphanumeric characters should be avoided, as
they may be given special meanings in the future.

It is traditional to use upper case letters in variable names, but we
recommend using lower case letters for variable names that serve internal
purposes in the makefile, and reserving upper case for parameters that
control implicit rules or for parameters that the user should override with
command options (@pxref{Overriding}).

@menu
* Reference::	How to use the value of a variable.
* Values::      All the ways variables get their values.
* Setting::	How to set a variable in the makefile.
* Override Directive:: Setting a variable in the makefile
		 even if the user has set it with a command argument.
* Defining::    An alternate way to set a variable to a verbatim string.
* Environment:: Variable values can come from the environment.
@end menu

@node Reference, Values, Variables, Variables
@section Reference to Variables
@cindex reference to variables
@cindex $

To substitute a variable's value, write a dollar sign followed by the name
of the variable in parentheses or braces: either @samp{$(foo)} or
@samp{$@{foo@}} is a valid reference to the variable @code{foo}.  This
special significance of @samp{$} is why you must write @samp{$$} to have
the effect of a single dollar sign in a file name or command.

Variable references can be used in any context: targets, dependencies,
commands, most directives, and new variable values.  Here is a common kind
of example, where a variable holds the names of all the object files in a
program:

@example
objects = program.o foo.o utils.o
program : $(objects)
        cc -o program $(objects)

$(objects) : defs.h
@end example

Variable references work by strict textual substitution.  Thus, the rule

@example
foo = c
prog.o : prog.c
        $(foo)$(foo) prog.c
@end example

@noindent
could be used to compile a C program @file{prog.c}.  (Since spaces around
the variable value are ignored in variable assignments, the value of
@code{foo} is precisely @samp{c}.)

A dollar sign followed by a character other than a dollar sign,
open-parenthesis or open-brace treats that single character as the variable
name.  Thus, you could reference the variable @code{x} with @samp{$x}.
However, this practice is strongly discouraged, except with the automatic
variables (@pxref{Automatic}).

@node Values, Setting, Reference, Variables
@section How Variables Get Their Values

Variables can get values in several different ways:

@itemize @bullet
@item
You can specify an overriding value when you run @code{make}.
@xref{Overriding}.

@item
You can specify a value in the makefile, either with an assignment
(@pxref{Setting}) or with a verbatim definition (@pxref{Defining}).

@item
Values are inherited from the environment.  @xref{Environment}.

@item
Several @dfn{automatic} variables are given new values for each rule.
@xref{Automatic}.

@item
Several variables have constant initial values.  @xref{Implicit
Variables}.
@end itemize

@node Setting, Override Directive, Values, Variables
@section Setting Variables
@cindex setting variables

To set a variable from the makefile, write a line starting with the
variable name followed by @samp{=}.  Whatever follows the @samp{=} on the
line becomes the value.  For example,

@example
objects = main.o foo.o bar.o utils.o
@end example

@noindent
defines a variable named @code{objects}.  Spaces around the variable name
are ignored, and so are spaces after the @samp{=} or at the end of the
line.

The line that sets the variable can contain variable references.  Such
references are replaced by their values before the new variable is set.
The value given to the new variable does not contain variable references;
it contains the substuted values.  Thus,

@example
x = foo
y = $(x) bar
@end example

@noindent
is equivalent to

@example
x = foo
y = foo bar
@end example

This gives you a way to introduce leading or trailing spaces into variable
values.  Such spaces are discarded from your input before substitution of
variable references and function calls; this means you can include leading
or trailing spaces in a variable value by protecting them with variable
references, like this:

@example
nullstring=
space=$(nullstring) $(nullstring)
@end example

@noindent
Here the value of the variable @code{space} is precisely one space.

There is no limit on the length of the value of a variable except the
amount of swapping space on the computer.  When a variable definition is
long, it is a good idea to break it into several lines by inserting
backslash-newline at convenient places in the definition.  This will not
affect the functioning of @code{make}, but it will make the makefile easier
to read.

Most variable names are considered to have the empty string as a value if
you have never set them.  Several variables have built-in initial values
that are not empty, but can be set by you in the usual ways
(@pxref{Implicit Variables}).  Several special variables are set
automatically to a new value for each rule; these are called the
@dfn{automatic} variables (@pxref{Automatic}).

@node Override Directive, Defining, Setting, Variables
@section The @code{override} Directive
@findex override

If a variable has been set with a command argument (@pxref{Overriding}),
then ordinary assignments in the makefile are ignored.  If you want to set
the variable in the makefile even though it was set with a command
argument, you can use an @code{override} directive, which is a line that
looks like this:

@example
override @var{variable} = @var{value}
@end example

The @code{override} directive was not invented for escalation in the war
between makefiles and command arguments.  It was invented so you can alter
and add to values that the user specifies with command arguments.

For example, suppose you always want the @samp{-g} switch when you run the
C compiler, but you would like to allow the user to specify the other
switches with a command argument just as usual.  You could use this
@code{override} directive:

@example
override CFLAGS = $(CFLAGS) -g
@end example

@node Defining, Environment, Override Directive, Variables
@section Defining Variables Verbatim
@findex define
@findex endef

Another way to set the value of a variable is to use the @code{define}
directive.  This directive has a different syntax and provides different
features, and its intended use is for defining canned sequences of commands
(@pxref{Sequences}).  But the variables made with @code{define} are just
like those made the usual way.  Only the variable's value matters.

The @code{define} directive is followed on the same line the name of the
variable and nothing more.  The value to give the variable appears on the
following lines.  These lines are used verbatim; the character @samp{$} is
not treated specially and whitespace is not changed.  The end of the value
is marked by a line containing just the word @code{endef}.

@example
define two-lines
echo foo
echo $100
endef
@end example

Aside from syntax, there are two differences between @code{define} and
ordinary variable assignments:

@itemize @bullet
@item
The value assigned in an ordinary variable assignment is scanned for
variable references and function calls, which are expanded.  The
@code{define} commands are used verbatim, with no replacement.

@item
The value in an ordinary assignment cannot contain a newline.  The text in
a @code{define} can be multiple lines; the newlines that separate the lines
become part of the variable's value.  (The final newline which is always
present does not become part of the variable value.)
@end itemize

@node Environment,, Defining, Variables
@section Variables from the Environment

@cindex environment
Variables in @code{make} can come from the environment with which
@code{make} is run.  Every environment variable that @code{make} sees when
it starts up is transformed into a @code{make} variable with the same name
and value.  But an explicit assignment in the makefile, or with a command
argument, overrides the environment.  (If the @samp{-e} flag is specified,
then values from the environment override assignments in the makefile.
@xref{Options}.)

By setting the variable @code{CFLAGS} in your environment, you can cause
all C compilations in most makefiles to use the compiler switches you
prefer.  This is safe for variables with standard or conventional meanings
because you know that no makefile will use them for other things.  (But
this is not totally reliable; some makefiles set @code{CFLAGS} explicitly
and therefore are not affected by the value in the environment.)

When @code{make} is invoked recursively, variables defined in the outer
invocation are automatically passed to inner invocations through the
environment (@pxref{Recursion}).  This is the main purpose of turning
environment variables into @code{make} variables, and it requires no
attention from you.

Other use of variables from the environment is not recommended.  It is not
wise for makefiles to depend for their functioning on environment variables
set up outside their control, since this would cause different users to get
different results from the same makefile.  This is against the whole
purpose of most makefiles.

Such problems would be especially likely with the variable @code{SHELL},
which is normally present in the environment to specify the user's choice
of interactive shell.  It would be very undesirable for this choice to
affect @code{make}.  So @code{make} ignores the environment value of
@code{SHELL} (and @code{SHFLAGS}) if the value of @code{MAKELEVEL} is zero
(which is normally true except in recursive invocations of
@code{make}).@refill

@node Conditionals, Functions, Variables, Top
@chapter Conditional Parts of Makefiles

@cindex conditionals
A @dfn{conditional} causes part of a makefile to be obeyed or ignored
depending on the values of variables.  Conditionals can compare the value
of one variable with another, or the value of a variable with a constant
string.

@menu
* Example: Conditional Example.   An annotated example.
* Syntax: Conditional Syntax.     Precise rules for syntax of conditionals.
* Flags: Testing Flags.           Conditionals testing flags such as @samp{-t}.
@end menu

@node Conditional Example, Conditional Syntax, Conditionals, Conditionals
@section Example of a Conditional

This conditional tells @code{make} to use one set of libraries if the
@code{CC} variable is @samp{gcc}, and a different set of libraries
otherwise.  It works by controlling which of two command lines will be used
as the command for a rule.  The result is that @samp{CC=gcc} as an argument
to @code{make} not only changes which compiler is used but also which
libraries are linked.

@example
libs_for_gcc = -lgnu
normal_libs =

foo: $(objects)
ifeq ($(CC),gcc)
        $(CC) -o foo $(objects) $(libs_for_gcc)
else
        $(CC) -o foo $(objects) $(normal_libs)
endif
@end example

@noindent
This conditional uses three directives: one @code{ifeq}, one @code{else}
and one @code{endif}.

The @code{ifeq} directive contains two arguments, separated by a comma and
surrounded by parentheses.  Variable substitution is performed on both
arguments and then they are compared.  The lines of the makefile following
the @code{ifeq} are obeyed if the two arguments match; otherwise they are
ignored.

The @code{else} directive causes the following lines to be obeyed if the
previous conditional failed.  In the example above, this means that the
second alternative linking command is used whenever the first alternative
is not used.  It is optional to have an @code{else} in a conditional.

The @code{endif} directive ends the conditional.  Every conditional must
end with an @code{endif}.  Unconditional makefile text follows.

When the variable @code{CC} has the value @samp{gcc}, the above example has
this effect:

@example
foo: $(objects)
        $(CC) -o foo $(objects) $(libs_for_gcc)
@end example

@noindent
When the variable @code{CC} has any other value, this effect is this:

@example
foo: $(objects)
        $(CC) -o foo $(objects) $(normal_libs)
@end example

Equivalent results can be obtained in another way by conditionalizing a
variable assignment and then using the variable unconditionally:

@example
libs_for_gcc = -lgnu
normal_libs =

ifeq ($(CC),gcc)
  libs=$(libs_for_gcc)
else
  libs=$(normal_libs)
endif

foo: $(objects)
        $(CC) -o foo $(objects) $(libs)
@end example

@node Conditional Syntax, Testing Flags, Conditional Example, Conditionals
@section Syntax of Conditionals
@findex ifdef
@findex ifeq
@findex else
@findex endif

The syntax of a simple conditional with no @code{else} is as follows:

@example
@var{conditional-directive}
@var{text-if-true}
endif
@end example

@noindent
The @var{text-if-true} may be any lines of text, to be considered as part
of the makefile if the condition is true.  If the condition is false, no
text is used instead.

The syntax of a complex conditional is as follows:

@example
@var{conditional-directive}
@var{text-if-true}
else
@var{text-if-false}
endif
@end example

@noindent
If the condition is true, @var{text-if-true} is used; otherwise,
@var{text-if-false} is used instead.  The @var{text-if-false} can be any
number of lines of text.

Conditionals work at the textual level.  The lines of the
@var{text-if-true} are read as part of the makefile if the condition is
true; if the condition is false, those lines are ignored completely.  It
follows that syntactic units of the makefile, such as rules, may safely be
split across the beginning or the end of the conditional.@refill

You may use an @code{include} directive within a conditional, but you may
not start a conditional in one file and end it in another.

The syntax of the @var{conditional-directive} is the same whether the
conditional is simple or complex.  There are four different directives that
test different conditions.  Here is a table of them:

@table @code
@item ifeq (@var{arg1}, @var{arg2})
Expand all variable references in @var{arg1} and @var{arg2} and
compare them.  If they are identical, the @var{text-if-true} is
effective; otherwise, the @var{text-if-false}, if any, is effective.

@item ifneq (@var{arg1}, @var{arg2})
Expand all variable references in @var{arg1} and @var{arg2} and
compare them.  If they are different, the @var{text-if-true} is
effective; otherwise, the @var{text-if-false}, if any, is effective.

@item ifdef @var{variable-name}
If the variable @var{variable-name} has a non-empty value, the
@var{text-if-true} is effective; otherwise, the @var{text-if-false},
if any, is effective.  Variables that have never been defined have an
empty value.

@item ifndef @var{variable-name}
If the variable @var{variable-name} has an empty value, the
@var{text-if-true} is effective; otherwise, the @var{text-if-false},
if any, is effective.
@end table

Extra spaces are allowed and ignored at the beginning of the conditional
directive line, but a tab is not allowed.  (If the line begins with a tab,
it will be considered a command for a rule.)  Aside from this, extra spaces
or tabs may be inserted with no effect anywhere except within the directive
name or within an argument.  A comment starting with @samp{#} may appear at
the end of the line.

The other two directives that play a part in a conditional are @code{else}
and @code{endif}.  Each of these directives is written as one word, with no
arguments.  Extra spaces are allowed and ignored at the beginning of the
line, and spaces or tabs at the end.  A comment starting with @samp{#} may
appear at the end of the line.

@node Testing Flags,, Conditional Syntax, Conditionals
@section Conditionals that Test Flags

You can write a conditional that tests @code{make} command flags such as
@samp{-t} by using the variable @code{MAKEFLAGS} together with the
@code{findstring} function.  This is useful when @code{touch} is not
enough to make a file appear up to date.

The @code{findstring} function determines whether one string appears as a
substring of another.  If you want to test for the @samp{-t} flag,
use @samp{t} as the first string and the value of @code{MAKEFLAGS} as
the other.

For example, here is how to arrange to use @samp{ranlib -t} to finish
marking an archive file up to date:

@example
archive.a: @dots{}
ifneq (,$(findstring t,$(MAKEFLAGS)))
        @@echo $(MAKE) > /dev/null
        touch archive.a
	ranlib -t archive.a
else
        ranlib archive.a
endif
@end example

@noindent
The @code{echo} command does nothing when executed; but its presence, with
a reference to the variable @code{MAKE}, marks the rule as ``recursive'' so
that its commands will be executed despite use of the @samp{-t} flag.

@node Functions, Running, Conditionals, Top
@chapter Functions for Transforming Text
@cindex function

@dfn{Functions} allow you to do text processing in the makefile to
compute the files to operate on or the commands to use.

@menu
* Syntax: Function Syntax.  Syntax of function calls in general.
* Text Functions::          Text manipulation functions.
* Expand Function::         The function @code{expand} can search a variable's
                             value for variable references.
* Filename Functions::      Functions for manipulating file names.
@end menu

@node Function Syntax, Text Functions, Functions, Functions
@section Function Call Syntax
@cindex $

A function call resembles a variable reference.  It looks like this:

@example
$(@var{function} @var{arguments})
@end example

@noindent
or like this:

@example
$@{@var{function} @var{arguments}@}
@end example

Here @var{function} is a function name; one of a short list of names that
are part of @code{make}.  There is no provision for defining new functions.

The @var{arguments} are the arguments of the function.  They are separated
from the function name by one or more spaces and/or tabs, and if there are
more than one argument they are separated by commas.  Such whitespace and
commas are not part of any argument's value.  Parentheses or braces,
whichever you use to surround the function call, can appear in an argument
only in matching pairs; the ones that were not used to surround the
function call can appear freely.  If the arguments contain other function
calls or variable references, it is wisest to surround them with the same
delimiters used for the containing function call.

The text written for each argument is processed by substitution of
variables and function calls in order to produce the argument value, which
is the text on which the function acts.

Commas and unmatched parentheses or braces cannot appear in the text of an
argument as written; leading spaces cannot appear in the text of the first
argument as written.  These characters can be put into the argument value
by variable substitution.  First define variables @code{comma} and
@code{space} whose values are isolated comma and space characters, then
substitute those variables where such characters are wanted, like this:

@example
comma= ,
space= $(empty) $(empty)
foo= a b c
bar= $(subst $(space),$(comma),$(foo))
@r{# bar is now `a,b,c'.}
@end example

@noindent
Here the @code{subst} function replaces each space with a comma, through
the value of @code{foo}, and substitutes the result.

@node Text Functions, Expand Function, Function Syntax, Functions
@section Functions for String Substitution and Analysis

Here are two functions that operate on substrings of a string:
@code{subst} and @code{findstring}.

@table @code
@item $(subst @var{from},@var{to},@var{text})
@findex subst
Performs a textual replacement on the text @var{text}: each occurrence
of @var{from} is replaced by @var{to}.  The result is substituted for
the function call.  For example,

@example
$(subst ee,EE,feet on the street)
@end example

substitutes the string @samp{fEEt on the strEEt}.

@item $(findstring @var{find},@var{in})
@findex findstring
Searches @var{in} for an occurrence of @var{find}.  If it occurs, the
value is @var{find}; otherwise, the value is empty.  You can use this
function in a conditional to test for the presence of a specific
substring in a given string.  @xref{Testing Flags}, for a practical
application of @code{findstring}.
@end table

Here is a realistic example of use of @code{subst}.  Suppose that a
makefile uses the @code{VPATH} variable to specify a list of directories
that @code{make} should search for dependency files.  This example shows
how to tell the C compiler to search for header files in the same list of
directories.

The value of @code{VPATH} is a list of directories separated by colons,
such as @samp{src:../headers}.  First, the @code{subst} function is used to
change the colons to spaces:

@example
$(subst :, ,$(VPATH))
@end example

@noindent
This produces @samp{src ../headers}.  Then another function,
@code{addprefix}, can turn each directory name into an @samp{-I} flag.
These can be added to the value of the variable @code{CFLAGS}, which is
passed automatically to the C compiler, like this:

@example
CFLAGS= $(CFLAGS) $(addprefix -I,$(subst :, ,$(VPATH)))
@end example

@noindent
The effect is to append the text @samp{-Isrc -I../headers} to the
previously given value of @code{CFLAGS}.

@node Expand Function, Filename Functions, Text Functions, Functions
@section Rescanning Text for Variable References

@table @code
@item $(expand @var{text})
@findex expand
Expands the text @var{text} twice.  This is to say, after variable
references and function calls written in @var{text} are expanded,
producing the argument text, that text is scanned over again for
variable references and function calls.
@end table

For an example, suppose the following variable values have been set up:

@example
foo=cross$$(intermediate)
intermediate=bar
@end example

@noindent
where the double @samp{$} is used in setting @code{foo} in order to prevent
variable substitution from occurring when @code{foo} is set, thus getting
an actual dollar sign into the value of @code{foo}.

Now if we expand @code{foo} in the usual way, with @samp{$(foo)}, the
result is @samp{cross$(intermediate)}.

The effect of @samp{$(expand $(foo))} is to rescan that string
@samp{cross$(intermediate)}, which appears as the expanded argument string.
In the process, variable substitution operates on the variable
@code{intermediate}.  The ultimate result is @samp{crossbar}.

The @code{expand} function is most often useful with canned sequences of
commands (@xref{Sequences}).

@node Filename Functions,, Expand Function, Functions
@section Functions for File Names

Several of the built-in expansion functions relate specifically to
taking apart file names or lists of file names.

Each of these functions performs a specific transformation on a file name.
The argument of the function is regarded as a series of file names,
separated by whitespace.  (Leading and trailing whitespace is ignored.)
Each file name in the series is transformed in the same way and the results
are concatenated with single spaces between them.

@table @code
@item $(dir @var{names})
@findex dir
Extracts the directory-part of each file name in @var{names}.  The
directory-part of the file name is everything up through (and
including) the last slash in it.  If the file name contains no slash,
the directory part is the string @samp{./}.  For example,

@example
$(dir src/foo.c hacks)
@end example

@noindent
produces the result @samp{src/ ./}.

@item $(notdir @var{names})
@findex notdir
Extracts all but the directory-part of each file name in @var{names}.
If the file name contains no slash, it is left unchanged.  Otherwise,
everything through the last slash is removed from it.  A file name
that ends with a slash becomes an empty string.  This is unfortunate,
because it means that the result does not always have the same number
of whitespace-separated file names as the argument had; but we do not
see any other valid alternative.

For example,

@example
$(notdir src/foo.c hacks)
@end example

@noindent
produces the result @samp{foo.c hacks}.

@item $(suffix @var{names})
@findex suffix
Extracts the suffix of each file name in @var{names}.  If the file name
contains a period, the suffix is everything starting with the last
period.  Otherwise, the suffix is the empty string.  This frequently
means that the result will be empty when @var{names} is not, and if
@var{names} contains multiple file names, the result may contain fewer
file names.

For example,

@example
$(notdir src/foo.c hacks)
@end example

@noindent
produces the result @samp{.c}.

@item $(basename @var{names})
@findex basename
Extracts all but the suffix of each file name in @var{names}.  If the
file name contains a period, the basename is everything starting up to
(and not including) the last period.  Otherwise, the basename is the
entire file name.  For example,

@example
$(basename src/foo.c hacks)
@end example

@noindent
produces the result @samp{src/foo hacks}.

@item $(addsuffix @var{suffix},@var{names})
@findex addsuffix
The argument @var{names} is regarded as a series of names, separated
by whitespace; @var{suffix} is used as a unit.  The value of
@var{suffix} is appended to the end of each individual name and the
resulting larger names are concatenated with single spaces between
them.  For example,

@example
$(addsuffix .c,foo bar)
@end example

@noindent
produces the result @samp{foo.c bar.c}.

@item $(addprefix @var{prefix},@var{names})
@findex addprefix
The argument @var{names} is regarded as a series of names, separated
by whitespace; @var{prefix} is used as a unit.  The value of
@var{prefix} is appended to the front of each individual name and the
resulting larger names are concatenated with single spaces between
them.  For example,

@example
$(addprefix src/,foo bar)
@end example

@noindent
produces the result @samp{src/foo src/bar}.

@item $(firstword @var{names})
@findex firstword
The argument @var{names} is regarded as a series of names, separated
by whitespace.  The value is the first name in the series.  The rest
of the names are ignored.  For example,

@example
$(firstword foo bar)
@end example

@noindent
produces the result @samp{foo}.

@item $(wildcard @var{pattern})
@findex wildcard
The argument @var{pattern} is a file name pattern, typically
containing wildcards characters.  The result of @code{wildcard} is a
space-separated list of the names of existing files that match the
pattern.

Wildcard are expanded automatically in rules (@pxref{Wildcards}).  But
it does not normally take place when a variable is set, or inside the
arguments of other functions.  Those occasions are when the
@code{wildcard} function is useful.
@end table

@node Running, Implicit, Functions, Top
@chapter How to Run @code{make}

A makefile that says how to recompile a program can be used in more than
one way.  The simplest use is to recompile every file that is out of date.
This is what @code{make} will do if run with no arguments.

But you might want to update only some of the files; you might want to use
a different compiler or different compiler options; you might want just to
find out which files are out of date without changing them.

By specifying arguments when you run @code{make}, you can do any of these
things or many others.

@menu
* Makefile Arguments::    Arguments to specify which makefile to use.

* Goals::                 Goal arguments specify which parts of the makefile
                           should be used.

* Avoid Compilation::     How to avoid recompiling certain files.

* Instead of Execution::  Mode flags specify what kind of thing to do
                           with the commands in the makefile
                           other than simply execute them.

* Overriding::            Overriding a variable can specify an alternate
                           compiler, or alternate flags for the compiler,
                           or whatever else you program into the makefile.

* Testing::               How to proceed past some errors, to test compilation.

* Options::               Summary of all options @code{make} accepts.
@end menu

@node Makefile Arguments, Goals, Running, Running
@section Arguments to Specify the Makefile

The way to specify the name of the makefile is with the @samp{-f} option.
For example, @samp{-f altmake} says to use the file @file{altmake} as
the makefile.

If you use the @samp{-f} flag several times (each time with a
following argument), all the specified files are used jointly as
makefiles.

If you do not use the @samp{-f} flag, the default is to use
@file{./makefile}, or, if that does not exist, @file{./Makefile}.
@xref{Makefiles}.@refill

@node Goals, Avoid Compilation, Makefile Arguments, Running
@section Goals
@cindex goal

The @dfn{goals} are the targets that @code{make} should strive ultimately
to update.  Other targets are updated as well if they appear as
dependencies of goals, or dependencies of dependencies of goals, etc.

By default, the goal is the first target in the makefile (not counting
targets that start with a period or that appear in included makefiles).
Therefore, makefiles are usually written so that the first target is for
compiling the entire program or programs they describe.

You can specify a different goal or goal with arguments to @code{make}.
Use the name of the goal as an argument.  If you specify several goals,
@code{make} processes each of them in turn, in the order you name them.

Any target in the makefile may be specified as a goal (unless it starts
with @samp{-} or contains an @samp{=}).  Even targets not in the makefile
may be specified, if @code{make} can find implicit rules that say how to
make them.

One use of specifying a goal is if you want to compile only a part of
the program, or only one of several programs.  Specify as a goal each
file that you wish to remake.  For example, consider a directory containing
a several programs, with a makefile that starts like this:

@example
all: size nm ld ar as
@end example

If you are working on the program @code{size}, you might want to say
@samp{make size} so that only the files of that program are recompiled.

Another use of specifying a goal is to make files that aren't normally
made.  For example, there may be a file of debugging output, or a version
of the program that is compiled specially for testing, which has a rule
in the makefile but isn't a dependency of the default goal.

Another use of specifying a goal is to run the commands associated with a
phony target (@pxref{Phony Targets}) or empty target (@pxref{Empty Targets}).
Many makefiles contain a phony target named @file{clean} which deletes
everything except source files.  Naturally, this is done only if you
request it explicitly with @samp{make clean}.  Here is a list of typical
phony and empty target names:

@table @file
@item clean
Delete all files that the makefile could remake.

@item install
Copy the executable file into a directory that users typically search for
commands.

@item print
Print listings of the source files that have changed.

@item tar
Create a tar file of the source files.
@end table

@node Avoid Compilation, Instead of Execution, Goals, Running
@section Avoiding Recompilation of Some Files

Sometimes you may have changed a source file but you don't want to
recompile all the files that depend on it.  For example, suppose you add a
macro or a declaration to a header file that many other files depend on.
Being conservative, @code{make} assumes that any change in the header file
requires recompilation of all dependent files, but you know that they don't
need to be recompiled and you would rather not waste the time waiting.

If you anticipate the problem before making the change, you can use the
@samp{-t} flag.  This flag tells @code{make} not to run the commands in the
rules, but rather to mark the target up-to-date by changing its
last-modification date.  You would follow this procedure:

@enumerate
@item
Use the command @samp{make} to recompile the source files that really
need recompilation.

@item
Make the changes in the header files.

@item
Use the command @samp{make -t} to mark all the object files as
up-to-date.  The next time you run @code{make}, the changes in the
header files will not cause any recompilation.
@end enumerate

If you have already changed the header file at a time when some files do
need recompilation, it is too late to do this.  Instead, you can use the
@samp{-o @var{file}} flag, which marks a specified file as ``old''
(@pxref{Options}).  This means that the file itself won't be remade,
and nothing else will be remade on its account.  Follow this procedure:

@enumerate
@item
Recompile the source files that need compilation for reasons independent
of the particular header file, with @samp{make -o @var{headerfile}}.
If several header files are involved, use a separate @samp{-o} option
for each header file.

@item
Touch all the object files with @samp{make -t}.
@end enumerate

@node Instead of Execution, Overriding, Avoid Compilation, Running
@section Instead of Executing the Commands
@cindex -t
@cindex touch
@cindex -q
@cindex -n

The makefile tells @code{make} how to tell whether a target is up to date,
and how to update each target.  But updating the targets is not always
what you want.  Certain options specify other activities for @code{make}.

@table @samp
@item -t
``Touch''.  The activity is to mark the targets as up to date without
actually changing them.  In other words, @code{make} pretends to compile
the targets but does not really change their contents.

@item -n
``No-op''.  The activity is to print what commands would be used to make
the targets up to date, but not actually execute them.

@item -q
``Question''.  The activity is to find out silently whether the targets
are up to date already; but execute no commands in either case.  In other
words, neither compilation nor output will occur.
@end table

With the @samp{-n} flag, @code{make} prints without execution the commands
that it would normally execute.

With the @samp{-t} flag, @code{make} ignores the commands in the rules
and uses (in effect) the command @code{touch} for each target that needs to
be remade.  The @code{touch} command is also printed, unless @samp{-s} or
@code{.SILENT} is used.  For speed, @code{make} does not actually invoke
the program @code{touch}.  It does the work directly.

With the @samp{-q} flag, @code{make} prints nothing and executes no
commands, but the exit status code it returns is zero if and only if the
targets to be considered are already up to date.

It is an error to use more than one of these three flags in the same
invocation of @code{make}.

@node Overriding, Testing, Instead of Execution, Running
@section Overriding Variables

You can override the value of a variable using an arguments to @code{make}
that contains a @samp{=}.  The argument @samp{@var{v}=@var{x}} sets the
value of the variable @var{v} to @var{x}.

Values specified this way override all values specified in the makefile
itself; once you have set a variable with a command argument, any ordinary
attempt in the makefile to change that variable is simply ignored.

One way to use this facility is to pass extra flags to compilers.
For example, in a properly written makefile, the variable @code{CFLAGS}
is included in each command that runs the C compiler, so a file
@file{foo.c} would be compiled like this:

@example
cc -c $(CFLAGS) foo.c
@end example

Thus, whatever value you set for @code{CFLAGS} affects each compilation
that occurs.  The makefile probably specifies the usual value for
@code{CFLAGS}, like this:

@example
CFLAGS=-g
@end example

Each time you run @code{make}, you can override this value and specify a
different value.  For example, if you say @samp{make CFLAGS='-g -O'}, each
C compilation will be done with @samp{cc -c -g -O}.  (This illustrates how
you can enclose spaces and other special characters in the value of a
variable when you override it.)

The variable @code{CFLAGS} is only one of many standard variables that
exist just so that you can change them this way.  @xref{Implicit
Variables}, for a complete list.

You can also program the makefile to look at additional variables of your
own, giving the user ability to control other aspects of how the makefile
works by changing the variables.

There is one way that the makefile can change a variable that you have
overridden.  This is to use the @code{override} directive, which is a line
that looks like this:

@example
override @var{variable} = @var{value}
@end example

@noindent
This line acts like an ordinary variable assignment except that it is
not ignored even if you have used a commadn option to set the variable.
@xref{Override Directive}.

@node Testing, Options, Overriding, Running
@section Testing the Compilation of a Program

Normally, when an error happens in executing a shell command, @code{make}
gives up immediately, returning a nonzero status.  No further commands are
executed for any target.  The error implies that the goal cannot be
correctly remade, so @code{make} reports this as soon as it knows.

When you are compiling a program that you have just changed, this is not
what you want.  Instead, you would rather that @code{make} try compiling
every file that can be tried, to show you all the compilation errors.

@cindex -k
Then you should use the @samp{-k} flag.  If the @samp{-k} flag is
specified, @code{make} continues to consider the other dependencies of the
pending targets, remaking them if necessary, before it gives up and returns
nonzero status.  For example, after an error in compiling one object file,
@samp{make -k} will continue compiling other object files even though it
already knows that linking them will be impossible.  @xref{Options}.

The usual behavior of @code{make} assumes that your purpose is to get the
goals up to date; once @code{make} learns that this is impossible, it might
as well report the failure immediately.  The @samp{-k} flag says that the
real purpose is to test as much as possible of the changes made in the
program, perhaps to find several independent problems so that you can
correct them all before the next attempt to compile.  This is why Emacs's
@code{compile} command passes the @samp{-k} flag by default.

@node Options,, Testing, Running
@section Summary of Options
@cindex options
@cindex flags

Here is a table of all the options @code{make} understands:

@table @samp
@item -b
This option is ignored for compatibility with other versions of
@code{make}.

@item -c @var{dir}
Change to directory @var{dir} before executing the rules.  If multiple
@samp{-c} options are specified, each is interpreted relative to the
previous one: @samp{-c / -c etc} is equivalent to @samp{-c /etc}.
This is typically used with recursive invocations of @code{make}
(@pxref{Recursion}).

@item -d
Print debugging information in addition to normal processing.  The
debugging information says which files are being considered for
remaking, which file-times are being compared and with what results,
which files actually need to be remade, which implicit rules are
considered and which are applied---everything interesting about how
@code{make} decides what to do.

@item -f @var{file}
Use file @var{file} as a makefile.  @xref{Makefiles}.

@item -i
Ignore all errors in commands executed to remake files.
@xref{Errors}.

@item -I @var{dir}
Specifies a directory @var{dir} to search for included makefiles.
@xref{Include}.  If several @samp{-I} options are used to specify
several directories, the directories are searched in the order
specified.

@item -k
Continue as much as possible after an error.  While the target that
failed, and those that depend on it, cannot be remade, the other
dependencies of these targets can be processed all the same.
@xref{Testing}.

@item -n
Print the commands that would be executed, but do not execute them.
@xref{Instead of Execution}.

@item -o @var{file}
Do not remake the file @var{file} even if it is older than its
dependencies, and do not remake anything on account of changes in
@var{file}.  Essentially the file is treated as very old and its rules
are ignored.  @xref{Avoid Compilation}.

@item -p
Print the data base (rules and variable values) that results from
reading the makefiles; then execute as usual or as otherwise
specified.

@item -q
``Question mode''.  Do not run any commands, or print anything; just
return an exit status that is zero if the specified targets are
already up to date, nonzero otherwise.  @xref{Instead of Execution}.

@item -r
Eliminate use of the built-in implicit rules (@pxref{Implicit}).
Also clear out the default list of suffixes for suffix rules
(@pxref{Suffix Rules}).

@item -s
Silent operation; do not print the commands as they are executed.
@xref{Echoing}.

@item -S
Cancel the effect of the @samp{-k} option.  This is never necessary
except in a recursive @code{make} where @samp{-k} might be inherited
from the top-level @code{make} via @code{MAKEFLAGS}.  @xref{Recursion}.

@item -t
Touch files (mark them up to date without really changing them)
instead of running their commands.  This is used to pretend (to fool
future invocations of @code{make}) that the commands were done.
@xref{Instead of Execution}.
@end table

@node Implicit, Archives, Running, Top
@chapter Using Implicit Rules
@cindex implicit rule

Certain standard ways of remaking target files are used very often.  For
example, one customary way to make an object file is from a C source file
using the C compiler, @code{cc}.

@dfn{Implicit rules} tell @code{make} how to use customary techniques so
that you don't have to specify them in detail when you want to use them.
For example, there is an implicit rule for C compilation.

Implicit rules work based on file names.  For example, C compilation typically
takes a @file{.c} file and makes a @file{.o} file.  So @code{make} applies
the implicit rule when it sees this combination of file-name endings.

A chain of implicit rules can apply in sequence; for example, @code{make}
will remake a @file{.o} file from a @file{.y} file by way of a @file{.c} file.
@iftex
@xref{Chained Rules}.
@end iftex

The built-in implicit rules use several variables in their commands so
that, by changing the values of the variables, you can change the way the
implicit rule works.  For example, the variable @code{CFLAGS} controls the
flags given to the C compiler by the implicit rule for C compilation.
@iftex
@xref{Implicit Variables}.
@end iftex

You can define your own implicit rules by writing @dfn{pattern rules}.
@iftex
@xref{Pattern Rules}.
@end iftex

@menu
* Using Implicit::       How to use an existing implicit rule
                          to get the commands for updating a file.

* Catalogue of Rules::   Catalogue of built-in implicit rules.

* Implicit Variables::   By changing certain variables, you can
                          change what the predefined implicit rules do.

* Chained Rules::        Using a chain of implicit rules.

* Pattern Rules::        Defining new implicit rules.

* Search Algorithm::     Precise algorithm for applying implicit rules.
@end menu

@node Using Implicit, Catalogue of Rules, Implicit, Implicit
@section Using Implicit Rules

To allow @code{make} to find a customary method for updating a target file,
all you have to do is refrain from specifying commands yourself.  Either
write a rule with no command lines, or don't write a rule at all.  Then
@code{make} will figure out which implicit rule to use based on which
kind of source file exists.

For example, suppose the makefile looks like this:

@example
foo : foo.o bar.o
        cc -o foo foo.o bar.o $(CFLAGS) $(LDFLAGS)
@end example

@noindent
Because you mention @file{foo.o} but do not give a rule for it, @code{make}
will automatically look for an implicit rule that tells how to update it.
This happens whether or not the file @file{foo.o} currently exists.

If an implicit rule is found, it supplies both commands and a dependency
(the source file).  You would want to write a rule for @file{foo.o} with no
command lines if you need to specify additional dependencies, such as
header files, that the implicit rule cannot supply.

Each implicit rule has a target pattern and dependency patterns.  There may
be many implicit rules with the same target pattern.  For example, numerous
rules make @samp{.o} files: one, from a @samp{.c} file with the C compiler;
another, from a @samp{.p} file with the Pascal compiler; and so on.  The rule
that actually applies is the one whose dependency exists or can be made.

So, if you have a file @file{foo.c}, @code{make} will run the C compiler;
otherwise, if you have a file @file{foo.p}, @code{make} will run the Pascal
compiler; and so on.

Of course, when you write the makefile, you know which implicit rule you
want @code{make} to use, and you know it will choose that one because you
know which other files are supposed to exist.  @xref{Catalogue of Rules},
for a catalogue of all the predefined implicit rules.

Above, we said an implicit rule applies if the required dependency ``exists
or can be made''.  A file ``can be made'' if it is mentioned explicitly in
the makefile as a target or a dependency, or if an implicit rule can be
recursively found for how to make it.  When the implicit dependency is the
result of another implicit rule, we say that @dfn{chaining} is occurring.
@xref{Chained Rules}.

In general, @code{make} searches for an implicit rule for each target, and
for each double-colon rule, that has no commands.  A file that is mentioned
only as a dependency is considered a target whose rule specifies nothing,
so implicit rule search happens for it.  @xref{Search Algorithm}, for the
details of how the search is done.

@node Catalogue of Rules, Implicit Variables, Using Implicit, Implicit
@section Catalogue of Implicit Rules

Here is a catalogue of predefined implicit rules which are always available
unless the makefile explicitly overrides or cancels them.
(@xref{Cancelling Rules}, for information on cancelling or overriding an
implicit rule.  The @samp{-r} option cancels all predefined rules.)

@table @asis
@item Compiling C programs
@file{@var{n}.o} will be made automatically from @file{@var{n}.c} 
with the command @samp{$(CC) -c $(CFLAGS)}.@refill

@item Compiling Pascal programs
@file{@var{n}.o} will be made automatically from @file{@var{n}.p}
with the command @samp{$(PC) -c $(PFLAGS)}.@refill

@item Compiling Fortran, EFL and Ratfor programs
@file{@var{n}.o} will be made automatically from @file{@var{n}.e},
@file{@var{n}.r}, @file{@var{n}.F} or @file{@var{n}.f} by running the
Fortran compiler.  The precise command used is as follows:@refill

@table @samp
@item .e
@samp{$(FC) -c $(EFLAGS)}.
@item .f
@samp{$(FC) -c $(FFLAGS)}.
@item .F
@samp{$(FC) -c $(FFLAGS)}.
@item .r
@samp{$(FC) -c $(RFLAGS)}.
@end table

@item Preprocessing Fortran, EFL and Ratfor programs
@file{@var{n}.f} will be made automatically from @file{@var{n}.e},
@file{@var{n}.r} or @file{@var{n}.F}.  This rule runs just the
preprocessor to convert a Ratfor, EFL or preprocessable Fortran
program into a strict Fortran program.  The precise command used is as
follows:@refill

@table @samp
@item .e
@samp{$(FC) -F $(EFLAGS)}.
@item .F
@samp{$(FC) -F $(FFLAGS)}.
@item .r
@samp{$(FC) -F $(RFLAGS)}.
@end table

@item Assembling assembler programs
@file{@var{n}.o} will be made automatically from @file{@var{n}.s} by
running the assembler @code{as}.  The precise command used is
@samp{$(AS) $(ASFLAGS)}.@refill

@item Linking a single object file
@file{@var{n}} will be made automatically from @file{@var{n}.o} by
running the linker @code{ld} via the C compiler.  The precise command
used is @samp{$(CC) $(LDFLAGS) @dots{} $(LOADLIBES)}.@refill

This rule does the right thing for a simple program with only one source
file.  In more complicated cases, you must write an explicit command for
linking.

@item Compiling C into assembler code
@file{@var{n}.s} will be made automatically from @file{@var{n}.c}
with the command @samp{$(CC) -S $(CFLAGS)}.@refill

It would be possible for @code{make} to convert @file{@var{n}.c} into
@file{@var{n}.o} by way of @file{@var{n}.s}, using this rule and the
rule for running the assembler.  But that is not what @code{make}
does, because the rule for compiling @file{@var{n}.c} into
@file{@var{n}.o} directly comes earlier in the order of rules.  The
upshot is that the file @file{@var{n}.s} is not created or changed
when @file{@var{n}.o} is being remade.  This rule is used only if you
explicitly specify @file{@var{n}.s} as a goal or needed dependency.

This is a deliberate decision, for the sake of compatibility with Unix
@code{make}.@refill

If you want @code{make} update @file{@var{n}.s} on the way to updating
@file{@var{n}.o}, you can request this by cancelling the other rule
that allows direct compilation.  @xref{Cancelling Rules}.@refill

@item Compiling Pascal, Fortran, EFL or Ratfor into assembler code
@file{@var{n}.s} will be made automatically from @file{@var{n}.p},
@file{@var{n}.e}, @file{@var{n}.r}, @file{@var{n}.F} or @file{@var{n}.F}
by running the appropriate compiler with the @samp{-S} flag
instead of the @samp{-c} flag.@refill

For compatibility with Unix @code{make}, these rules apply only if you
expressly request @code{make} to update @file{@var{n}.s}.  See the
information immediately above.

@item Yacc for C programs
@file{@var{n}.c} will be made automatically from @file{@var{n}.y} by
running Yacc with the command @samp{$(YACC) $(YFLAGS)}.

@item Yacc for Ratfor programs
@file{@var{n}.r} will be made automatically from @file{@var{n}.yr} by
running Yacc with the command @samp{$(YACCR) $(YFLAGS)}.

@item Yacc for EFL programs
@file{@var{n}.e} will be made automatically from @file{@var{n}.ye} by
running Yacc with the command @samp{$(YACCE) $(YFLAGS)}.

@item Lex for C programs
@file{@var{n}.c} will be made automatically from @file{@var{n}.l} by
by running Lex.  The actual command is @samp{$(LEX) $(LFLAGS)}.

@item Lex for Ratfor programs
@file{@var{n}.r} will be made automatically from @file{@var{n}.l} by
by running Lex.  The actual command is @samp{$(LEX) $(LFLAGS)}.

The traditional custom of using the same suffix @samp{.l} for all Lex
files regardless of whether they produce C code or Ratfor code makes
it impossible for @code{make} to determine autmatically which of the
two languages you are using in any particular case.  If @code{make} is
called upon to remake an object file from a @samp{.l} file, it must
guess which compiler to use.  It will guess the C compiler, because
that is more common.  If you are using Ratfor, make sure @code{make}
knows this by mentioning @file{@var{n}.r} in the makefile.

@item RCS
Any file @file{@var{n}} will be extracted if necessary from an RCS
file named either @file{@var{n},v} or @file{RCS/@var{n},v}.  The
precise command used is @samp{$(CO) $(COFLAGS)}.  The variable
@code{CO} has default value @samp{co}.

@item SCCS
Any file @file{@var{n}} will be extracted if necessary from an SCCS
file named either @file{s.@var{n}} or @file{SCCS/s.@var{n}}.  The
precise command used is @samp{$(GET) $(GFLAGS)}.

I recommend that you avoid the use of SCCS.  RCS is widely held to be
superior, and RCS is also free.  By choosing free software in place of
comparable proprietary software, you support the free software
movement.
@end table

@node Implicit Variables, Chained Rules, Catalogue of Rules, Implicit
@section Variables Used by Implicit Rules
@cindex flags for compilers

The commands in built-in implicit rules make liberal use of certain
predefined variables.  You can redefine these variables, either in the
makefile or with arguments to @code{make}, to alter how the implicit rules
work without actually redefining them.

For example, the command used to compile a C source file actually says
@samp{$(CC) -c $(CFLAGS)}.  The default values of the variables used
are @samp{cc} and nothing, resulting in the command @samp{cc -c}.  By
redefining @samp{$(CC)} to @samp{ncc}, you could cause @samp{ncc} to
be used for all C compilations performed by the implicit rule.  By
redefining @samp{$(CFLAGS)} to be @samp{-g}, you could pass the
@samp{-g} option to each compilation.  @emph{All} implicit rules that
do C compilation use @samp{$(CC)} to get the program name for the
compiler and @emph{all} include @samp{$(CFLAGS)} among the arguments
given to the compiler.@refill

The variables used in implicit rules fall into two classes: those that are
names of programs (like @code{CC}) and those that contain arguments for the
programs (like @code{CFLAGS}).  (The ``name of a program'' may also contain
some command arguments, but it must start with an actual executable program
name.)  If a variable value contains more than one argument, separate them
with spaces.

Here is a table of variables used as names of programs:

@table @code
@item AS
@vindex AS
Program for doing assembly; default @samp{as}.

@item CC
@vindex CC
Program for compiling C programs; default @samp{cc}.

@item CO
@vindex CO
Program for extracting a file from RCS; default @samp{co}.

@item FC
@vindex FC
Program for compiling or preprocessing Fortran programs (or Ratfor or
EFL programs); default @samp{f77}.

@item GET
@vindex GET
Program for extracting a file from SCCS; default @samp{get}.

@item LEX
@vindex LEX
Program to use to turn Lex grammars into C programs or Ratfor programs;
default @samp{lex}.

@item PC
@vindex PC
Program for compiling Pascal programs; default @samp{pc}.

@item YACC
@vindex YACC
Program to use to turn Yacc grammars into C programs; default
@samp{yacc}.

@item YACCR
@vindex YACCR
Program to use to turn Yacc grammars into Ratfor programs; default
@samp{yacc -r}.

@item YACCE
@vindex YACCE
Program to use to turn Yacc grammars into EFL programs; default
@samp{yacc -e}.

@item RANLIB
@vindex RANLIB
Program to use to update the symbol-directory of an archive
(the @file{__.SYMDEF} member); default @samp{ranlib}.
@end table

Here is a table of variables whose values are additional arguments for the
programs above:

@table @code
@item ASFLAGS
@vindex ASFLAGS
Extra flags to give to the assembler (when explicitly invoked
on a @samp{.s} file).

@item CFLAGS
@vindex CFLAGS
Extra flags to give to the C compiler; default is empty.

@item EFLAGS
@vindex EFLAGS
Extra flags to give to the Fortran compiler for EFL programs; default
is empty.

@item FFLAGS
@vindex FFLAGS
Extra flags to give to the Fortran compiler; default is empty.

@item LFLAGS
@vindex LFLAGS
Extra flags to give to Lex; default is empty.

@item LDFLAGS
@vindex LDFLAGS
Extra flags to give to compilers when they are supposed to invoke the
linker, @samp{ld}; default is empty.

@item PFLAGS
@vindex PFLAGS
Extra flags to give to the Pascal compiler; default is empty.

@item RFLAGS
@vindex RFLAGS
Extra flags to give to the Fortran compiler for Ratfor programs;
default is empty.

@item YFLAGS
@vindex YFLAGS
Extra flags to give to Yacc; default is empty.
@end table

@node Chained Rules, Pattern Rules, Implicit Variables, Implicit
@section Chains of Implicit Rules

@cindex chains of rules
Sometimes a file can be made by a sequence of implicit rules.  For example,
a file @file{@var{n}.o} could be made from @file{@var{n}.y} by running
first Yacc and then @code{cc}.  Such a sequence is called a @dfn{chain}.

If the file @file{@var{n}.c} exists, or is mentioned in the makefile, no
special searching is required: @code{make} finds that the object file can
be made by C compilation from @file{@var{n}.c}; later on, when considering
how to make @file{@var{n}.c}, the rule for running Yacc will be
used.  Ultimately both @file{@var{n}.c} and @file{@var{n}.o} are
updated.@refill

@cindex intermediate file
However, even if @file{@var{n}.c} does not exist and is not mentioned,
@code{make} knows how to envision it as the missing link between
@file{@var{n}.o} and @file{@var{n}.y}!  In this case, @file{@var{n}.c} is
called an @dfn{intermediate file}.  Once @code{make} has decided to use the
intermediate file, it is entered in the data base as if it had been
mentioned in the makefile, along with the implicit rule that says how to
create it.@refill

Intermediate files are remade using their rules just like all other
files.  The difference is that the intermediate file is deleted when
@code{make} is finished.  Therefore, the intermediate file which did
not exist before @code{make} also does not exist after @code{make}.
The deletion is reported to you by printing a @code{rm -f} command
that shows what @code{make} is doing.  (You can optionally define an
implicit rule so as to preserve certain intermediate files.)

A chain can involve more than two implicit rules.  For example, it is
possible to make a file @file{foo} from @file{RCS/foo.y,v} by running RCS,
Yacc and @code{cc}.  Then both @file{foo.y} and @file{foo.c} are
intermediate files that are deleted at the end.@refill

No single implicit rule can appear more than once in a chain.  This means
that @code{make} will not even consider such a ridiculous thing as making
@file{foo} from @file{foo.o.o} by running the linker twice.  This
constraint has the added benefit of preventing any infinite loop in the
search for an implicit rule chain.

There are some special implicit rules to optimize certain cases that would
otherwise by handled by rule chains.  For example, making @file{foo} from
@file{foo.c} could be handled by compiling and linking with separate rules,
using @file{foo.o} as an intermediate file.  But what actually happens is
that a special rule for this case does the compilation and linking with a
single @code{cc} command.  The optimized rule is used in preference to the
step-by-step chain because it comes earlier in the ordering of rules.

@node Pattern Rules, Last Resort, Chained Rules, Implicit
@section Defining and Redefining Pattern Rules

@cindex pattern rule
You define an implicit rule by writing a @dfn{pattern rule}.  A pattern
rule looks like an ordinary rule, except that its target contains the
character @samp{%} (exactly one of them).  The target is considered a
pattern for matching file names; the @samp{%} can match any substring,
while other characters match only themselves.

For example, @samp{%.c} as a pattern matches any file name that ends in
@samp{.c}.  @samp{s.%.c} as a pattern matches any file name that starts
with @samp{s.}, ends in @samp{.c} and is at least five characters long.
(There must be at least one character to match the @samp{%}.)  The substring
that the @samp{%} matches is called the @dfn{stem}.@refill

A pattern rule must have at least one dependency that uses @samp{%}.
@samp{%} in a dependency of a pattern rule stands for the same stem
that was matched by the @samp{%} in the target.  In order for
the pattern rule to apply, its target pattern must match the file name
under consideration, and its dependency patterns must name files that
exist or can be made.  These files become dependencies of the target.

There may also be dependencies that do not use @samp{%}; such a dependency
attaches to every file made by this pattern rule.  These unvarying
dependencies are rarely useful.

The order in which pattern rules appear in the makefile is important
because the rules are considered in that order.  Of equally applicable
rules, the first one found is used.  The rules you write take precedence
over those that are built in.  Note, however, that a rule whose
dependencies actually exist or are mentioned always takes priority over a
rule with dependencies that must be made by chaining other implicit rules.

@menu
* Examples: Pattern Examples.  Real examples of pattern rule definitions.

* Vars: Automatic.             The automatic variables enable the commands
                                in pattern rules to act on the right files.

* Matching: Pattern Match.     Details of how patterns match.

* Match-Anything Rules::       Precautions in defining a rules that can
                                match any target file whatever.

* Cancelling Rules::           Overriding or cancelling built-in rules.

* Last Resort::                How to define a last-resort rule
                                that applies to any target that no other
                                rule applies to.

* Suffix Rules::               The old-fashioned way to define implicit rules.
@end menu

@node Pattern Examples, Automatic, Pattern Rules, Pattern Rules
@subsection Pattern Rule Examples

Here are some examples of pattern rules actually predefined in
@code{make}.  First, the rule that compiles @samp{.c} files into @samp{.o}
files:@refill

@example
%.o : %.c
        $(CC) -c $(CFLAGS) $< -o $@@
@end example

@noindent
defines a rule that can make any file @file{@var{x}.o} from
@file{@var{x}.c}.  The command uses the automatic variables @samp{$@@} and
@samp{$<} to substitute the names of the target file and the source file
as they are in each case where the rule apply (@pxref{Automatic}).@refill

Here is a second built-in rule:

@example
% :: RCS/%,v
        $(CO) $(COFLAGS) $<
@end example

@noindent
defines a rule that can make any file @file{@var{x}} whatever from a
corresponding file @file{@var{x},v} in the subdirectory @file{RCS}.  Since
the target is @samp{%}, this rule will apply to any file whatever, provided
the appropriate dependency file exists.  The double colon makes the rule
@dfn{terminal}, which means that its dependency may not be an intermediate
file (@pxref{Match-Anything Rules}).@refill

@node Automatic, Pattern Match, Pattern Examples, Pattern Rules
@subsection Automatic Variables
@cindex automatic variables
@cindex $

Suppose you are writing a pattern rule to compile a @samp{.c} file into a
@samp{.o} file: how do you write the @samp{cc} command so that it operates
on the right source file name?  You can't write the name in the command,
because the name is different each time the implicit rule is applied.

What you do is use a special feature of @code{make}, the @dfn{automatic
variables}.  These variables have values computed afresh for each rule that
is executed, based on the target and dependencies of the rule.  In this
example, you would use @samp{$@@} for the object file name and @samp{$<}
for the source file name.

Here is a table of automatic variables:

@table @code
@item $@@
The file name of the target of the rule.  If the target is an archive
member, then @samp{$@@} is the name of the archive file.

@item $%
The target member name, when the target is an archive member.  For
example, if the target is @file{foo.a(bar.o)} then @samp{$%} is
@file{bar.o} and @samp{$@@} is @file{foo.a}.  @samp{$%} is empty
when the target is not an archive member.

@item $<
The name of the first implicit dependency, when an implicit rule is
being applied.

@item $?
The names of all the dependencies that are newer than the target, with
spaces between them.

@item $^
The names of all the dependencies, with spaces between them.

@item $*
The stem with which an implicit rule matches (@pxref{Pattern Match}).
If the target is @file{dir/a.foo.b} and the target pattern is
@file{a.%.b} then the stem is @file{dir/foo}.  The stem is useful for
constructing names of related files.@refill

@item $($@@)
The same as @code{$@@}, for compatibility with some other versions
of @code{make}.
@end table

@samp{$?} is useful even in explicit rules when you wish to operate on only
the dependencies that have changed.  For example, suppose that an archive
named @file{lib} is supposed to contain copies of several object files.
This rule copies just the changed object files into the archive:

@example
lib: foo.o bar.o lose.o win.o
        ar c lib $?
@end example

Of the variables listed above, four have values that are single file names.
These four have variants that get just the file's directory name or just
the file name within the directory.  The variant variables' names are
formed by appending @samp{D} or @samp{F}, respectively.  These variants are
semi-obsolete in GNU @code{make} since the functions @code{dir} and
@code{notdir} can be used to get an equivalent effect (@pxref{Filename
Functions}).  Here is a table of the variants:@refill

@table @samp
@item $(@@D)
The directory part of the file name of the target.  If the value of
@samp{$@@} is @file{dir/foo.o} then @samp{$(@@D)} is @file{dir/}.
This value is @file{./} if @samp{$@@} does not contain a slash.
@samp{$(@@D)} is equivalent to @samp{$(dir $@@)}.@refill

@item $(@@F)
The file-within-directory part of the file name of the target.  If the
value of @samp{$@@} is @file{dir/foo.o} then @samp{$(@@F)} is
@file{foo.o}.  @samp{$(@@F)} is equivalent to @samp{$(notdir $@@)}.

@item $($/)
The same as @code{$(@@F)}, for compatibility with some other versions
of @code{make}.

@item $(%D)
@itemx $(%F)
The directory part and the file-within-directory part of the archive
member name.

@item $(*D)
@itemx $(*F)
The directory part and the file-within-directory part of the stem;
@file{dir/} in this example.

@item $(<D)
@itemx $(<F)
The directory part and the file-within-directory part of the first
implicit dependency.
@end table

@node Pattern Match, Match-Anything Rules, Automatic, Pattern Rules
@subsection How Patterns Match

@cindex stem
A target pattern is composed of a @samp{%} between a prefix and a suffix,
either of which may be empty.  The pattern matches a file name only if the
file name starts with the prefix and ends with the suffix, without overlap.
The text between the prefix and the suffix is called the @dfn{stem}.  Thus,
when the pattern @samp{%.o} matches the file name @file{test.o}, the stem
is @samp{test}.  The pattern rule dependencies are turned into actual file
names by substituting the stem for the character @samp{%}.  Thus, if in the
same example one of the dependencies is written as @samp{%.c}, it expands
to @samp{test.c}.@refill

When the target pattern does not contain a slash (and usually it does not),
directory names in the file names are removed from the file name before it
is compared with the target prefix and suffix.  The directory names, along
with the slash that ends them, are added back to the stem.  Thus,
@samp{e%t} does match the file name @file{src/eat}, with @samp{src/a} as
the stem.  When dependencies are turned into file names, the directories
from the stem are added at the front, while the rest of the stem is
substituted for the @samp{%}.  The stem @samp{src/a} with a dependency
pattern @samp{c%r} gives the file name @file{src/car}.@refill

@node Match-Anything Rules, Cancelling Rules, Pattern Match, Pattern Rules
@subsection Match-Anything Pattern Rules

@cindex match-anything rule
@cindex terminal rule
When a pattern rule's target is just @samp{%}, it matches any filename
whatever.  We call these rules @dfn{match-anything} rules.  They are very
useful, but it can take a lot of time for @code{make} to think about them,
because it must consider every such rule for each file name listed either
as a target or as a dependency.

Suppose the makefile mentions @file{foo.c}.  For this target, @code{make}
would have to consider making it by linking an object file @file{foo.c.o},
or by C compilation-and-linking in one step from @file{foo.c.c}, or by
Pascal compilation-and-linking from @file{foo.c.p}, and many other
possibilities.  We know these possibilities are ridiculous since
@file{foo.c} is a C source file, not an executable.@refill

If @code{make} did consider these possibilities, it would ultimately reject
them, because files such as @file{foo.c.o}, @file{foo.c.p}, etc. would not
exist.  But these possibilities are so numerous that @code{make} would run
very slowly if it had to consider them.@refill

To gain speed, we have put various constraints on the way @code{make}
considers match-anything rules.  There are two different constraints that
can be applied, and each time you define a match-anything rule you must
choose one or the other for that rule.

One choice is to mark the match-anything rule as @dfn{terminal} by defining
it with a double colon.  When a rule is terminal, it does not apply
unless its dependencies actually exist.  Dependencies that could be made
with other implicit rules are not good enough.

For example, the built-in implicit rules for extracting sources from RCS
and SCCS files are terminal; as a result, if the file @file{foo.c,v} does
not exist, @code{make} will not even consider trying to make it as an
intermediate file from @file{foo.c,v.o} or from @file{RCS/SCCS/s.foo.c,v}.
RCS and SCCS files are generally ultimate source files, which should not be
remade from any other files; therefore, @code{make} can save time by not
looking for ways to remake them.@refill

If you do not mark the match-anything rule as terminal, then it is
nonterminal.  A nonterminal match-anything rule cannot apply to a file name
that indicates a specific type of data.  A file name indicates a specific
type of data if some non-match-anything implicit rule target matches it.

For example, the file name @file{foo.c} matches the target for the pattern
rule @samp{%.c : %.y} (the rule to run Yacc).  Regardless of whether this
rule is actually applicable (which happens only if there is a file
@file{foo.y}), the fact that its target matches is enough to prevent
consideration of any nonterminal match-everything rules for the file
@file{foo.c}.  Thus, @code{make} will not even consider trying to make
@file{foo.c} as an executable file from @file{foo.c.o}, @file{foo.c.c},
@file{foo.c.p}, etc.@refill

The motivation for this constraint is that nonterminal match-everything
rules are used for making files containing specific types of data (such as
executable files) and a file name with a recognized suffix indicates a
specific different type of data (such as a C source file).

Special built-in dummy pattern rules are provided solely to recognize
certain file names so that nonterminal match-everything rules won't be
considered.  These dummy rules have no dependencies and no commands, and
they are ignored for all other purposes.  For example, the built-in
implicit rule

@example
%.p :
@end example

@noindent
exists to make sure that Pascal source files such as @file{foo.p} match a
specific target pattern and thereby prevent time from being wasted looking
for @file{foo.p.o} or @file{foo.p.c}.

@node Cancelling Rules,, Match-Anything Rules, Pattern Rules
@subsection Cancelling Implicit Rules

You can override a built-in implicit rule by defining a new pattern rule
with the same target and dependencies, but different commands.  When the
new rule is defined, the built-in one is replaced.  The new rule's position
in the sequence of implicit rules is determined by where you write the new
rule.

You can cancel a built-in implicit rule by defining a pattern rule with the
same target and dependencies, but no commands.  For example, the following
would cancel the rule that runs the assembler:

@example
%.o : %.s
@end example

@node Last Resort, Suffix Rules, Pattern Rules, Implicit
@section Defining Last-Resort Default Rules

@findex .DEFAULT
You can define a last-resort implicit rule by writing a rule for the target
@code{.DEFAULT}.  Such a rule's commands are used for all targets and
dependencies that have no commands of their own and for which no other
implicit rule applies.  Naturally, there is no @code{.DEFAULT} rule unless
you write one.

For example, when testing a makefile, you might not care if the source
files contain real data, only that they exist.  Then you might do this:

@example
.DEFAULT:
        touch $@@
@end example

@noindent
to cause all the source files needed (as dependencies) to be created
silently.

@node Suffix Rules, Search Algorithm, Last Resort, Implicit
@section Old-Fashioned Suffix Rules
@cindex suffix rules

@dfn{Suffix rules} are the old-fashioned way of defining implicit rules for
@code{make}.  Suffix rules are obsolete because pattern rules are more
general and clearer.  They are supported in GNU @code{make} for
compatibility with old makefiles.  They come in two kinds:
@dfn{double-suffix} and @dfn{single-suffix}.@refill

A double-suffix rule is defined by a pair of suffixes: the target suffix
and the source suffix.  It matches any file whose name ends with the target
suffix.  The corresponding implicit dependency is to the file name made by
replacing the target suffix with the source suffix.  A two-suffix rule
whose target and source suffixes are @samp{.o} and @samp{.c} is equivalent
to the pattern rule @samp{%.o : %.c}.

A single-suffix rule is defined by a single suffix, which is the source
suffix.  It matches any file name, and the corresponding implicit
dependency name is made by appending the source suffix.  A single-suffix
rule whose source suffix is @samp{.c} is equivalent to the pattern rule
@samp{% : %.c}.

Suffix rule definitions are recognized by comparing each rule's target
against a defined list of known suffixes.  When @code{make} sees a rule
whose target is a known suffix, this rule is considered a single-suffix
rule.  When @code{make} sees a rule whose target is two known suffixes
concatenated, this rule is taken as a double-suffix rule.

For example, @samp{.c} and @samp{.o} are both on the default list of known
suffixes.  Therefore, if you define a rule whose target is @samp{.c.o},
@code{make} takes it to be a double-suffix rule with source suffix
@samp{.c} and target suffix @samp{.o}.  For example, here is the old
fashioned way to define the rule for compiling a C source:@refill

@example
.c.o:
        $(CC) -c $(CFLAGS) -o $@@ $<
@end example

@findex .SUFFIXES
The known suffixes are simply the names of the dependencies of the special
target @code{.SUFFIXES}.  You can add your own suffixes by writing a rule
for @code{.SUFFIXES} that adds more dependencies, as in

@example
.SUFFIXES: .hack .win
@end example

@noindent
which adds @samp{.hack} and @samp{.win} to the end of the list of suffixes.

If you wish to eliminate the default known suffixes instead of just adding
to them, write a rule for @code{.SUFFIXES} with no dependencies.  By
special dispensation, this eliminates all existing dependencies of
@code{.SUFFIXES}.  You can then write another rule to add the suffixes you
want.  For example,

@example
.SUFFIXES:    @r{# Delete the default suffixes}
.SUFFIXES: .c .o .h   @r{# Define our suffix list}
@end example

The @samp{-r} flag causes the default list of suffixes to be empty.

@node Search Algorithm,, Last Resort, Implicit
@section Implicit Rule Search Algorithm

Here is the procedure @code{make} uses for searching for an implicit rule
for a target @var{t}.  This procedure is followed for each double-colon
rule with no commands, for each target of ordinary rules none of which have
commands, and for each dependency that is not the target of any rule.  It
is also followed recursively for dependencies that come from implicit
rules, in the search for a chain of rules.

Suffix rules are not mentioned in this algorithm because suffix rules are
converted to equivalent pattern rules after the makefiles have been read
in.

For an archive member target of the form
@samp{@var{archive}(@var{member})}, the following algorithm is run twice,
first using @samp{(@var{member})} as the target @var{t}, and second using
the entire target if the first run found no rule.@refill

@enumerate
@item
Split @var{t} into a directory part, called @var{d}, and the rest,
called @var{n}.  For example, if @var{t} is @samp{src/foo.o}, then
@var{d} is @samp{src/} and @var{n} is @samp{foo.o}.@refill

@item
Make a list of the pattern rules whose target matches @var{d} or
@var{n}.  If the target pattern contains a slash, it is matched
against @var{t}; otherwise, against @var{n}.

@item
If any rule in that list is @emph{not} a match-anything rule, then
remove all nonterminal match-anything rules from the list.

@item
Remove any rules with no dependencies from the list.

@item
For each pattern rule in the list:

@enumerate
@item
Find the stem @var{s}: the part of @var{d} or @var{n} that the
@samp{%} in the target pattern matches.@refill

@item
Compute the dependency names by substituting @var{s} for @samp{%}; if
the target pattern does not contain a slash, @var{d} is appended to
the front of each dependency name.

@item
Test whether all the dependencies exist or ought to exist.  (If a
file name mentioned in the makefile as a target or as an explicit
dependency then we say it ought to exist.)

If all dependencies exist or ought to exist, then this rule applies.
@end enumerate

@item
If no pattern rule has been found so far, try harder.  For
each pattern rule in the list:

@enumerate
@item
If the rule is a terminal match-anything rule, ignore it and go
on to the next rule.

@item
Compute the dependency names as before.

@item
Test whether all the dependencies exist or ought to exist.

@item
For each dependency that does not exist, follow this algorithm
recursively to see if the dependency can be made by an implicit
rule.

@item
If all dependencies exist, ought to exist, or can be made by
implicit rules, then this rule applies.
@end enumerate

@item
If no rule has been found so far, this target cannot be made by an
implicit rule.  Return failure.

@item
If no implicit rule applies, the rule for @code{.DEFAULT}, if any,
applies.  In that case, give @var{t} the same commands that
@code{.DEFAULT} has.  Otherwise, there are no commands for @var{t}.
@end enumerate

When the commands of a pattern rule are executed for @var{t}, the automatic
variables @samp{$@@}, @samp{$*} and @samp{$<} are set as follows:

@table @samp
@item $@@
@var{t}
@item $*
If the target pattern contains a slash, this is @var{s}; otherwise, it
is @var{d}@var{s}.
@item $<
The name of the first dependency that came via the implicit rule.
@end table

For @code{.DEFAULT} commands, as for non-implicit commands, @samp{$*}
and @samp{$<} are empty.  @samp{$@@} is @var{t}, as always.

@node Archives, Missing, Implicit, Top
@chapter Using @code{make} to Update Archive Files
@cindex archive

@dfn{Archive files} are files containing named subfiles called
@dfn{members}; they are maintained with the program @code{ar} and their
main use is as subroutine libraries for linking.

@menu
* Members: Archive Members.    How to name an archive member
				as a target or dependency.
* Update: Archive Update.      An implicit rule can update
				most archive member targets just right.
* Symbols: Archive Symbols.    Another implicit rule runs @code{ranlib}
				to update the special member @file{__.SYMDEF}.
@end menu

@node Archive Members, Archive Update, Archives, Archives
@section Archive Members as Targets
@cindex archive member targets

An individual member of an archive file can be used as a target or
dependency in @code{make}.  The archive file must already exist, but the
member need not exist.  You specify the member named @var{member} in
archive file @var{archive} as follows:

@example
@var{archive}(@var{member})
@end example

@noindent
This construct is available only in targets and dependencies, not in
commands!  Most programs that you might use in commands do not support this
syntax and cannot act directly on archive members.  Only @code{ar} and
other programs specifically designed to operate on archives can do so.
Therefore, valid commands to update an archive member target probably must
use @code{ar}.  For example, this rule says to create a member
@file{hack.o} in archive @file{foolib} by copying the file @file{hack.o}:

@example
foolib(hack.o) : hack.o
        ar r foolib hack.o
@end example

In fact, nearly all archive member targets are updated in just this way
and there is an implicit rule to do it for you.

@node Archive Update, Archive Symbols, Archive Members, Archives
@section Implicit Rule for Archive Member Targets

Recall that a target that looks like @file{@var{a}(@var{m})} stands for the
member named @var{m} in the archive file @var{a}.

When @code{make} looks for an implicit rule for such a target, as a special
feature it considers implicit rules that match @file{(@var{m})}, as well as
those that match the actual target @file{@var{a}(@var{m})}.

This causes one special rule whose target is @file{(%)} to match.  This
rule updates the target @file{@var{a}(@var{m})} by copying the file @var{m}
into the archive.  For example, it will update the archive member target
@file{foo.a(bar.o)} by copying the @emph{file} @file{bar.o} into the
archive @file{foo.a} as a member named @file{bar.o}.

When this rule is chained with others, the result is very powerful.  The
command @samp{make "foo.a(bar.o)"} in the presence of a file @file{bar.c}
is enough to cause the following commands to be run, even without a
makefile:

@example
cc -c bar.c -o bar.o
ar r foo.a bar.o
rm -f bar.o
@end example

@noindent
Here the file @file{bar.o} has been invented as an intermediate file.

@node Archive Symbols,, Archive Update, Archives
@subsection Updating Archive Symbol Directories
@cindex __.SYMDEF

An archive file that is used as a library usually contains a special member
named @file{__.SYMDEF} that contains a directory of the external symbol
names defined by all the other members.  After you update any other
members, you need to update @file{__.SYMDEF} so that it will summarize the
other members properly.  This is done by running the @code{ranlib} program:

@example
ranlib @var{archivefile}
@end example

Normally you would put this command in the rule for the archive file,
and make all the members of the archive file dependents of that rule.
For example,

@example
libfoo.a: libfoo.a(x.o) libfoo.a(y.o) @dots{}
        ranlib libfoo.a
@end example

@noindent
The effect of this is to update archive members @file{x.o}, @file{y.o},
etc., and then update the symbol directory member @file{__.SYMDEF} by
running @code{ranlib}.  The rules for updating the members are not shown
here; most likely you can omit them and use the implicit rule which copies
files into the archive, as described in the preceding section.

@node Missing, Concept Index, Archives, Top
The variable @code{MAKELEVEL} which keeps track of the current level
of @code{make} recursion.  @xref{Recursion}.
The @code{make} programs in various other systems support two features that
are not implemented in GNU @code{make}.
Static pattern rules.  @xref{Static Pattern}.

@item
Selective @code{vpath} search.  @xref{Directory Search}.

@item
Recursive variable references.  @xref{Reference}.
@end itemize

@node Missing, Concept Index, Features, Top
@chapter Missing Features in GNU @code{make}

The @code{make} programs in various other systems support a few features
that are not implemented in GNU @code{make}.

suffix rule @samp{.c~.o} would make files @file{@var{n}.o} file from
@item
A target of the form @samp{@var{file}((@var{entry}))} stands for a member
of archive file @var{file}.  The member is chosen, not by name, but by
being an object file which defines the linker symbol @var{entry}.@refill

This feature was not put into GNU @code{make} because of the
@noindent
can be replaced with the GNU @code{make} static pattern rule:

@example
$(targets): %: %.o lib.a
@end example

@item
In System V and 4.3 BSD @code{make}, files found by @code{VPATH} search
(@pxref{Directory Search}) have their names changed inside command strings.
We feel it is much cleaner to always use automatic variables and thus
obviate the need for this feature.  We are still debating whether to
implement this to be compatible or to leave it out to avoid such ugliness.
@end itemize

@node Concept Index, Name Index, Missing, Top
@unnumbered Index of Concepts

@printindex cp

@node Name Index,, Concept Index, Top
@unnumbered Index of Functions, Directives and Variables

@printindex fn

@contents
@bye
