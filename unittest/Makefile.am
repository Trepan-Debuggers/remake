AM_CPPFLAGS =	-Isrc -I$(top_srcdir)/src -Ilib -I$(top_srcdir)/lib \
		-DLIBDIR=\"$(libdir)\" -DINCLUDEDIR=\"$(includedir)\" \
		-DLOCALEDIR=\"$(localedir)\"

test_profile_SOURCES = test_profile.c

test_profile_LDADD = \
	../src/mock.o \
	../src/version.o \
	../src/alloc.o \
	../src/globals.o \
	../src/misc.o \
	../src/output.o \
	../src/file_basic.o \
	../src/hash.o \
	../src/profile.o \
	../src/strcache.o \
	../src/posixos.o \
	../lib/libgnu.a \
	@LIBINTL@

check_PROGRAMS = test_profile

TESTS = $(check_PROGRAMS)

MOSTLYCLEANFILES = callgrind.out.*
