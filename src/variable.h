/* Definitions for using variables in GNU Make.
Copyright (C) 1988-2020 Free Software Foundation, Inc.
This file is part of GNU Make.

GNU Make is free software; you can redistribute it and/or modify it under the
terms of the GNU General Public License as published by the Free Software
Foundation; either version 3 of the License, or (at your option) any later
version.

GNU Make is distributed in the hope that it will be useful, but WITHOUT ANY
WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR
A PARTICULAR PURPOSE.  See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License along with
this program.  If not, see <http://www.gnu.org/licenses/>.  */

#ifndef VARIABLE_H
#define VARIABLE_H

#include "gnuremake.h"
#include "hash.h"

/* Codes in a variable definition saying where the definition came from.
   Increasing numeric values signify less-overridable definitions.  */
enum variable_origin
  {
    o_default,          /* Variable from the default set.  */
    o_env,              /* Variable from environment.  */
    o_file,             /* Variable given in a makefile.  */
    o_env_override,     /* Variable from environment, if -e.  */
    o_command,          /* Variable given by user.  */
    o_override,         /* Variable from an 'override' directive.  */
    o_automatic,        /* Automatic variable -- cannot be set.  */
    o_debugger,  	/* Set inside debugger.  */
    o_invalid           /* Core dump time.  */
  };

enum variable_flavor
  {
    f_bogus,            /* Bogus (error) */
    f_simple,           /* Simple definition (:= or ::=) */
    f_recursive,        /* Recursive definition (=) */
    f_append,           /* Appending definition (+=) */
    f_conditional,      /* Conditional definition (?=) */
    f_shell,            /* Shell assignment (!=) */
    f_append_value      /* Append unexpanded value */
  };

/* Structure that represents one variable definition.
   Each bucket of the hash table is a chain of these,
   chained through 'next'.  */

#define EXP_COUNT_BITS  15      /* This gets all the bitfields into 32 bits */
#define EXP_COUNT_MAX   ((1<<EXP_COUNT_BITS)-1)

struct variable
  {
    char *name;                 /* Variable name.  */
    char *value;                /* Variable value.  */
    gmk_floc fileinfo;              /* Where the variable was defined.  */
    unsigned int length;        /* strlen (name) */
    unsigned int recursive:1;   /* Gets recursively re-evaluated.  */
    unsigned int append:1;      /* Nonzero if an appending target-specific
                                   variable.  */
    unsigned int conditional:1; /* Nonzero if set with a ?=. */
    unsigned int per_target:1;  /* Nonzero if a target-specific variable.  */
    unsigned int special:1;     /* Nonzero if this is a special variable.  */
    unsigned int exportable:1;  /* Nonzero if the variable _could_ be
                                   exported.  */
    unsigned int expanding:1;   /* Nonzero if currently being expanded.  */
    unsigned int private_var:1; /* Nonzero avoids inheritance of this
                                   target-specific variable.  */
    unsigned int exp_count:EXP_COUNT_BITS;
                                /* If >1, allow this many self-referential
                                   expansions.  */
    enum variable_flavor
      flavor ENUM_BITFIELD (4); /* Variable flavor.  */
    enum variable_origin
      origin ENUM_BITFIELD (4); /* Variable origin.  */
    enum variable_export
      {
        v_export,               /* Export this variable.  */
        v_noexport,             /* Don't export this variable.  */
        v_ifset,                /* Export it if it has a non-default value.  */
        v_default               /* Decide in target_environment.  */
      } export ENUM_BITFIELD (2);
  };

/* Structure that represents a variable set.  */

struct variable_set
  {
    struct hash_table table;    /* Hash table of variables.  */
  };

/* Structure that represents a list of variable sets.  */

struct variable_set_list
  {
    struct variable_set_list *next;     /* Link in the chain.  */
    struct variable_set *set;           /* Variable set.  */
    int next_is_parent;                 /* True if next is a parent target.  */
  };

/* Structure used for pattern-specific variables.  */

struct pattern_var
  {
    struct pattern_var *next;
    const char *suffix;
    const char *target;
    size_t len;
    struct variable variable;
  };

extern char *variable_buffer;
extern struct variable_set_list *current_variable_set_list;
extern struct variable *default_goal_var;
extern struct variable shell_var;

/* expand.c */
#ifndef SIZE_MAX
# define SIZE_MAX ((size_t)~(size_t)0)
#endif

char *variable_buffer_output (char *ptr, const char *string, size_t length);
char *variable_expand (const char *line);
char *variable_expand_for_file (const char *line, struct file *file);
char *allocated_variable_expand_for_file (const char *line, struct file *file);
#define allocated_variable_expand(line) \
  allocated_variable_expand_for_file (line, (struct file *) 0)
char *expand_argument (const char *str, const char *end);
char *variable_expand_string (char *line, const char *string, size_t length);
void install_variable_buffer (char **bufp, size_t *lenp);
void restore_variable_buffer (char *buf, size_t len);

/* function.c */
int handle_function (char **op, const char **stringp);
int pattern_matches (const char *pattern, const char *percent, const char *str);
char *subst_expand (char *o, const char *text, const char *subst,
                    const char *replace, size_t slen, size_t rlen,
                    int by_word);
char *patsubst_expand_pat (char *o, const char *text, const char *pattern,
                           const char *replace, const char *pattern_percent,
                           const char *replace_percent);
char *patsubst_expand (char *o, const char *text, char *pattern, char *replace);
char *func_shell_base (char *o, char **argv, int trim_newlines);
void shell_completed (int exit_code, int exit_sig);

