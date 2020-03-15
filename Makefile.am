# This is a -*-Makefile-*-, or close enough
#
# Copyright (C) 1997-2020 Free Software Foundation, Inc.
# This file is part of GNU Make.
#
# GNU Make is free software; you can redistribute it and/or modify it under
# the terms of the GNU General Public License as published by the Free Software
# Foundation; either version 3 of the License, or (at your option) any later
# version.
#
# GNU Make is distributed in the hope that it will be useful, but WITHOUT ANY
# WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
# FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more
# details.
#
# You should have received a copy of the GNU General Public License along with
# this program.  If not, see <http://www.gnu.org/licenses/>.

AUTOMAKE_OPTIONS = 1.8 dist-lzip silent-rules std-options subdir-objects nostdinc
ACLOCAL_AMFLAGS = -I m4

transform = s,make,@MAKE_NAME@,

MAKE_HOST =	@MAKE_HOST@

SUBDIRS =	lib po doc glob unittest libdebugger

bin_PROGRAMS =	make
include_HEADERS = src/gnuremake.h

man_MANS =	doc/make.1

DEBUGGER_SRC = 	\
		src/debugger/break.c \
		src/debugger/cmd.c 	\
		src/debugger/fns.c 	\
		src/debugger/file2line.c 	\
		src/debugger/msg.c 	\
		src/debugger/stack.c

