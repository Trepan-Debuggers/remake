# This is a -*-Makefile-*-, or close enough

AUTOMAKE_OPTIONS := dist-bzip2 check-news

transform := s,make,@MAKE_NAME@,

SUBDIRS :=	glob config po doc src emacs

man_MANS :=	doc/make.1

# Extra stuff to include in the distribution.
# Note we need all the glob stuff here, rather than in glob/Makefile.am,
# because often that directory isn't built on the systems used by the
# maintainers.

M4_DIST :=	gettext.m4 glibc21.m4 iconv.m4 isc-posix.m4 \
		intdiv0.m4 inttypes-pri.m4 inttypes.m4 inttypes_h.m4 \
		isc-posix.m4 lcmessage.m4 lib-ld.m4 lib-link.m4 lib-prefix.m4 \
		progtest.m4 stdint_h.m4 uintmax_t.m4 ulonglong.m4 


EXTRA_DIST :=	README build.sh.in $(man_MANS) \
		README.customs README.mdb getloadavg.c \
		README.W32 TODO ChangeLog.GNUMake $(M4_DIST) doc/make.1.in

MAKE_HOST :=	@MAKE_HOST@


test: check
# Forward targets

html:
	cd doc && $(MAKE) $(AM_MAKEFLAGS) $@

# --------------- Internationalization Section

localedir :=	$(datadir)/locale

# --------------- Local INSTALL Section

# If necessary, change the gid of the app and turn on the setgid flag.
#

# Whether or not make needs to be installed setgid.
# The value should be either `true' or `false'.
# On many systems, the getloadavg function (used to implement the `-l'
# switch) will not work unless make is installed setgid kmem.
#
inst_setgid = @NEED_SETGID@

# Install make setgid to this group so it can get the load average.
#
inst_group = @KMEM_GROUP@

install-exec-local:
	@if $(inst_setgid); then \
	   app=$(DESTDIR)$(bindir)/`echo $(bin_PROGRAMS)|sed '$(transform)'`; \
	   if chgrp $(inst_group) $$app && chmod g+s $$app; then \
	     echo "chgrp $(inst_group) $$app && chmod g+s $$app"; \
	   else \
	     echo "$$app needs to be owned by group $(inst_group) and setgid;"; \
	     echo "otherwise the \`-l' option will probably not work."; \
	     echo "You may need special privileges to complete the installation"; \
	     echo "of $$app."; \
	   fi; \
	 else true; fi

doxygen:
	-( cd ${top_srcdir}/doc/doxygen && /bin/sh ${srcdir}/run_doxygen )

# --------------- Local DIST Section

# Install the w32 and tests subdirectories
#
dist-hook:
	(cd $(srcdir); \
	 sub=`find w32 tests -follow \( -name CVS -prune -o -name .cvsignore -o -name work -prune \) -o \( -name \*.orig -o -name \*.rej -o -name \*~ -prune \) -o -type f -print`; \
	 tar chf - $$sub) \
	| (cd $(distdir); tar xfBp -)


# --------------- Local CHECK Section

check-local: check-regression 
	@banner=" Regression PASSED: GNU Make $(VERSION) ($(MAKE_HOST)) built with $(CC) "; \
	dashes=`echo "$$banner" | sed s/./=/g`; \
	echo; \
	echo "$$dashes"; \
	echo "$$banner"; \
	echo "$$dashes"; \
	echo

# check-regression
#
# Look for the make test suite, and run it if found and we can find perl.
# If we're building outside the tree, we use symlinks to make a local copy of
# the test suite.  Unfortunately the test suite itself isn't localizable yet.
#
MAKETESTFLAGS :=

check-regression:
	@if test -f "$(srcdir)/tests/run_make_tests"; then \
	  if $(PERL) -v >/dev/null 2>&1; then \
	    case `cd $(srcdir); pwd` in `pwd`) : ;; \
	      *) test -d tests || mkdir tests; \
		 rm -f srctests; \
		 if ln -s "$(srcdir)/tests" $(top_builddir)/srctests; then \
		   for f in run_make_tests run_make_tests.pl test_driver.pl scripts; do \
		     rm -f tests/$$f; ln -s $(top_builddir)/srctests/$$f tests; \
		   done; fi ;; \
	    esac; \
	    if test $(top_srcdir) = $(top_builddir) ; then \
	      echo "cd tests && $(PERL) ./run_make_tests.pl --make ../src/make$(EXEEXT) $(MAKETESTFLAGS)"; \
	      cd tests && $(PERL) ./run_make_tests.pl --make ../src/make$(EXEEXT) $(MAKETESTFLAGS); \
	    fi \
	  else \
	    echo "Can't find a working Perl ($(PERL)); the test suite requires Perl."; \
	  fi; \
	 else \
	  echo "Can't find the GNU Make test suite ($(srcdir)/tests)."; \
	 fi


# --------------- Maintainer's Section

# cvs2cl
MAINTAINERCLEANFILES = ChangeLog

@MAINT_MAKEFILE@

PHONY := html test

if MAINTAINER_MODE

PHONY += ChangeLog
ChangeLog:
	$(CVS2CL) -W 450 --header $(srcdir)/cvs2cl_header --utc -w -I ChangeLog --usermap $(srcdir)/cvs2cl_usermap -P 

ACLOCAL_AMFLAGS= -I . -I config

else 

ACLOCAL_AMFLAGS	=  -I config

endif

.PHONY: $(PHONY)
