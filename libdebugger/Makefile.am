noinst_HEADERS = commands.h debugger.h
noinst_LIBRARIES = libdebugger.a

# Because there is both a break.c and a command/break.c we need
# to specify the latter explicitly.
DEBUGGER_COMMANDS_C = \
	command/break.c \
	command/info.c \
	$(wildcard command/*.c)

libdebugger_a_SOURCES =  \
	$(DEBUGGER_COMMANDS_C) \
	command/chdir.c \
	command/comment.c \
	break.c \
	cmd.c 	\
	fns.c 	\
	file2line.c 	\
	msg.c 	\
	stack.c

#: Reissue building this target in parent directory
doxygen readthedocs:
	$(MAKE) -C .. $@
