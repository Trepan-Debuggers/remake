How to install
****************

.. contents:: :local:

.. toctree::


From a Package
----------------

Repology_ maintains a list of various bundled `remake` packages. Below are some specific distributions that contain `remake`.

At the time this documentation was built, here is status that they provide:

|packagestatus|

Check the link above for more up-to-date information.


.. |packagestatus| image:: https://repology.org/badge/vertical-allrepos/remake.svg
		 :target: https://repology.org/project/remake/versions


Debian/Ubuntu
+++++++++++++++

On Debian systems, and derivatives, `remake` can be installed by running:

.. code:: console

    $ sudo apt-get install remake


The latest version may not yet be included in the archives. If you are running
a stable version of Debian or a derivative, you may need to install `remake` from
the backports repository for your version to get a recent version installed.

MacOSX
+++++++

On OSX systems, you can install from Homebrew or MacPorts_.

.. code:: console

    $  brew install remake


From Source
------------

SourceForge
++++++++++++

Go to sourceforge_ and find the most recent version and download a tarball of that.


.. code:: console

    $ tar -xpf remake-xxx.tar.bz2
    $ cd remake-xxx
    $ ./autogen.sh
    $ make && make test
    $ make install # may need sudo



github
++++++


Many package managers have back-level versions of this debugger. The most recent versions is from the github_.

Prerequisites
..............

To build from sources you need:

* a previous version of remake or GNU make
* A C compiler like gcc_ or clang_
* gettext_
* GNU _Readline

Optionally you may want:

* Guile version 2.0 or greater


Additionally if installing from git you need:

* `git` (duh)
* `autoconf`
* `automake`
* `autopoint`
* `gettext` to process the language-customizaton files in the `po` director

and optionally:

* gzip and lzip (to compress the tarball)

Here is a `apt-get` command you can use to install on Debian-ish systems:

.. code:: console

   $ sudo apt-get install git gcc pkg-config autoconf automake autopoint gettext libreadline-dev make guile-2.0 texinfo lzip


Here is a `yum`/`dnf` command for Redhat/CentOS:

.. code:: console

   $ sudo yum install git gcc pkgconfig autoconf automake gettext readline-devel make guile lzip

   # on CentOS 7 and later, autopoint is part of gettext-devel
   $ sudo yum install git gcc pkgconfig autoconf automake gettext gettext-devel readline-devel make guile lzip

Here is a `pkg` command for FreeBSD:

.. code:: console

   $ sudo pkg install git gcc pkgconf autotools automake gettext gmake readline rsync guile2 lzip wget


Here is a `pkg_add` command for OpenBSD as, root:

.. code:: console

   $ pkg_add install git pkgconf autoconf-2.69p2 automake-1.16.1 gettext-tools ggrep gmake readline rsync-3.1.3 guile2 lzip wget

To build documentation you need:

* texinfo_

Add that to the `apt-get` or `yum` command above.

Simplified approach
...................

.. code:: console

   $ $SHELL ./autogen.sh
   $ make && make check


This performs the step below steps up to but not including
"Building".

Creating and running configure script
.....................................

.. code:: console

    $ autoreconf -f -i
    $ ./configure --enable-maintainer-mode "$@"
    $ make po-update
    $ (cd doc && make stamp-1 stamp-vti)


Updating language-translation text substitutions
................................................

After running `configure` run:

.. code:: console

    $ make po-update

to pull in the latest translation strings.


TeXinfo mess
.............

.. code:: console

    $ (cd doc && make stamp-1 stamp-vti)


Building
........

So the full sequence is:

.. code:: console

   $ cd remake*
   $ autoreconf -f -i
   $ ./configure
   $ make po-update
   $ (cd doc && make stamp-1 stamp-vti)
   $ make && make check
   $ make install # may need sudo

Unbuilding
..........

The main targets to remove `remake` are:

* `uninstall` - removes files created via  `make install` or removes installation. Since some files
* `clean`  - removes files created via  `make` or `make all`
* `distclean` - more aggressively removes any files that are not part of git

Therefore to remove file installed via `make install`:

.. code:: console

    $ make uninstall # ;-)


.. _MacPorts: https://ports.macports.org/port/remake/summary
.. _Repology: https://repology.org/project/remake/versions
.. _github: https://github.com/rocky/remake
.. _sourceforge: https://sourceforge.net/projects/bashdb/files/remake/
.. _gcc: https://gcc.gnu.org/
.. _clang: https://clang.llvm.org/
.. _gettext: https://www.gnu.org/software/gettext/
.. _Guile: https://www.gnu.org/software/guile/
.. _texinfo: https://www.gnu.org/software/guile/
