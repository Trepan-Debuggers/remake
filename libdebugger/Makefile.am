noinst_HEADERS = debugger.h
noinst_LIBRARIES = libdebugger.a
libdebugger_a_SOURCES =  \
	break.c \
	cmd.c 	\
	fns.c 	\
	file2line.c 	\
	msg.c 	\
	stack.c

#: Reissue building this target in parent directory
doxygen readthedocs:
	$(MAKE) -C .. $@

#DEBUGGER_H = \
#	$(wildcard src/debugger/*.h) \
#	$(wildcard src/debugger/command/*.h) \
#	$(wildcard src/debugger/command/help/*.h)
#
