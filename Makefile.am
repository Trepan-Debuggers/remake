# This is a -*-Makefile-*-, or close enough
#
# Copyright (C) 1997-2014 Free Software Foundation, Inc.
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

AUTOMAKE_OPTIONS = 1.8 dist-bzip2 check-news
ACLOCAL_AMFLAGS	=  -I config

transform = s,make,@MAKE_NAME@,

MAKE_HOST =	@MAKE_HOST@

# Only process if target is MS-Windows
if WINDOWSENV
  MAYBE_W32 =	w32
  W32INC =	-I $(top_srcdir)/w32/include
  W32LIB =	-Lw32 -lw32
endif

SUBDIRS =	glob config po doc $(MAYBE_W32)

bin_PROGRAMS =	make
include_HEADERS = gnumake.h

if USE_CUSTOMS
  remote =	remote-cstms.c
else
  remote =	remote-stub.c
endif

DEBUGGER_SRC = 	\
		debugger/break.c \
		debugger/cmd.c 	\
		debugger/fns.c 	\
		debugger/file2line.c 	\
		debugger/msg.c 	\
		debugger/stack.c

DEBUGGER_H = \
		debugger/break.h 		\
		debugger/cmd.h 			\
		debugger/fns.h 			\
		debugger/msg.h 			\
		debugger/command/break.h 	\
		debugger/command/chdir.h  	\
		debugger/command/comment.h 	\
		debugger/command/continue.h	\
		debugger/command/delete.h 	\
		debugger/command/down.h 	\
		debugger/command/edit.h 	\
		debugger/command/eval.h 	\
		debugger/command/expand.h	\
		debugger/command/finish.h	\
		debugger/command/frame.h	\
		debugger/command/help.h		\
		debugger/command/info.h		\
		debugger/command/list.h		\
		debugger/command/next.h		\
		debugger/command/print.h	\
		debugger/command/pwd.h		\
		debugger/command/quit.h		\
		debugger/command/run.h		\
		debugger/command/set.h		\
		debugger/command/setq.h		\
		debugger/command/shell.h	\
		debugger/command/show.h		\
		debugger/command/skip.h		\
		debugger/command/source.h	\
		debugger/command/step.h		\
		debugger/command/target.h	\
		debugger/command/up.h		\
		debugger/command/where.h	\
		debugger/command/write.h	\
		debugger/file2line.h 	        \
		debugger/fns.h 			\
		debugger/info.h			\
		debugger/stack.h		\
		debugger/subcmd.h

make_SOURCES =	ar.c arscan.c commands.c default.c dir.c expand.c file.c \
		function.c getopt.c getopt1.c guile.c implicit.c job.c load.c \
		loadapi.c main.c misc.c output.c print.c read.c remake.c rule.c \
		signame.c strcache.c variable.c version.c vpath.c hash.c \
		buildargv.c debug.c trace.c \
		$(remote)

EXTRA_make_SOURCES = vmsjobs.c remote-stub.c remote-cstms.c

noinst_HEADERS = commands.h dep.h filedef.h job.h makeint.h rule.h variable.h \
		debug.h getopt.h gettext.h hash.h output.h

make_LDADD =	@LIBOBJS@ @ALLOCA@ $(GLOBLIB) @GETLOADAVG_LIBS@ @LIBINTL@ \
		$(GUILE_LIBS)
# Only process if target is MS-Windows
if WINDOWSENV
    make_LDADD += $(W32LIB)
endif

man_MANS =	make.1

DEFS =		-DLOCALEDIR=\"$(localedir)\" -DLIBDIR=\"$(libdir)\" -DINCLUDEDIR=\"$(includedir)\" @DEFS@

AM_CPPFLAGS =	$(GLOBINC)
AM_CFLAGS =	$(GUILE_CFLAGS)
# Only process if target is MS-Windows
if WINDOWSENV
    AM_CPPFLAGS +=	$(W32INC)
endif


# Extra stuff to include in the distribution.

EXTRA_DIST =	README build.sh.in $(man_MANS) \
		README.customs README.OS2 \
		SCOPTIONS SMakefile \
		README.Amiga Makefile.ami config.ami make.lnk amiga.c amiga.h \
		README.DOS Makefile.DOS configure.bat dosbuild.bat configh.dos\
		README.W32 NMakefile config.h.W32 build_w32.bat subproc.bat \
		make_msvc_net2003.sln make_msvc_net2003.vcproj \
		README.VMS makefile.vms makefile.com config.h-vms \
		vmsdir.h vmsfunctions.c vmsify.c \
		gmk-default.scm gmk-default.h

# This is built during configure, but behind configure's back

DISTCLEANFILES = build.sh

MOSTLYCLEANFILES = *.orig *.rej @MAKE_NAME@

# Forward targets

html-local:
	cd doc && $(MAKE) $(AM_MAKEFLAGS) $@

make$(EXEEXT): $(make_OBJECTS) $(make_DEPENDENCIES)
	@rm -f make$(EXEEXT) @MAKE_NAME@$(EXEEXT)
	$(LINK) $(make_OBJECTS) $(make_LDADD) $(LIBS)
	$(LN_S) make$(EXEEXT) @MAKE_NAME@$(EXEEXT)

