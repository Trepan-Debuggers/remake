#ifndef GLOBALS_H
#define GLOBALS_H

#include "types.h"
#include "variable.h"

#define OUTPUT_SYNC_NONE    0
#define OUTPUT_SYNC_LINE    1
#define OUTPUT_SYNC_TARGET  2
#define OUTPUT_SYNC_RECURSE 3

extern int env_overrides;

/* Nonzero means ignore status codes returned by commands
   executed to remake files.  Just treat them all as successful (-i).  */
extern int ignore_errors_flag;

/* Nonzero means don't remake anything, just print the data base
   that results from reading the makefile (-p).  */
extern int print_data_base_flag;

/* Nonzero means don't remake anything; just return a nonzero status
   if the specified targets are not up to date (-q).  */
extern int question_flag;

/* Nonzero means do not use any of the builtin rules (-r) / variables (-R).  */

extern int no_builtin_rules_flag;
extern int no_builtin_variables_flag;

/* Nonzero means check symlink mtimes.  */
extern int check_symlink_flag;

/* Nonzero means print directory before starting and when done (-w).  */
extern int print_directory_flag;

/* Nonzero means print version information.  */
extern int print_version_flag;

/*! Nonzero means --trace and shell trace with input.  */
extern int shell_trace;

/* Nonzero means profile calls (option --profile).  */
extern int profile_flag;

/* Nonzero means do extra verification (that may slow things down).  */
extern int verify_flag;

/* Nonzero means do not print commands to be executed (-s).  */
extern int silent_flag;

/* Nonzero means just touch the files
   that would appear to need remaking (-t)  */
extern int touch_flag;

/* Nonzero means just print what commands would need to be executed,
   don't actually execute them (-n).  */
extern int just_print_flag;

/*! If 1, we don't give additional error reporting information. */
extern int no_extended_errors;

extern int db_level;

/*! Value of the MAKELEVEL variable at startup (or 0).  */
extern unsigned int makelevel;

/*! Nonzero gives a list of explicit target names and exits. Set by option
  --targets
 */
extern int show_targets_flag;

/*! Nonzero gives a list of explicit target names that have commands
  associated with them and exits. Set by option --tasks
 */
extern int show_tasks_flag;

/*! If 1, same as --debugger=preaction */
extern int debugger_flag;

/** True if we are inside the debugger, false otherwise. */
extern int in_debugger;

/*! If true, enter the debugger before reading any makefiles. */
extern bool b_debugger_preread;

/* Remember the original value of the SHELL variable, from the environment.  */
struct variable shell_var;

extern unsigned int color_option;

#endif /*GLOBALS_H*/
