/** \file libdebugger/command/help/list.h
 *
 *  \brief Help text for debugger command `list`.
 *
 */
#define list_HELP_TEXT						       \
  "List target dependencies and commands for TARGET or LINE NUMBER.\n" \
  "Without a target name or line number, use the current target.\n"	\
  "A target name of '-' will use the parent target on the target stack.\n" \
  "\n"									\
  "See also:\n"								\
  "---------\n"								\
  "`target`, `edit`"
