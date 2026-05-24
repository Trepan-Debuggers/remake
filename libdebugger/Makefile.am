noinst_HEADERS = \
	break.h \
	cmd.h \
	cmd_initialize.h \
	commands.h \
	debugger.h \
	command/help/break.h \
	command/help/chdir.h \
	command/help/comment.h \
	command/help/continue.h \
	command/help/delete.h \
	command/help/down.h \
	command/help/edit.h \
	command/help/expand.h \
	command/help/finish.h \
	command/help/frame.h \
	command/help/help.h \
	command/help/info.h \
	command/help/list.h \
	command/help/load.h \
	command/help/next.h \
	command/help/print.h \
	command/help/pwd.h \
	command/help/quit.h \
	command/help/run.h \
	command/help/set.h \
	command/help/setq.h \
	command/help/setqx.h \
	command/help/shell.h \
	command/help/show.h \
	command/help/skip.h \
	command/help/source.h \
	command/help/step.h \
	command/help/target.h \
	command/help/up.h \
	command/help/watch.h \
	command/help/where.h \
	command/help/write.h \
	file2line.h \
	subcmd.h

noinst_LIBRARIES = libdebugger.a

# Because there is both a break.c and a command/break.c we need
# to specify the latter explicitly.
DEBUGGER_COMMANDS_C = \
	command/break.c \
	command/info.c \
	$(wildcard command/*.h)
	$(wildcard command/*.c)

libdebugger_a_SOURCES =  \
	$(DEBUGGER_COMMANDS_C) \
	command/chdir.c \
	command/comment.c \
	command/continue.c \
	command/delete.c \
	command/down.c \
	command/edit.c \
	command/expand.c \
	command/finish.c \
	command/frame.c \
	command/help.c \
	command/list.c \
	command/next.c \
	command/load.c \
	command/print.c \
	command/pwd.c \
	command/quit.c \
	command/run.c \
	command/set.c \
	command/setq.c \
	command/setqx.c \
	command/shell.c \
	command/show.c \
	command/skip.c \
	command/source.c \
	command/step.c \
	command/target.c \
	command/up.c \
	command/watch.c \
	command/where.c \
	command/write.c \
	break.c \
	cmd.c 	\
	fns.c 	\
	fns.h \
	file2line.c 	\
	info.h \
	msg.c 	\
	msg.h \
	stack.c \
	stack.h

# FIXME: In order to make "make distcheck" work, we need to include
# things from the top src directory. In particular this includes
# "src/gettext.h" which is used by "src/makeint.h" when <gettext.h> is not
# found.
#
# Some debugger commands include "makeint.h".
DEFAULT_INCLUDES = -I$(top_srcdir)/src -I.@am__isrc@ -I$(top_builddir)/src

#: Reissue building this target in parent directory
doxygen readthedocs:
	$(MAKE) -C .. $@
