# This is a -*-Makefile-*-, or close enough
#
# Copyright (C) 1997-2022, 2026 Free Software Foundation, Inc.
# This file is part of GNU ReMake.
#
# GNU Make is free software; you can redistribute it and/or modify it
# under the terms of the GNU General Public License as published by
# the Free Software # Foundation; either version 3 of the License, or
# (at your option) any later version.
#
# GNU ReMake is distributed in the hope that it will be useful, but WITHOUT ANY
# WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
# FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more
# details.
#
# You should have received a copy of the GNU General Public License along with
# this program.  If not, see <https://www.gnu.org/licenses/>.

AUTOMAKE_OPTIONS = 1.8 dist-lzip silent-rules std-options subdir-objects nostdinc
ACLOCAL_AMFLAGS = -I m4

transform = s,make,@MAKE_NAME@,

MAKE_HOST =	@MAKE_HOST@

SUBDIRS =	lib po doc glob unittest libdebugger

bin_PROGRAMS =	make
include_HEADERS = src/gnuremake.h

man_MANS =	doc/make.1

make_SRCS =	\
		src/ar.c src/arscan.c src/buildargv.c \
		src/commands.c src/debug.c src/default.c src/dir.c src/expand.c \
		src/file.c src/file_basic.c src/function.c src/getopt.c \
		src/getopt1.c src/globals.c src/guile.c src/hash.c src/implicit.c \
		src/job.c src/load.c src/loadapi.c src/main.c src/misc.c \
		src/output.c src/print.c src/profile.c src/read.c src/remake.c \
		src/rule.c src/rule.h src/signame.c src/strcache.c \
		src/types.h  src/trace.c src/variable.c src/version.c src/vpath.c \
		src/callgrind_format.c src/json_format.c \
		src/watch.c src/watch_inotify.c src/watch_poll.c \
		src/watchbackend.h

glob_SRCS =	lib/fnmatch.c lib/fnmatch.h lib/glob.c lib/glob.h

alloca_SRCS =	lib/alloca.c

loadavg_SRCS =	lib/getloadavg.c

w32_SRCS =	src/w32/pathstuff.c src/w32/w32os.c src/w32/compat/dirent.c \
		src/w32/compat/posixfcn.c src/w32/include/dirent.h \
		src/w32/include/dlfcn.h src/w32/include/pathstuff.h \
		src/w32/include/sub_proc.h src/w32/include/w32err.h \
		src/w32/subproc/misc.c src/w32/subproc/proc.h \
		src/w32/subproc/sub_proc.c src/w32/subproc/w32err.c

make_SOURCES =	$(make_SRCS) $(w32_SRCS)

EXTRA_make_SOURCES = src/mock.c

noinst_HEADERS = src/buildargv.h src/commands.h src/debug.h \
				src/dep.h src/expand.h src/file.h src/file_basic.h \
				src/filedef.h src/function.h src/getopt.h src/gettext.h \
				src/globals.h src/hash.h src/implicit.h src/job.h \
				src/main.h src/make.h src/makeint.h src/mock.h src/os.h \
				src/output.h src/print.h src/profile.h src/read.h \
				src/rule.h src/trace.h src/types.h src/variable.h src/vpath.h

if HAVE_GUILE
  _GUILE_CFLAGS = $(GUILE_CFLAGS)
  _GUILE_LIBS = $(GUILE_LIBS)
else
  _GUILE_CFLAGS =
  _GUILE_LIBS =
endif

make_LDADD =	$(LIBOBJS) $(GUILE_LIBS) lib/libgnu.a $(GETLOADAVG_LIBS) \
				libdebugger/libdebugger.a @LIBINTL@

localedir =	$(datadir)/locale

AM_CPPFLAGS =	-Isrc -I$(top_srcdir)/src -Ilib -I$(top_srcdir)/libdebugger \
		-I$(top_srcdir)/libdebugger -DLIBDIR=\"$(libdir)\" \
		-DINCLUDEDIR=\"$(includedir)\" -DLOCALEDIR=\"$(localedir)\"

AM_CFLAGS += $(GUILE_CFLAGS)

if WINDOWSENV
  make_SOURCES += $(w32_SRCS)
  AM_CPPFLAGS  += -I $(top_srcdir)/src/w32/include
  make_LDADD   += -lmingw32 
else
  make_SOURCES += src/posixos.c
endif

make_SOURCES += src/posixos.c

# FIXME: In order to make "make distcheck" work, we need to include
# things from the top lib directory. In particular this includes
# "lib/findprog.h" which is used by "src/jobs.c" for POSIX spawn.
DEFAULT_INCLUDES = -I$(top_srcdir)/lib -I.@am__isrc@ -I$(top_builddir)/src


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

EXTRA_DIST =	ChangeLog codespell-ignore.txt README $(man_MANS) \
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

.PHONY: check-regression codespell readthedocs

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
	$(MAKE) -C doc/readthedocs html

#: Generate the Doxygen source code documentation (in HTML format)
doxygen:
	cd doc/doxygen && $(SHELL) ./run_doxygen

# --------------- Maintainer's Section

# Tell automake that I haven't forgotten about this file and it will be
# created before we build a distribution (see maintMakefile in the Git
# distribution).

README:

@MAINT_MAKEFILE@

if MAINTAINER_MODE

@MAINTAINER_MODE_TRUE@#: create ChangeLog for distribution from 'git log'

# FIXME fix up to use the git2cl in MAINT_MAKEFILE
GIT2CL_PROG = ./git2cl

#: Run codspell over ChangeLog
codespell: codespell-ignore.txt ChangeLog
	codespell --ignore-words=codespell-ignore.txt ChangeLog

#: Remove ChangeLog
rmChangeLog:
	rm ChangeLog || true

#: Create ChangeLog from version control without corrections
ChangeLog-without-corrections:
	git log --pretty --numstat --summary | $(GIT2CL_PROG) >ChangeLog

#: Create ChangeLog from version control with additonal corrections
ChangeLog: ChangeLog-without-corrections
	patch ChangeLog < ChangeLog-spell-corrected.diff

endif
