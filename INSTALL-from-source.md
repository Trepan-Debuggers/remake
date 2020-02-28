# Prerequisites:

To build from sources you need:

* a previous version of remake or GNU make
* A C compiler like [`gcc`](https://gcc.gnu.org/) or [`clang`](https://clang.llvm.org/)
* [`gettext`](https://www.gnu.org/software/gettext/)
* [GNU Readline](https://tiswww.case.edu/php/chet/readline/rltop.html) which the debugger currently requires

Optionally you may want:

* [Guile](https://www.gnu.org/software/guile/) version 2.0 or greater


Additionally, if installing from git you need:

* `git` (duh)
* `autoconf`
* `automake`
* `autopoint`

and optionally:

* `gzip` and `lzip` if building a compressed tarball

Here is a `apt-get` command you can use to install on Debian-ish systems:

```console
   $ sudo apt-get install git gcc pkg-config autoconf automake autopoint libreadline-dev make guile-2.2 texinfo lzip
```

Here is a `yum` command Redhat/CentOS:

```console
   $ sudo yum install git gcc pkgconfig autoconf automake readline-devel make guile lzip
```

To build documentation you need:

* [`texinfo`](https://www.gnu.org/software/texinfo/) for building the TeXInfo docs

Add that to the `apt-get` or `yum` command above.

# Simplified approach

```console
   $ $SHELL ./autogen.sh
   $ make && make check
```

This performs the step below steps up to but not including
"Building".

# Creating and running configure script


```console
	$ autoreconf -f -i
	$ ./configure
```

# Updating translation links

After running `configure` run:

    $ make po-update

to pull in the latest translation strings.


# TeXinfo mess

```console
    $ (cd doc && make stamp-1 stamp-vti)
```

# Building

So the full sequence is:

```console

   $ cd remake*
   $ autoreconf -f -i
   $ ./configure
   $ make po-update
   $ (cd doc && make stamp-1 stamp-vti)
   $ make && make check
   $ make install # may need sudo
```

# Unbuilding

The main targets to remove `remake` are:

* `uninstall` - removes files created via  `make install` or removes installation
* `clean`  - removes files created via  `make` or `make all`
* `distclean` - more aggresively removes any files that are not part of git
