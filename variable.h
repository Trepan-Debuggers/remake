/* Definitions for using variables in GNU Make.
Copyright (C) 1988, 1989, 1990, 1991, 1992, 1993, 1994, 1995, 1996, 1997,
1998, 1999, 2000, 2001, 2002, 2003, 2004, 2005, 2006 Free Software
Foundation, Inc.
Copyright (c) 2008  Rocky Bernstein <rocky@gnu.org>

This file is part of GNU Make.

GNU Make is free software; you can redistribute it and/or modify it under the
terms of the GNU General Public License as published by the Free Software
Foundation; either version 2, or (at your option) any later version.

GNU Make is distributed in the hope that it will be useful, but WITHOUT ANY
WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR
A PARTICULAR PURPOSE.  See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License along with
GNU Make; see the file COPYING.  If not, write to the Free Software
Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301 USA.  */


#ifndef VARIABLE_H
#define VARIABLE_H

#include "hash.h"
#include "filedef.h"

/* Codes in a variable definition saying where the definition came from.
   Increasing numeric values signify less-overridable definitions.  */
typedef enum variable_origin {
    o_default,		/* Variable from the default set.  */
    o_env,		/* Variable from environment.  */
    o_file,		/* Variable given in a makefile.  */
    o_env_override,	/* Variable from environment, if -e.  */
    o_command,		/* Variable given by user.  */
    o_override, 	/* Variable from an `override' directive.  */
    o_automatic,	/* Automatic variable -- cannot be set.  */
    o_debugger,  	/** set inside debugger.  */
    o_invalid		/* Core dump time.  */
  } variable_origin_t;

typedef enum variable_flavor {
    f_bogus,            /* Bogus (error) */
    f_simple,           /* Simple definition (:=) */
    f_recursive,        /* Recursive definition (=) */
    f_append,           /* Appending definition (+=) */
    f_conditional       /* Conditional definition (?=) */
} variable_flavor_t;


/* Structure that represents one variable definition.
   Each bucket of the hash table is a chain of these,
   chained through `next'.  */

#define EXP_COUNT_BITS  15      /* This gets all the bitfields into 32 bits */
#define EXP_COUNT_MAX   ((1<<EXP_COUNT_BITS)-1)

typedef struct variable
  {
    char *name;			/* Variable name.  */
    int length;			/* strlen (name) */
    char *value;		/* Variable value.  */
    struct floc fileinfo;       /* Where the variable was defined.  */
    unsigned int recursive:1;	/* Gets recursively re-evaluated.  */
    unsigned int append:1;	/* Nonzero if an appending target-specific
                                   variable.  */
    unsigned int conditional:1; /* Nonzero if set with a ?=. */
    unsigned int per_target:1;	/* Nonzero if a target-specific variable.  */
    unsigned int special:1;     /* Nonzero if this is a special variable. */
    unsigned int exportable:1;  /* Nonzero if the variable _could_ be
                                   exported.  */
    unsigned int expanding:1;	/* Nonzero if currently being expanded.  */
    unsigned int exp_count:EXP_COUNT_BITS;
                                /* If >1, allow this many self-referential
                                   expansions.  */
    enum variable_flavor
      flavor ENUM_BITFIELD (3);	/* Variable flavor.  */
    enum variable_origin
      origin ENUM_BITFIELD (4);	/* Variable origin.  */
    enum variable_export
      {
	v_export,		/* Export this variable.  */
	v_noexport,		/* Don't export this variable.  */
	v_ifset,		/* Export it if it has a non-default value.  */
	v_default		/* Decide in target_environment.  */
      } export ENUM_BITFIELD (2);
  } variable_t;

/* Structure that represents a variable set.  */

struct variable_set
  {
    struct hash_table table;	/* Hash table of variables.  */
  };

/* Structure that represents a list of variable sets.  */

struct variable_set_list
  {
    struct variable_set_list *next;	/* Link in the chain.  */
    struct variable_set *set;		/* Variable set.  */
  };

/* Structure used for pattern-specific variables.  */

struct pattern_var
  {
    struct pattern_var *next;
    char *target;
    unsigned int len;
    char *suffix;
    struct variable variable;
  };

extern char *variable_buffer;
extern struct variable_set_list *current_variable_set_list;

/* expand.c */
extern char *allocated_variable_expand_for_file PARAMS ((char *line, struct file *file));
extern char *variable_expand PARAMS ((char *line));
extern char *variable_expand_for_file PARAMS ((char *line, struct file *file));
extern char *allocated_variable_expand_for_file PARAMS ((char *line, struct file *file));
#define	allocated_variable_expand(line) \
  allocated_variable_expand_for_file (line, (struct file *) 0)
extern char *expand_argument PARAMS ((const char *str, const char *end));
extern void install_variable_buffer PARAMS ((char **bufp, unsigned int *lenp));
extern void restore_variable_buffer PARAMS ((char *buf, unsigned int len));

