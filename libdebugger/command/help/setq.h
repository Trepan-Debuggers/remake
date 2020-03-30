/** \file libdebugger/command/help/setq.h
 *
 *  \brief Help text for debugger command `setq`.
 *
 */
#define setq_HELP_TEXT							\
  "Set MAKE variable *variable* to *value*.\n"				\
  "\n"									\
  "In contrast to `setqx`, variable definitions inside *value* are\n"	\
  "not expanded before assignment occurs.\n"				\
  "\n"									\
  "See also:\n"								\
  "---------\n"								\
  "`setqx` and `set`."
