#define quit_HELP_TEXT							\
  "Exit make. If a numeric argument is given, it will be the exit\n"	\
  "status reported back. A status of 77 in a nested make will signals\n" \
  "termination in the parent. So if no numeric argument is given and\n" \
  "MAKELEVEL is 0, then status 0 is set; otherwise it is 77."		\
  "Select and print the immediate child dependency target that is\n"	\
  "currently under consideration.\n"					\
  "\n"									\
  "If *count* is the default is 1.\n"					\
  "\n"									\
  "See also:\n"								\
  "---------\n"								\
  "`down`, and `frame`"
