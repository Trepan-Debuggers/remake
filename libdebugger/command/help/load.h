/** \file libdebugger/command/help/load.h
 *
 *  \brief Help text for debugger command `load`.
 *
 */
#define load_HELP_TEXT							\
  "Read in and evaluate GNU Makefile *file-glob*.\n"			\
  "\n"									\
  "*file-glob* should resolve after glob expansion to single GNU\n"	\
  "Makefile. Target dependencies are updated after reading in the file."
