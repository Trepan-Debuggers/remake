AM_CPPFLAGS = -I$(top_srcdir)/src -I$(top_srcdir)/lib

test_profile_SOURCES = test_profile.c

test_profile_LDADD = \
	../src/version.o \
	../src/globals.o \
	../src/misc.o \
	../src/output.o \
	../src/file_basic.o \
	../src/hash.o \
	../src/profile.o \
	../src/strcache.o \
	../src/posixos.o \
	../src/json_format.o \
	../src/callgrind_format.o \
	../lib/libgnu.a \
	@LIBINTL@

check_PROGRAMS = test_profile

TESTS = $(check_PROGRAMS)

MOSTLYCLEANFILES = callgrind.out.* build.*.json
