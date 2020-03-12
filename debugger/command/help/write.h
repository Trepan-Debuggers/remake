#define write_HELP_TEXT							\
  "Writes the commands associated of a target to a file with MAKE\n"	\
  "variables expanded.\n"						\
  "\n"									\
  "If no target given, the basename of the current target is used.\n"	\
  "If a filename is supplied it is used. If it is the string\n"		\
  "\"here\", we write the output to stdout. If no filename is\n"	\
  "given then create the filename by prepending a directory name to\n"	\
  "the target name and then append \".sh\"."
