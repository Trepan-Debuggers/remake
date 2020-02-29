.. index:: info; variables
.. _info_variables:

Info Variables
----------------

**info variables**

List all POSIX-shell environment and GNU Make variables.

Each variable is shown on a single line and is preceded by a line which indicates the
type of variable. This list of types is:

* automatic: variables which have values computed afresh for each rule
* default: a variable using its default value
* environment: a POSIX shell environment variable
* pattern-specific

At the end of the list, hash table statistics are shown.

Example:
++++++++

::

    # 'override' directive
    GNUMAKEFLAGS :=
    # automatic
    <D = $(patsubst %/,%,$(dir $<))
    # automatic
    @D = $(patsubst %/,%,$(dir $@))
    # default
    .SHELLFLAGS := -c
    # default
    LD = ld
    # environment
    PATH = /usr/bin/:/sbin/
    ...
    # variable set hash-table stats:
    # Load=214/1024=21%, Rehash=0, Collisions=35/240=15%

.. seealso::

   :ref:`print <print>`,  :ref:`expand <expand>`.
