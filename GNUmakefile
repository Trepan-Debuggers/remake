# GNU Make-specific makefile for GNU Make.

# Copyright (C) 1990, 1991, 1992 Free Software Foundation, Inc.
# This file is part of GNU Make.
# 
# GNU Make is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 2, or (at your option)
# any later version.
#
# GNU Make is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
# 
# You should have received a copy of the GNU General Public License
# along with GNU Make; see the file COPYING.  If not, write to
# the Free Software Foundation, 675 Mass Ave, Cambridge, MA 02139, USA.

.PHONY: default
default:

# Get most of the information from the Unix-compatible makefile.
include compatMakefile

# Set `ARCH' to a string for the type of machine.
ifndef ARCH
ifdef machine
ARCH = $(machine)
endif # machine
endif # not ARCH

ifdef ARCH

# We know the type of machine, so put the binaries in subdirectories.
$(ARCH)/%.o: %.c
	$(COMPILE.c) $< $(OUTPUT_OPTION)
objs := $(addprefix $(ARCH)/,$(objs))
prog := $(ARCH)/make

archpfx = $(ARCH)/

$(archpfx)load.o: load.c
	$(COMPILE.c) $(LOAD_AVG) $< -o $@
$(archpfx)load.dep: load.c
	$(mkdep) $(LOAD_AVG) $< | sed 's,$*\.o,& $@,' > $@
$(archpfx)remote.o: remote.c
	$(COMPILE.c) $(REMOTE) $< -o $@
$(archpfx)remote.dep: remote.c
	$(mkdep) $(REMOTE) $< | sed 's,$*\.o,& $@,' > $@

CPPFLAGS = $(defines)

ifneq "$(wildcard $(ARCH)/makefile)" ""
include $(ARCH)/makefile
objs := $(sort $(objs) $(addprefix $(ARCH)/,$(ALLOCA) $(extras)))
endif

ifndef no_libc
libc_dir = /home/gd2/gnu/libc/$(ARCH)
ifneq ($(wildcard $(libc_dir)),)
ifneq ($(wildcard $(libc_dir)/works-for-make),)
#CPPFLAGS := -I$(libc_dir)/include
#LDFLAGS := -nostdlib $(libc_dir)/lib/start.o
#LOADLIBES := $(LOADLIBES) \
#	     $(libc_dir)/lib/mcheck-init.o \
#	     $(libc_dir)/lib/libc.a \
#	     -lgcc \
#	     $(libc_dir)/lib/libc.a
CC := $(CC) -b glibc

# glob is in the C library.
globdep = $(libc_dir)/lib/libc.a
globlib =

# So is getopt.
GETOPT =

else

globdep = glob/libglob.a
globlib = $(globdep)
CPPFLAGS := $(CPPFLAGS) -Iglob

endif
endif
endif

else # Not ARCH
prog := make
endif

ifneq	"$(findstring gcc,$(CC))" ""
CFLAGS = -g -W -Wunused -Wpointer-arith -Wreturn-type -Wswitch
else
CFLAGS = -g
endif

ifdef yescustoms
REMOTE := -DCUSTOMS
LOADLIBES := libcustoms.a
endif

# Define the command to make dependencies.
ifneq	"$(findstring gcc,$(CC))" ""
# Don't include system directories.
mkdep-nolib = $(CC) -MM $(CPPFLAGS)
else
mkdep-nolib = $(mkdep)
endif
mkdep = $(CC) -M $(CPPFLAGS)

depfiles = $(objs:.o=.dep)


ifdef yescustoms
prog := $(prog)-customs
endif

.PHONY: default
default: $(prog)

$(prog): $(objs) $(globdep)
	$(CC) $(LDFLAGS) $^ $(globlib) $(LOADLIBES) -o $@.new
	mv -f $@.new $@

globfiles = $(addprefix glob/,COPYING.LIB Makefile \
			glob.c fnmatch.c glob.h fnmatch.h)
$(globfiles): /home/gd/gnu/libc/posix/glob.tar
	tar xvfm $< $@
/home/gd/gnu/libc/posix/glob.tar: force
	$(MAKE) -C $(@D) $(@F) no_deps=t
