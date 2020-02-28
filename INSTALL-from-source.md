# Prerequisites:

To build from sources you need:

* a previous version of remake or GNU make
* A C compiler like [`gcc`](https://gcc.gnu.org/) or [`clang`](https://clang.llvm.org/)
* [`gettext`](https://www.gnu.org/software/gettext/)
* [GNU Readline](https://tiswww.case.edu/php/chet/readline/rltop.html) which the debugger currently requires

Optionally you may want:

* [Guile](https://www.gnu.org/software/guile/) version 2.0 or greater


Additionally, if installing from _git_ you need:

* `git` (duh)
* `autoconf`
* `automake`
* `autopoint`
* `gettext` to process the language-customizaton files in the `po` director

and optionally:

* `gzip` and `lzip` if building a compressed tarball
* `guile` version 2 or greater if you want Guile support in `remake`

Here is a `apt-get` command you can use to install on Debian-ish systems:

```console
   $ sudo apt-get install git gcc pkg-config autoconf automake autopoint gettext libreadline-dev make guile-2.2 lzip
```

Here is a `yum`/`dnf` command for Redhat/CentOS:

```console
   $ sudo yum install git gcc pkgconfig autoconf automake gettext readline-devel make guile lzip

   # on CentOS 7 and later, autopoint is part of gettext-devel
   $ sudo yum install git gcc pkgconfig autoconf automake gettext gettext-devel readline-devel make guile lzip
```

Here is a `brew` command for macOS (additionally Xcode is required):

```console
   $ sudo brew install git pkg-config autoconf automake gettext readline guile lzip
```

Here is a `pkg` command for FreeBSD:

```console
   $ sudo pkg install git gcc pkgconf autotools automake gettext gmake readline rsync guile2 lzip wget
```

Here is a `pkg_add` command for OpenBSD as, root:

```console
   $ pkg_add install git pkgconf autoconf-2.69p2 automake-1.16.1 gettext-tools ggrep gmake readline rsync-3.1.3 guile2 lzip wget
```

To build documentation you need:

* [`texinfo`](https://www.gnu.org/software/texinfo/) for building the TeXInfo docs

Add that to any of the above package installation commands above.

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
	$ ./configure --enable-maintainer-mode "$@"
```

# Updating language-translation text substitutions

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

Therefore to remove file installed via `make install`:

```console
    $ make uninstall # ;-)
```