DEBUGGER_H = \
	$(wildcard src/debugger/*.h) \
	$(wildcard src/debugger/command/*.h) \
	$(wildcard src/debugger/command/help/*.h)

make_SRCS =	\
		src/alloc.c src/ar.c src/arscan.c src/buildargv.c \
		src/commands.c src/debug.c src/default.c src/dir.c src/expand.c \
		src/file.c src/file_basic.c src/function.c src/getopt.c \
		src/getopt1.c src/globals.c src/guile.c src/hash.c src/implicit.c \
		src/job.c src/load.c src/loadapi.c src/main.c src/misc.c \
		src/output.c src/print.c src/profile.c src/read.c src/remake.c \
		src/rule.c src/rule.h src/signame.c src/strcache.c \
		src/types.h  src/trace.c src/variable.c src/version.c src/vpath.c \
		$(DEBUGGER_SRC)

glob_SRCS =	lib/fnmatch.c lib/fnmatch.h lib/glob.c lib/glob.h

alloca_SRCS =	lib/alloca.c

loadavg_SRCS =	lib/getloadavg.c

make_SOURCES =	$(make_SRCS)

EXTRA_make_SOURCES = src/mock.c

noinst_HEADERS = src/buildargv.h src/commands.h src/debug.h src/debugger.h \
				src/dep.h src/expand.h src/file.h src/file_basic.h \
				src/filedef.h src/function.h src/getopt.h src/gettext.h \
				src/globals.h src/hash.h src/implicit.h src/job.h \
				src/main.h src/make.h src/makeint.h src/mock.h src/os.h \
				src/output.h src/print.h src/profile.h src/read.h \
				src/rule.h src/trace.h src/types.h src/variable.h src/vpath.h \
				$(DEBUGGER_H)

make_LDADD =	$(LIBOBJS) $(GUILE_LIBS) lib/libgnu.a $(GETLOADAVG_LIBS) \
				libdebugger/libdebugger.a @LIBINTL@

localedir =	$(datadir)/locale

AM_CPPFLAGS =	-Isrc -I$(top_srcdir)/src -Ilib -I$(top_srcdir)/lib \
		-DLIBDIR=\"$(libdir)\" -DINCLUDEDIR=\"$(includedir)\" \
		-DLOCALEDIR=\"$(localedir)\"

AM_CFLAGS += $(GUILE_CFLAGS)

make_SOURCES += src/posixos.c

if USE_CUSTOMS
  # make_SOURCES += src/remote-cstms.c src/mock.c
  make_SOURCES += src/remote-cstms.c
else
  # make_SOURCES += src/remote-stub.c src/mock.c
  make_SOURCES += src/remote-stub.c
endif

# Extra stuff to include in the distribution.

# We don't need this, since the standard automake output will do.
#mk/Posix.mk.in

m4_FILES =	m4/gnulib-cache.m4

test_FILES =	tests/run_make_tests \
		tests/run_make_tests.pl tests/test_driver.pl \
		tests/config-flags.pm.in  \
		tests/mkshadow tests/thelp.pl tests/guile.supp tests/README
# test/scripts are added via dist-hook below.

EXTRA_DIST =	ChangeLog README $(man_MANS) \
		README.customs \
		src/gmk-default.scm src/gmk-default.h \
		NEWS-remake.md README.remake REMAKE.todo Rakefile \
		$(m4_FILES) $(test_FILES)

# --------------- Generate the Guile default module content

#: Generate the Guile module
src/guile.$(OBJEXT): src/gmk-default.h
src/gmk-default.h: $(top_srcdir)/src/gmk-default.scm
	(echo 'static const char *const GUILE_module_defn = " '\\ \
	  && sed -e 's/;.*//' -e '/^[ \t]*$$/d' -e 's/"/\\"/g' -e 's/$$/ \\/' \
		 $(top_srcdir)/src/gmk-default.scm \
	  && echo '";') > src/gmk-default.h

# --------------- Local DIST Section

#: Install the tests subdirectories
dist-hook:
	(cd $(top_srcdir); \
	 sub=`find src tests/scripts -follow \( -name .git -o -name .deps -o -name work -o -name .gitignore -o -name \*.orig -o -name \*.rej -o -name \*~ -o -name \*.out -o -name Makefile \) -prune -o -type f -print`; \
	 tar chf - $$sub) \
	| (cd $(distdir); tar xfBp -)


# --------------- Local CHECK Section

#: run check-regression and show a banner
check-local: check-regression
	@banner=" Regression PASSED: GNU Make $(VERSION) ($(MAKE_HOST)) built with $(CC) "; \
	dashes=`echo "$$banner" | sed s/./=/g`; \
	echo; \
	echo "$$dashes"; \
	echo "$$banner"; \
	echo "$$dashes"; \
	echo

# > check-regression
#
# Look for the make test suite, and run it if found and we can find perl.
# If we're building outside the tree, we use symlinks to make a local copy of
# the test suite.  Unfortunately the test suite itself isn't localizable yet.
#
MAKETESTFLAGS =

.PHONY: check-regression

#: run regression tests in tests/scripts
check-regression: tests/config-flags.pm
	@if test -f '$(top_srcdir)/tests/run_make_tests'; then \
	  ulimit -n 128; \
	  if $(PERL) -v >/dev/null 2>&1; then \
	    case `cd '$(top_srcdir)'; pwd` in `pwd`) : ;; \
	      *) test -d tests || mkdir tests; \
		 rm -f srctests; \
		 if ln -s '$(top_srcdir)/tests' srctests; then \
		   for f in run_make_tests run_make_tests.pl test_driver.pl scripts jhelp.pl; do \
		     rm -f tests/$$f; ln -s ../srctests/$$f tests; \
		   done; fi ;; \
	    esac; \
	    echo "cd tests && $(PERL) $(PERLFLAGS) ./run_make_tests.pl -srcdir $(abs_top_srcdir)/src -make ../make$(EXEEXT) $(MAKETESTFLAGS)"; \
	    cd tests && $(PERL) $(PERLFLAGS) ./run_make_tests.pl -srcdir '$(abs_top_srcdir)/src' -make '../make$(EXEEXT)' $(MAKETESTFLAGS); \
	  else \
	    echo "Can't find a working Perl ($(PERL)); the test suite requires Perl."; \
	  fi; \
	else \
	  echo "Can't find the GNU Make test suite ($(top_srcdir)/tests)."; \
	fi

#: Generate the ReadTheDocs manual in HTML format
readthedocs:
	$(MAKE) -C docs html

# --------------- Maintainer's Section

# Tell automake that I haven't forgotten about this file and it will be
# created before we build a distribution (see maintMakefile in the Git
# distribution).

README:

@MAINT_MAKEFILE@

# FIXME fix up to use the git2cl in MAINT_MAKEFILE
GIT2CL = ./git2cl
if MAINTAINER_MODE

@MAINTAINER_MODE_TRUE@#: create ChangeLog for distribution from 'git log'
ChangeLog:
	git log --pretty --numstat --summary | $(GIT2CL) > $@
#:

endif