.PHONY: force
force:

# Make the Unix-compatible Makefile to be distributed by appending
# the automatically-generated dependencies to compatMakefile.
ifeq ($(mkdep),$(mkdep-nolib))
nolib-deps = $(depfiles)
else
%.dep: %.c
	$(mkdep-nolib) $< | sed 's,$*\.o,$(@:.dep=.o) $@,' > $@
ifdef	archpfx
load.dep: load.c
	$(mkdep-nolib) $(LOAD_AVG) $< | sed 's,$*\.o,& $@,' > $@
remote.dep: remote.c
	$(mkdep-nolib) $(REMOTE) $< | sed 's,$*\.o,& $@,' > $@
endif
nolib-deps = $(patsubst $(archpfx)%,%,$(depfiles))
endif
Makefile: compatMakefile $(nolib-deps)
	(cat $<; \
	 echo '# Automatically generated dependencies.'; \
	 sed -e 's/ [^ ]*\.dep//' -e 's=$(archpfx)==' $(filter-out $<,$^) \
	) > $@

.SUFFIXES: .dep
# Maintain the automatically-generated dependencies.
ifndef	   no_deps
include $(archpfx)depend
$(archpfx)depend: GNUmakefile compatMakefile
	(for file in $(depfiles); \
	 do echo include $$file; done) > $@
$(archpfx)%.dep: %.c
	$(mkdep) $< | sed 's,$*\.o,$(@:.dep=.o) $@,' > $@
endif

ETAGS = etags -T # for v19 etags

# Run the tests.
.PHONY: tests
testdir := $(shell ls -d1 make-test-?.? | sort -n +0.10 -0.11 +0.12 | tail -1l)
tests: $(testdir)/run_make_tests.pl $(prog)
	cd $(<D); perl $(<F)

# Make the distribution tar files.

.PHONY: dist
# Figure out the version number from the source of `version.c'.
version := \
  $(strip $(shell sed -e '/=/!d' -e 's/^.*"\(.*\)";$$/\1/' < version.c))
tarfiles := make make-doc
tarfiles := $(addsuffix -$(version).tar.Z,$(tarfiles))
# Depend on default and doc so we don't ship anything that won't compile.
dist: default doc tests $(tarfiles)

dist: local-inst
.PHONY: local-inst
local-inst: $(prog)
	install -c -g kmem -o $(USER) -m 2755 $< /usr/local/gnubin/make

# Put the alpha distribution files in the anonymous FTP directory.
alpha-files = $(tarfiles) GNUmakefile compatMakefile $(testdir).tar.Z
dist: alpha
.PHONY: alpha
alpha-dir := ~ftp/gnu
alpha-files := $(addprefix $(alpha-dir)/,$(alpha-files))
alpha: $(alpha-dir) $(alpha-files)
$(alpha-dir)/%: %
	@rm -f $@
	cp $< $@

# Implicit rule to make README and README-doc.
%: %.template version.c
	sed 's/VERSION/$(version)/' < $< > $@

define make-tar
@rm -fr make-$(version)
ln -s . make-$(version)
tar cvhf $(@:.Z=) $(addprefix make-$(version)/,$^)
rm -f make-$(version)
compress -f $(@:.Z=)
endef

make-doc-$(version).tar.Z: README-doc COPYING make.dvi make.info make.info*
	$(make-tar)
make-$(version).tar.Z: README COPYING ChangeLog CHANGES TAGS tags Makefile \
	  $(srcs) remote-*.c $(globfiles) make.texinfo gpl.texinfo \
  	  make.cp* make.fn* make.ky* make.pg* make.toc make.tp* make.vr* \
	  make.aux make.man texinfo.tex
	$(make-tar)

ifneq (,)
tests := $(filter-out %~,$(wildcard tests/*))
make-tests-$(version).tar.Z: $(tests)
	@rm -fr make-tests-$(version)
	ln -s tests make-tests-$(version)
	tar cvhf $(@:.Z=) $(patsubst tests/%,make-tests-$(version)/%,$^)
	rm -f make-tests-$(version)
	compress -f $(@:.Z=)
endif

$(archpfx)loadtest: $(archpfx)load.o
