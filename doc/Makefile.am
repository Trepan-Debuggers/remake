## Process this file with automake to create Makefile.in.

## Makefile for GNU make documentation.
## Copyright 2002, 2004 Free Software Foundation, Inc.

TEXI2HTML = texi2html
TEXI2HTML_FLAGS = -split_chapter

info_TEXINFOS =	make.texi makedb.texi
make_TEXINFOS = fdl.texi make-stds.texi

CLEANFILES = make*.html

# Extra stuff to include in the distribution.

EXTRA_DIST =	doxygen/Doxyfile doxygen/run_doxygen

## ----------------------------- ##
## Other documentation formats.  ##
## ----------------------------- ##

reference:
	-( cd ${top_srcdir} && $(MAKE) doxygen )

doxygen:
	-( cd ${top_srcdir}/doc/doxygen && /bin/sh ${srcdir}/run_doxygen )

html: make_1.html

make_1.html: $(info_TEXINFOS) $(make_TEXINFOS)
	-$(TEXI2HTML) $(TEXI2HTML_FLAGS) $(srcdir)/make.texi

.PHONY: html
