/** \file libdebugger/command/help/continue.h
 *
 *  \brief Help text for debugger command `continue`.
 *
 */
#define continue_HELP_TEXT						\
  "Continue executing debugged Makefile until another breakpoint or\n"	\
  "stopping point. If a target is given and valid we set a temporary\n" \
  "breakpoint at that target before continuing.\n"			\
  "\n"									\
  "When a target name is given, breakpoint properties can be given after\n" \
  "the target name\n"							\
  "\n"									\
  "See also:\n"								\
  "`break` and `finish`.\n"
