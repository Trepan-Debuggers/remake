#define target_HELP_TEXT						\
  "Show information about a target.\n"					\
  "\nThe following attributes names can be given after a target name:\n" \
  "  attributes - rule attributes: precious, rule search, and pattern stem\n" \
  "  commands   - shell commands that are run to update the target\n"	\
  "  expand     - like 'commands', but Makefile variables are expanded\n" \
  "  nonorder   - non-order dependencies\n"				\
  "  order      - order dependencies\n"					\
  "  depends    - all target dependencies, i.e. order and non-order\n"	\
  "  previous   - previous target name hwen there are multiple double-colons\n" \
  "  state      - shell command state\n"				\
  "  time       - last modification time and whether file has been updated\n" \
  "  variables  - automatically set variables such as  @ or  <\n"	\
  "\n"									\
  "TARGET-NAME can be a variable like `@' (current target) or `<'\n"	\
  "(first dependency). If no variable or target name is supplied\n"	\
  "we to use the current target name.\n"
