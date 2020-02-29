.. index:: write
.. _write:

Write the Commands of a Target (write)
--------------------------------------

**write** [*target* [[*filename|**here**]]]

Use this to write the command portion of a target with `remake`s
internal variables expanded.  If a filename is given that is the file
where the expanded commands are written. If the filename is `here`
then it is not written to a file but output inside the debugger as
other debugger commands behave. And if no file name is given a
filename based on the target name is created.


Examples:
+++++++++

::

     $ remake -X -f tests/spec/example/simple.Makefile
     Reading makefiles...
     Updating makefiles....
     Updating goal targets....
     -> (/tmp/remake/tests/spec/example/simple.Makefile:2)
     all:
     remake<0> write
     File "/tmp/all.sh" written.
     remake<1> w all here
     #!/bin/sh
     ## /src/external-vcs/github/rocky/remake/tests/spec/example/simple.Makefile:2
     ## all:

     #cd /src/external-vcs/github/rocky/remake
     echo all here
