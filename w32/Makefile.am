# Makefile.am to create libw32.a for mingw32 host.

noinst_LIBRARIES = libw32.a

libw32_a_SOURCES =  subproc/misc.c subproc/sub_proc.c subproc/w32err.c \
		    pathstuff.c

libw32_a_CPPFLAGS = -I$(srcdir)/include -I$(srcdir)/subproc -I$(top_srcdir)
