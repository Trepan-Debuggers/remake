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

EXTRA_DIST = 	make.man $(BUILT_SOURCES) remote-cstms.c \
		make-stds.texi texinfo.tex SCOPTIONS SMakefile \
		Makefile.ami README.Amiga config.ami amiga.c amiga.h \
		NMakefile README.DOS configh.dos configure.bat makefile.com \
		README.W32 build_w32.bat config.h.W32 subproc.bat make.lnk \
		config.h-vms makefile.vms readme.vms vmsdir.h vmsfunctions.c \
		vmsify.c

SUBDIRS =	glob

# Look for the make test suite, and run it if found.  Look in MAKE_TEST, or
# in the srcdir or the distdir, their parents, and their parents.
#
check-local:
	@here=`pwd`; testdir=""; \
	  case "$(MAKE_TEST)" in "") \
	    for d1 in $$here $(srcdir); do \
	      for d2 in ../.. .. .; do \
		try=`ls -1 $$d1/$$d2/make-test-*/run_make_tests 2>/dev/null | tail -1`; \
		case "$$try" in "") : ;; *) testdir=`dirname $$try` ;; esac; \
            done; done ;; \
	    *) testdir="$(MAKE_TEST)" ;; \
	  esac; \
	  case "$$testdir" in \
	    "") echo "Couldn't find make-test-* test suite."; exit 0;; \
	  esac; \
	  echo "cd $$testdir && ./run_make_tests -make_path $$here/make"; \
	  cd $$testdir && ./run_make_tests -make_path $$here/make

# Install the w32 subdirectory
#
dist-hook:
	(cd $(srcdir); \
	 w32=`find w32 -follow \( -name CVS -prune \) -o -type f -print`; \
	 tar chf - $$w32) \
	| (cd $(distdir); tar xfBp -)

if MAINT_MAKEFILE
# Note this requires GNU make.  Not to worry, since it will only be included
# in the Makefile if we're in the maintainer's environment.
include $(srcdir)/maintMakefile
endif
