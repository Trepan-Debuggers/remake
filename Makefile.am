# -*-Makefile-*-, or close enough

AUTOMAKE_OPTIONS = 1.2

bin_PROGRAMS =	make

make_SOURCES =	main.c commands.c job.c dir.c file.c misc.c read.c remake.c \
		rule.c implicit.c default.c variable.c expand.c function.c \
		vpath.c version.c ar.c arscan.c remote-$(REMOTE).c \
		commands.h dep.h filedef.h job.h make.h rule.h variable.h \
		signame.c signame.h \
		getopt.c getopt1.c getopt.h
make_LDADD =	@LIBOBJS@ @ALLOCA@ glob/libglob.a

info_TEXINFOS =	make.texinfo

INCLUDES =	-I$(srcdir)/glob -DLIBDIR=\"$(libdir)\" -DINCLUDEDIR=\"$(includedir)\"

BUILT_SOURCES =	README build.sh.in

EXTRA_DIST = 	make.man $(BUILT_SOURCES) remote-cstms.c\
		make-stds.texi texinfo.tex SCOPTIONS SMakefile\
		README.Amiga Makefile.ami config.ami make.lnk amiga.c amiga.h\
		README.DOS Makefile.DOS configure.bat dosbuild.bat configh.dos\
		README.W32 NMakefile config.h.W32 build_w32.bat subproc.bat\
		readme.vms makefile.vms makefile.com config.h-vms vmsdir.h\
		vmsfunctions.c vmsify.c

SUBDIRS =	glob

# --------------- Local DIST Section

# Install the w32 subdirectory
#
dist-hook:
	(cd $(srcdir); \
	 w32=`find w32 -follow \( -name CVS -prune \) -o -type f -print`; \
	 tar chf - $$w32) \
	| (cd $(distdir); tar xfBp -)


# --------------- Local CHECK Section

check-local: check-loadavg check-regression
.PHONY: check-loadavg check-regression

# > check-loadavg
#
loadavg: loadavg.c config.h
	@rm -f loadavg
	$(LINK) -I. -I$(srcdir) -DHAVE_CONFIG_H -DTEST $(make_LDFLAGS) loadavg.c $(LIBS)
# We copy getloadavg.c into a different file rather than compiling it
# directly because some compilers clobber getloadavg.o in the process.
loadavg.c: getloadavg.c
	ln $(srcdir)/getloadavg.c loadavg.c || \
	cp $(srcdir)/getloadavg.c loadavg.c
check-loadavg: loadavg
	@echo The system uptime program believes the load average to be:
	-uptime
	@echo The GNU load average checking code believes:
	-./loadavg

# > check-regression
#
# Look for the make test suite, and run it if found.  Look in MAKE_TEST if
# specified, or else in the srcdir or the distdir, their parents, and _their_
# parents.
#
check-regression:
	here=`pwd`; testdir=""; \
	  case "$(MAKE_TEST)" in "") \
	    for d1 in $$here $(srcdir); do \
	      for d2 in ../.. .. .; do \
		all=`echo $$d1/$$d2/make-test-[0-9]*/run_make_tests`; \
		case "$$all" in \
		  "$$d1/$$d2/make-test-[0-9]*/run_make_tests") : ;; \
		  *) try=`for x in $$all; do echo $$x; done | sort | tail -1`;\
		     testdir=`dirname $$try` ;; esac; \
            done; done ;; \
	    *) testdir="$(MAKE_TEST)" ;; \
	  esac; \
	  case "$$testdir" in \
	    "") echo "Couldn't find make-test-* test suite."; exit 0;; \
	  esac; \
	  echo "cd $$testdir && ./run_make_tests -make_path $$here/make"; \
	  cd $$testdir && ./run_make_tests -make_path $$here/make


# --------------- Maintainer's Section

if MAINT_MAKEFILE
# Note this requires GNU make.  Not to worry, since it will only be included
# in the Makefile if we're in the maintainer's environment.
include $(srcdir)/maintMakefile
endif
