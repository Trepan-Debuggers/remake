# Prerequisites:

Build from sources you need:

* a previous version of GNU make
* gcc
* gettext
* GNU Readline
* guile version 2.0


Additionally if installing from git you need:

* git (duh)
* autoconf
* automake
* autopoint

Here is a `apt-get` command you can use to install on Debian-ish systems:

```console
   $ sudo apt-get install git gcc pkg-config autoconf automake autopoint libreadline-dev make guile-2.0
```

Here is a `yum` command Redhat/CentOS:

```console
   $ sudo yum install git gcc pkgconfig autoconf automake readline-devel make guile
```

To build documentation you need:

* texinfo

Add that to the `apt-get` or `yum` command above.

# Simplicfied approach


   $ $SHELL ./autogen.sh
   $ make && make check


This performs the step below steps up to but not including
"Building".

# Creating and running configure script


	$ autoreconf -f -i
	$ ./configure


# Updating translation links

After running `configure` run:

    $ make po-update

to pull in the latest translation strings.


# TeXinfo mess


    $ (cd doc && make stamp-1 stamp-vti)


# Building

So the full sequence is:

```console

   $ cd remake*
   $ autoreconf -f -i
   $ ./configure
   $ make po-update
   $ (cd doc && make stamp-1 stamp-vti)
   $ make && make check
```
