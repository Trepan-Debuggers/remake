int env_overrides = 0;

/* Nonzero means ignore status codes returned by commands
   executed to remake files.  Just treat them all as successful (-i).  */

int ignore_errors_flag = 0;

/* Nonzero means don't remake anything, just print the data base
   that results from reading the makefile (-p).  */

int print_data_base_flag = 0;

/* Nonzero means don't remake anything; just return a nonzero status
   if the specified targets are not up to date (-q).  */

int question_flag = 0;

/* Nonzero means do not use any of the builtin rules (-r) / variables (-R).  */

int no_builtin_rules_flag = 0;
int no_builtin_variables_flag = 0;

/* Nonzero means check symlink mtimes.  */

int check_symlink_flag = 0;

/* Nonzero means print directory before starting and when done (-w).  */

int print_directory_flag = 0;

/* Nonzero means print version information.  */

int print_version_flag = 0;

/*! Nonzero means --trace and shell trace with input.  */

int shell_trace = 0;

/* Nonzero means do extra verification (that may slow things down).  */

int verify_flag;

/* Nonzero means do not print commands to be executed (-s).  */

int silent_flag;

/* Nonzero means just touch the files
   that would appear to need remaking (-t)  */

int touch_flag;

/* Nonzero means just print what commands would need to be executed,
   don't actually execute them (-n).  */

int just_print_flag;

/*! If 1, we don't give additional error reporting information. */
int no_extended_errors = 0;

int db_level = 0;

/*! Value of the MAKELEVEL variable at startup (or 0).  */
unsigned int makelevel;
