## Process this file with automake to create Makefile.in.

## Makefile for GNU make documentation.
## Copyright 2002, 2004, 2005 Free Software Foundation, Inc.

transform = s,make,@MAKE_NAME@,

make_TEXINFOS = fdl.texi make-stds.texi
makedb_TEXINFOS = makedb.texi 
TEXI2PDF = texi2dvi --pdf

MOSTLYCLEANFILES = make.tgs make.pdf make.html makedb.cps makedb.kys makedb.html makedb.fns makedb.ps.gz makedb.pdf 

CLEANFILES = make*.html make*.info

# Extra stuff to include in the distribution.

EXTRA_DIST =	doxygen/Doxyfile doxygen/run_doxygen makedb.info

## ----------------------------- ##
## Other documentation formats.  ##
## ----------------------------- ##

all: makedb.info make.info

reference:
	-( cd ${top_srcdir} && $(MAKE) doxygen )

doxygen:
	-( cd ${top_srcdir}/doc/doxygen && /bin/sh ${srcdir}/run_doxygen )

html: make.html makedb.html

.texi.html:
	texi2html $<

.texi.pdf:
	texi2pdf $<

.texi.dvi:
	texi2dvi $<

.texi.info:
	makeinfo $< 

.texi.txt:
	makeinfo --no-headers $< > $@


