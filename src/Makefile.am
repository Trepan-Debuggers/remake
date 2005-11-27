# This is a -*-Makefile-*-, or close enough

transform = s,make,@MAKE_NAME@,

ACLOCAL_AMFLAGS	=  -I config

bin_PROGRAMS =	make

if USE_CUSTOMS
  remote =	remote-cstms.c
else
  remote =	remote-stub.c
endif

make_SOURCES =	ar.c arscan.c commands.c dbg_cmd.c dbg_fns.c dbg_stack.c \
	        debug.c default.c dep.c dir.c expand.c file.c \
		function.c getopt.c getopt1.c  hash.c implicit.c job.c \
	        main.c misc.c print.c read.c remake.c $(remote) rule.c \
	        signame.c trace.c variable.c version.c vpath.c

EXTRA_make_SOURCES = remote-stub.c remote-cstms.c

noinst_HEADERS = arscan.h commands.h dbg_cmd.h dbg_fns.h dbg_stack.h debug.h \
	         default.h dep.h expand.h filedef.h \
	         function.h getopt.h gettext.h hash.h implicit.h job.h \
		 make.h print.h read.h remake.h remote-stub.h rule.h \
		 trace.h types.h variable.h  vpath.h

make_LDADD =	@LIBOBJS@ @ALLOCA@ $(GLOBLIB) @GETLOADAVG_LIBS@ @LIBINTL@

DEFS =		-DLOCALEDIR=\"$(localedir)\" -DLIBDIR=\"$(libdir)\" -DINCLUDEDIR=\"$(includedir)\" @DEFS@

AM_CPPFLAGS =	$(GLOBINC)


# Extra stuff to include in the distribution.
# Note we need all the glob stuff here, rather than in glob/Makefile.am,
# because often that directory isn't built on the systems used by the
# maintainers.

MAKE_HOST =	@MAKE_HOST@


.PHONY: check-loadavg 

check-loadavg: loadavg$(EXEEXT)
	@echo The system uptime program believes the load average to be:
	-uptime
	@echo The GNU load average checking code thinks:
	-./loadavg$(EXEEXT)

# The loadavg function is invoked during "make check" to test getloadavg.
noinst_PROGRAMS = loadavg
loadavg_CPPFLAGS = -DTEST
loadavg_LDADD = @GETLOADAVG_LIBS@

loadavg.o: loadavg.c
	source='$<' object='$@' libtool=no \
	depfile='$(DEPDIR)/$*.Po' tmpdepfile='$(DEPDIR)/$*.TPo' \
	$(CCDEPMODE) $(depcomp) \
	$(COMPILE) -DTEST -c `test -f '$<' || echo '$(srcdir)/'`$<



# Forward targets

# --------------- Local CHECK Section

check-local: check-loadavg

# check-regression
#
# Look for the make test suite, and run it if found and we can find perl.
# If we're building outside the tree, we use symlinks to make a local copy of
# the test suite.  Unfortunately the test suite itself isn't localizable yet.
#
MAKETESTFLAGS =

# --------------- Maintainer's Section

@MAINT_MAKEFILE@
