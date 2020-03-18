/** \file libdebugger/command/help/break.h
 *
 *  \brief Help text for debugger command `break`.
 *
 */
#define break_HELP_TEXT							\
  "Set a breakpoint at a target or line number; also show breakpoints.\n" \
  "\n"									\
  "With a target name or a line number, set a break before running commands\n" \
  "of that target or line number.  Without argument, list all breakpoints.\n" \
  "\n"									\
  "For a given target, there are 3 places where one may want to stop at;\n" \
  "that name can be given as a last option. The stopping points are:\n" \
  "\n"									\
  "- before target prerequisite checking: `prereq`\n"			\
  "- after target prerequisite checking but before running commands: `run`\n" \
  "- after target is complete: `end`\n"					\
  "\n"									\
  "Giving `all` will stop in all of the above places. The default behavior is `run`.\n" \
  "\n"									\
  "If no location specification is given, use the current target.\n"	\
  "\n"									\
  "Examples:\n"								\
  "---------\n"								\
  "\n"									\
  "   break               # list all breakpoints\n"			\
  "   break 10            # Break on line 10 of the Makefile we are\n"	\
  "                       # currently stopped at\n"			\
  "   break tests         # Break on the \"tests\" target\n"		\
  "   break tests prereq  # Break on the \"tests\" target before dependency checking is done\n" \
  "\n"									\
  "See also:\n"								\
  "---------\n"								\
  "`delete`"
