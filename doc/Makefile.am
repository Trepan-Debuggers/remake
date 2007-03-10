## Process this file with automake to create Makefile.in.

## Makefile for GNU make documentation.
## Copyright 2002, 2004, 2005, 2006 Free Software Foundation, Inc.

transform = s,make,@MAKE_NAME@,

info_TEXINFOS = make.texi mdb.texi
make_TEXINFOS = fdl.texi make-stds.texi
TEXI2PDF = texi2dvi --pdf

MOSTLYCLEANFILES = make.tgs make.pdf make.html mdb.cps mdb.kys mdb.html mdb.fns mdb.ps.gz mdb.pdf 

CLEANFILES = make*.html make*.info*

# Extra stuff to include in the distribution.

EXTRA_DIST =	doxygen/Doxyfile doxygen/Doxyfile.in doxygen/Doxyfile \
	        doxygen/run_doxygen fdl.texi make.texi \
	        make.info make-stds.texi mdb.texi mdb.info version-mdb.texi

## ----------------------------- ##
## Other documentation formats.  ##
## ----------------------------- ##

all: mdb.info make.info

$(srcdir)/version-mdb.texi: $(srcdir)/stamp-vti

reference:
	-( cd ${top_srcdir} && $(MAKE) doxygen )

doxygen:
	-( cd ${top_srcdir}/doc/doxygen && /bin/sh ${srcdir}/run_doxygen )

html: make.html mdb.html

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