/* expand.c */
char *recursively_expand_for_file (struct variable *v, struct file *file);
#define recursively_expand(v)   recursively_expand_for_file (v, NULL)

/* variable.c */
struct variable_set_list *create_new_variable_set (void);

/*!
  Return a string describing origin.
 */
const char *origin2str(variable_origin_t origin);

void free_variable_set (struct variable_set_list *);

/*! Create a new variable set, push it on the current setlist,
  and assign current_variable_set_list to it.
 */
struct variable_set_list *push_new_variable_scope (void);

/*! Pop the top set off the current_variable_set_list, and free all
   its storage.  If b_toplevel set we have the top-most global scope
   and some things don't get freed because they weren't malloc'd.
*/
void pop_variable_scope (void);

/*! Define the automatic variables, and record the addresses of their
  structures so we can change their values quickly.  */
void define_automatic_variables (void);

/*! Initialize FILE's variable set list.  If FILE already has a
   variable set list, the topmost variable set is left intact, but the
   the rest of the chain is replaced with FILE->parent's setlist.  If
   FILE is a double-colon rule, then we will use the "root"
   double-colon target's variable set as the parent of FILE's variable
   set.

   If we're READing a makefile, don't do the pattern variable search now,
   since the pattern variable might not have been defined yet.  */
void initialize_file_variables (struct file *file, int reading);

/*! Print all the local variables of P_TARGET.  Lines output have "# "
    prepended. If you want hash table statistics too, set b_hash_stats
    true.
*/
extern void print_file_variables(const file_t *p_target, bool b_hash_stats);

/*! Print the data base of variables.  */

extern void print_variable_data_base (void);

/** Print information for variable V, prefixing it with PREFIX.  */
extern void print_variable_info (const void *item, void *arg);

void print_target_variables (const struct file *file);

/*! Print all the variables in SET.  PREFIX is printed before the
   actual variable definitions (everything else is comments).  If you
   want hash table statistics too, set b_hash_stats true.
*/
void print_variable_set (struct variable_set *set, const char *prefix, int pauto);

void merge_variable_set_lists (struct variable_set_list **to_list,
                               struct variable_set_list *from_list);
struct variable *do_variable_definition (const gmk_floc *flocp,
                                         const char *name, const char *value,
                                         enum variable_origin origin,
                                         enum variable_flavor flavor,
                                         int target_var);
char *parse_variable_definition (const char *line,
                                 struct variable *v);
struct variable *assign_variable_definition (struct variable *v, const char *line);
struct variable *try_variable_definition (const gmk_floc *flocp, const char *line,
                                          enum variable_origin origin,
                                          int target_var);
void init_hash_global_variable_set (void);
void hash_init_function_table (void);
void define_new_function(const gmk_floc *flocp, const char *name,
                         unsigned int min, unsigned int max, unsigned int flags,
                         gmk_func_ptr func);
struct variable *lookup_variable (const char *name, size_t length);
struct variable *lookup_variable_in_set (const char *name, size_t length,
                                         const struct variable_set *set);

struct variable *define_variable_in_set (const char *name, size_t length,
                                         const char *value,
                                         enum variable_origin origin,
                                         int recursive,
                                         struct variable_set *set,
                                         const gmk_floc *flocp);

/* Define a variable in the current variable set.  */

#define define_variable(n,l,v,o,r) \
          define_variable_in_set((n),(l),(v),(o),(r),\
                                 current_variable_set_list->set,NILF)

/* Define a variable with a constant name in the current variable set.  */

#define define_variable_cname(n,v,o,r) \
          define_variable_in_set((n),(sizeof (n) - 1),(v),(o),(r),\
                                 current_variable_set_list->set,NILF)

/* Define a variable with a location in the current variable set.  */

#define define_variable_loc(n,l,v,o,r,f) \
          define_variable_in_set((n),(l),(v),(o),(r),\
                                 current_variable_set_list->set,(f))

/* Define a variable with a location in the global variable set.  */

#define define_variable_global(n,l,v,o,r,f) \
          define_variable_in_set((n),(l),(v),(o),(r),NULL,(f))

/* Define a variable in FILE's variable set.  */

#define define_variable_for_file(n,l,v,o,r,f) \
          define_variable_in_set((n),(l),(v),(o),(r),(f)->variables->set,NILF)

void undefine_variable_in_set (const char *name, size_t length,
                               enum variable_origin origin,
                               struct variable_set *set);

/* Remove variable from the current variable set. */

#define undefine_variable_global(n,l,o) \
          undefine_variable_in_set((n),(l),(o),NULL)

/* Warn that NAME is an undefined variable.  */

#define warn_undefined(n,l) do{\
                              if (warn_undefined_variables_flag)        \
                                error (reading_file, (l),               \
                                       _("warning: undefined variable '%.*s'"), \
                                       (int)(l), (n));                  \
                              }while(0)

char **target_environment (struct file *file);

struct pattern_var *create_pattern_var (const char *target,
                                        const char *suffix);

extern int export_all_variables;

struct variable *
define_makeflags (int all, int makefile);

#define MAKELEVEL_NAME "MAKELEVEL"
#define MAKELEVEL_LENGTH (CSTRLEN (MAKELEVEL_NAME))

#define MAKEPARENT_PID_NAME "MAKEPARENT_PID"
#define MAKEPARENT_PID_LENGTH (CSTRLEN (MAKEPARENT_PID_NAME))

#define MAKEPARENT_TARGET_NAME "MAKEPARENT_TARGET"
#define MAKEPARENT_TARGET_LENGTH (CSTRLEN (MAKEPARENT_TARGET_NAME))
#endif /*VARIABLE_H*/