.PHONY: html

# --------------- Internationalization Section

localedir =	$(datadir)/locale

# --------------- Local INSTALL Section

# If necessary, change the gid of the app and turn on the setgid flag.
#

# Whether or not make needs to be installed setgid.
# The value should be either 'true' or 'false'.
# On many systems, the getloadavg function (used to implement the '-l'
# switch) will not work unless make is installed setgid kmem.
#
inst_setgid = @NEED_SETGID@

# Install make setgid to this group so it can get the load average.
#
inst_group = @KMEM_GROUP@

#: install executables (e.g. remake)
install-exec-local:
	@if $(inst_setgid); then \
	   app=$(DESTDIR)$(bindir)/`echo $(bin_PROGRAMS)|sed '$(transform)'`; \
	   if chgrp $(inst_group) $$app && chmod g+s $$app; then \
	     echo "chgrp $(inst_group) $$app && chmod g+s $$app"; \
	   else \
	     echo "$$app needs to be owned by group $(inst_group) and setgid;"; \
	     echo "otherwise the '-l' option will probably not work."; \
	     echo "You may need special privileges to complete the installation"; \
	     echo "of $$app."; \
	   fi; \
	 else true; fi

# --------------- Generate the Guile default module content

guile.$(OBJEXT): gmk-default.h
gmk-default.h: $(srcdir)/gmk-default.scm
	(echo 'static const char *const GUILE_module_defn = " '\\ \
	  && sed -e 's/;.*//' -e '/^[ \t]*$$/d' -e 's/"/\\"/g' -e 's/$$/ \\/' \
	         $(srcdir)/gmk-default.scm \
          && echo '";') > $@

# --------------- Local DIST Section

#: Install the w32 and tests subdirectories
#
dist-hook:
	(cd $(srcdir); \
	 sub=`find w32 tests -follow \( -name .git -o -name .deps -o -name work -o -name .gitignore -o -name \*.orig -o -name \*.rej -o -name \*~ -o -name Makefile \) -prune -o -type f -print`; \
	 tar chf - $$sub) \
	| (cd $(distdir); tar xfBp -)


# --------------- Local CHECK Section

check-local: check-regression check-loadavg
	@banner=" Regression PASSED: GNU Make $(VERSION) ($(MAKE_HOST)) built with $(CC) "; \
	dashes=`echo "$$banner" | sed s/./=/g`; \
	echo; \
	echo "$$dashes"; \
	echo "$$banner"; \
	echo "$$dashes"; \
	echo

.PHONY: check-loadavg check-regression

#: Check internal load average routine against 'uptime'
check-loadavg: loadavg$(EXEEXT)
	@echo The system uptime program believes the load average to be:
	-uptime
	@echo The GNU load average checking code thinks:
	-./loadavg$(EXEEXT)

# FIXME: doesn't work.
##: Run remake-specific tests using Ruby rspec
#check-rspec:
#	$(SHELL) -c 'ruby tests/spec/run-tests.rb'

# The loadavg function is invoked during "make check" to test getloadavg.
check_PROGRAMS = loadavg
nodist_loadavg_SOURCES = getloadavg.c
loadavg_CPPFLAGS = -DTEST
loadavg_LDADD = @GETLOADAVG_LIBS@

# > check-regression
#
# Look for the make test suite, and run it if found and we can find perl.
# If we're building outside the tree, we use symlinks to make a local copy of
# the test suite.  Unfortunately the test suite itself isn't localizable yet.
#
MAKETESTFLAGS =

#: run regression tests in tests/scripts
check-regression: tests/config-flags.pm
	@if test -f '$(srcdir)/tests/run_make_tests'; then \
	  if $(PERL) -v >/dev/null 2>&1; then \
	    case `cd '$(srcdir)'; pwd` in `pwd`) : ;; \
	      *) test -d tests || mkdir tests; \
		 rm -f srctests; \
		 if ln -s '$(srcdir)/tests' srctests; then \
		   for f in run_make_tests run_make_tests.pl test_driver.pl scripts; do \
		     rm -f tests/$$f; ln -s ../srctests/$$f tests; \
		   done; fi ;; \
	    esac; \
	    echo "cd tests && $(PERL) ./run_make_tests.pl -srcdir $(abs_srcdir) -make ../make$(EXEEXT) $(MAKETESTFLAGS)"; \
	    cd tests && $(PERL) ./run_make_tests.pl -srcdir '$(abs_srcdir)' -make '../make$(EXEEXT)' $(MAKETESTFLAGS); \
	  else \
	    echo "Can't find a working Perl ($(PERL)); the test suite requires Perl."; \
	  fi; \
	 else \
	  echo "Can't find the GNU Make test suite ($(srcdir)/tests)."; \
	 fi

%.short:
	$(MAKE) $(@:.short=) 2>&1 >/dev/null

# --------------- Maintainer's Section

# Tell automake that I haven't forgotten about this file and it will be
# created before we build a distribution (see maintMakefile in the Git
# distribution).

if MAINTAINER_MODE

#: create ChangeLog for distirbution from 'git log'
ChangeLog:
	git log --pretty --numstat --summary | $(GIT2CL) > $@

endif

README:

@MAINT_MAKEFILE@
