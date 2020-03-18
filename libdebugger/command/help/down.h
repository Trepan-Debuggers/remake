/** \file libdebugger/command/help/down.h
 *
 *  \brief Help text for debugger command `down`.
 *
 */
#define down_HELP_TEXT \
  "Select and print the target this one caused to be examined.\n"	\
  "\n"									\
  "If *count* is given then select that many targets down; the default" \
  "is 1.\n"								\
  "\n"									\
  "When you enter the debugger this command doesn't make a lot of sense\n" \
  "because you are at the most-recently frame. However if you issue\n"	\
  "`down` and `frame` commands, this can change.\n"			\
  "\n"									\
  "See also:\n"								\
  "---------\n"								\
  "`up`, and `frame`"