/* function.c */
extern int handle_function PARAMS ((char **op, char **stringp));
extern int pattern_matches PARAMS ((char *pattern, char *percent, char *str));
extern char *subst_expand PARAMS ((char *o, char *text, char *subst, char *replace,
		unsigned int slen, unsigned int rlen, int by_word));
extern char *patsubst_expand PARAMS ((char *o, char *text, char *pattern, char *replace,
		char *pattern_percent, char *replace_percent));

/* expand.c */
extern char *recursively_expand_for_file PARAMS ((struct variable *v,
                                                  struct file *file));
#define recursively_expand(v)   recursively_expand_for_file (v, NULL)

/* variable.c */
extern struct variable_set_list *create_new_variable_set PARAMS ((void));

/*!
  Return a string describing origin.
 */
const char *origin2str(variable_origin_t origin);

extern void free_variable_set PARAMS ((struct variable_set_list *));

/*! Create a new variable set, push it on the current setlist,
  and assign current_variable_set_list to it. 
 */
extern struct variable_set_list *push_new_variable_scope PARAMS ((void));

/*! Pop the top set off the current_variable_set_list, and free all
   its storage.  If b_toplevel set we have the top-most global scope
   and some things don't get freed because they weren't malloc'd.
*/
extern void pop_variable_scope PARAMS ((void));

/*! Define the automatic variables, and record the addresses of their
  structures so we can change their values quickly.  */
extern void define_automatic_variables PARAMS ((void));

/*! Initialize FILE's variable set list.  If FILE already has a
   variable set list, the topmost variable set is left intact, but the
   the rest of the chain is replaced with FILE->parent's setlist.  If
   FILE is a double-colon rule, then we will use the "root"
   double-colon target's variable set as the parent of FILE's variable
   set.

   If we're READing a makefile, don't do the pattern variable search now,
   since the pattern variable might not have been defined yet.  */
extern void initialize_file_variables PARAMS ((struct file *file, int read));

/*! Print all the local variables of P_TARGET.  Lines output have "# "
    prepended. If you want hash table statistics too, set b_hash_stats
    true.
*/
extern void print_file_variables PARAMS ((struct file *file, 
					  bool b_hash_stats));

/*! Print all the variables in SET.  PREFIX is printed before the
   actual variable definitions (everything else is comments).  If you
   want hash table statistics too, set b_hash_stats true.
*/

/*! Print the data base of variables.  */

extern void print_variable_data_base (void);

/** Print information for variable V, prefixing it with PREFIX.  */
extern void print_variable_info (const void *item, void *arg);

/*! Print all the variables in SET.  PREFIX is printed before the
   actual variable definitions (everything else is comments).  If you
   want hash table statistics too, set b_hash_stats true.
*/
extern void print_variable_set PARAMS ((struct variable_set *set, 
					char *prefix, bool b_hash_stats));

/*! Merge FROM_SET into TO_SET, freeing unused storage in
    FROM_SET.  */
extern void merge_variable_set_lists PARAMS ((struct variable_set_list **to_list, struct variable_set_list *from_list));

/*! Given a variable, a value, and a flavor, define the variable.  See
   the try_variable_definition() function for details on the
   parameters. */
extern struct variable *do_variable_definition PARAMS ((const struct floc *flocp, const char *name, char *value, enum variable_origin origin, enum variable_flavor flavor, int target_var));

/*! Try to interpret LINE (a null-terminated string) as a variable
    definition.

   If LINE was recognized as a variable definition, a pointer to its `struct
   variable' is returned.  If LINE is not a variable definition, NULL is
   returned.  */
extern variable_t *parse_variable_definition PARAMS ((struct variable *v, char *line));

variable_t *try_variable_definition PARAMS ((const struct floc *flocp, char *line, enum variable_origin origin, int target_var));

void init_hash_global_variable_set PARAMS ((void));

void hash_init_function_table PARAMS ((void));
extern struct variable *lookup_variable PARAMS ((const char *name, unsigned int length));

struct variable *lookup_variable_in_set PARAMS ((const char *name,
						 unsigned int length,
						 const struct variable_set *set));

extern struct variable *define_variable_in_set
    PARAMS ((const char *name, unsigned int length, char *value,
             enum variable_origin origin, int recursive,
             struct variable_set *set, const struct floc *flocp));

/* Define a variable in the current variable set.  */

#define define_variable(n,l,v,o,r) \
          define_variable_in_set((n),(l),(v),(o),(r),\
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

/* Warn that NAME is an undefined variable.  */

#define warn_undefined(n,l) do{\
                              if (warn_undefined_variables_flag) \
                                error (reading_file, \
                                       _("warning: undefined variable `%.*s'"), \
                                (int)(l), (n)); \
                              }while(0)

extern char **target_environment PARAMS ((struct file *file));

extern struct pattern_var *create_pattern_var PARAMS ((char *target, char *suffix));

extern int export_all_variables;

#define MAKELEVEL_NAME "MAKELEVEL"
#define MAKELEVEL_LENGTH (sizeof (MAKELEVEL_NAME) - 1)

#endif /*VARIABLE_H*/
