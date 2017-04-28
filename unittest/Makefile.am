AM_CPPFLAGS  = -I$(abs_top_srcdir)
test_profile_SOURCES = test_profile.c

test_profile_LDADD = \
	../file_basic.o \
	../misc.o \
	../mock.o \
	../version.o \
	../globals.o \
	../hash.o \
	../profile.o \
	../strcache.o \
	@LIBINTL@

check_PROGRAMS = test_profile

TESTS = $(check_PROGRAMS)

MOSTLYCLEANFILES = callgrind.out.*
DISTCLEANFILES = mock.Po
