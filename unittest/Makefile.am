AM_CPPFLAGS  = -I$(abs_top_srcdir)
test_profile_SOURCES = test_profile.c

test_profile_LDADD = \
	../mock.o \
	../version.o \
	../alloc.o \
	../globals.o \
	../misc.o \
	../output.o \
	../file_basic.o \
	../hash.o \
	../profile.o \
	../strcache.o \
	@LIBINTL@

check_PROGRAMS = test_profile

TESTS = $(check_PROGRAMS)

MOSTLYCLEANFILES = callgrind.out.*
