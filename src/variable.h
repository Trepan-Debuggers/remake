/* $id: variable.h,v 1.9 2005/12/02 12:12:09 rockyb Exp $
Definitions for using variables in GNU Make.
Copyright (C) 1988, 1989, 1990, 1991, 1992, 2002, 2004, 2005
Free Software Foundation, Inc.
This file is part of GNU Make.

GNU Make is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2, or (at your option)
any later version.

GNU Make is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with GNU Make; see the file COPYING.  If not, write to
the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
Boston, MA 02111-1307, USA.  */

#ifndef VARIABLE_H
#define VARIABLE_H

#include "make.h"
#include "hash.h"
#include "filedef.h"

/** Codes in a variable definition saying where the definition came from.
   Increasing numeric values signify less-overridable definitions.  */
typedef enum {
  o_default,		/**< Variable from the default set.  */
  o_env,		/**< Variable from environment.  */
  o_file,		/**< Variable given in a makefile.  */
  o_env_override,	/**< Variable from environment, if -e.  */
  o_command,		/**< Variable given by user.  */
  o_override, 	        /**< Variable from an `override' directive.  */
  o_automatic,	        /**< Automatic variable -- cannot be set.  */
  o_debugger,  	        /**< set inside debugger.  */
  o_invalid		/**< Core dump time.  */
} variable_origin_t;

typedef enum {
  f_bogus,            /* Bogus (error) */
  f_simple,           /* Simple definition (:=) */
  f_recursive,        /* Recursive definition (=) */
  f_append,           /* Appending definition (+=) */
  f_conditional       /* Conditional definition (?=) */
} variable_flavor_t;

/** Structure that represents one variable definition.
   Each bucket of the hash table is a chain of these,
   chained through `next'.  */

#define EXP_COUNT_BITS  15      /* This gets all the bitfields into 32 bits */
#define EXP_COUNT_MAX   ((1<<EXP_COUNT_BITS)-1)

typedef struct variable
  {
    char *name;			/**< Variable name.  */
    int length;			/**< strlen (name) */
    char *value;		/**< Variable value.  */
    floc_t fileinfo;            /**< Where the variable was defined.  */
    unsigned int recursive:1;	/**< Gets recursively re-evaluated.  */
    unsigned int append:1;	/**< Nonzero if an appending target-specific
                                     variable.  */
    unsigned int conditional:1; /**< Nonzero if set with a ?=. */
    unsigned int per_target:1;	/**< Nonzero if a target-specific variable.  */
    unsigned int special:1;     /**< Nonzero if this is a special variable. */
    unsigned int exportable:1;  /**< Nonzero if the variable _could_ be
                                     exported.  */
    unsigned int expanding:1;	/**< Nonzero if currently being expanded.  */
    unsigned int exp_count:EXP_COUNT_BITS;
                                /**< If >1, allow this many self-referential
                                   expansions.  */
    variable_flavor_t
      flavor ENUM_BITFIELD (3);	/**< Variable flavor.  */
    variable_origin_t
      origin ENUM_BITFIELD (4);	/**< Variable origin.  */
    enum variable_export
      {
	v_export,	/**< Export this variable.  */
	v_noexport,	/**< Don't export this variable.  */
	v_ifset,	/**< Export it if it has a non-default value.  */
	v_default	/**< Decide in target_environment.  */
      } export ENUM_BITFIELD (2);
  } variable_t;


/** Structure that represents a variable set.  */

typedef struct variable_set
  {
    hash_table_t table;	/* Hash table of variables.  */
  } variable_set_t;

/** Structure that represents a list of variable sets.  */

typedef struct variable_set_list
  {
    struct variable_set_list *next;	/* Link in the chain.  */
    variable_set_t *set;		/* Variable set.  */
  } variable_set_list_t;

/** Structure used for pattern-specific variables.  */

struct pattern_var
  {
    struct pattern_var *next;
    char *target;
    unsigned int len;
    char *suffix;
    struct variable variable;
  };

extern char *variable_buffer;
extern variable_set_list_t *current_variable_set_list;

/* variable.c */

/*!
  Return a string describing origin.
 */
const char *origin2str(variable_origin_t origin);

/*! Create a new variable set and push it on the current setlist.  */
extern variable_set_list_t *create_new_variable_set (void);

/*! Create a new variable set, push it on the current setlist,
  and assign current_variable_set_list to it. 
 */
variable_set_list_t *push_new_variable_scope (void);

/*! Pop the top set off the current_variable_set_list, and free all
   its storage.  If b_toplevel set we have the top-most global scope
   and some things don't get freed because they weren't malloc'd.
*/
void pop_variable_scope (bool b_toplevel);

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
extern void initialize_file_variables (struct file *p_target, int read);

/*! Print all the local variables of P_TARGET.  Lines output have "# "
    prepended. If you want hash table statistics too, set b_hash_stats
    true.
*/
extern void print_file_variables (file_t *p_target, bool b_hash_stats);

/*! Print the data base of variables.  */

extern void print_variable_data_base (void);

/** Print information for variable V, prefixing it with PREFIX.  */
extern void print_variable_info (const void *item, void *arg);

/*! Print all the variables in SET.  PREFIX is printed before the
   actual variable definitions (everything else is comments).  If you
   want hash table statistics too, set b_hash_stats true.
*/
extern void print_variable_set (variable_set_t *p_set, char *psz_prefix,
				bool b_hash_stats);

/*! Merge FROM_SET into TO_SET, freeing unused storage in
    FROM_SET.  */
extern void merge_variable_set_lists (variable_set_list_t **to_list, variable_set_list_t *from_list);

/*! Given a variable, a value, and a flavor, define the variable.  See
   the try_variable_definition() function for details on the
   parameters. */
extern variable_t *do_variable_definition (const floc_t *p_floc, 
					   const char *name,  char *value, 
					   variable_origin_t origin,
					   variable_flavor_t flavor,
					   int target_var);


/*! Try to interpret LINE (a null-terminated string) as a variable
    definition.

   If LINE was recognized as a variable definition, a pointer to its `struct
   variable' is returned.  If LINE is not a variable definition, NULL is
   returned.  */
extern variable_t *parse_variable_definition (variable_t *v, char *line);

variable_t *try_variable_definition (const floc_t *p_floc, 
				     char *line,  variable_origin_t origin, 
				     int target_var);

void init_hash_global_variable_set (void);

void free_hash_global_variable_set (void);

void hash_init_function_table (void);

/*! Lookup a variable whose name is a string starting at NAME and with
   LENGTH chars.  NAME need not be null-terminated.  Returns address
   of the `struct variable' containing all info on the variable, or
   nil if no such variable is defined.  */
variable_t *lookup_variable (const char *name, unsigned int length);

variable_t *lookup_variable_in_set (const char *name, 
					   unsigned int length,
					   const variable_set_t *set);

/*! Define variable named NAME with value VALUE in SET.  VALUE is copied.
  LENGTH is the length of NAME, which does not need to be null-terminated.
  ORIGIN specifies the origin of the variable (makefile, command line
  or environment).
  If RECURSIVE is nonzero a flag is set in the variable saying
  that it should be recursively re-expanded.  */

variable_t *define_variable_in_set (const char *name, 
				    unsigned int length, char *value,
				    variable_origin_t origin, 
				    int recursive,
				    variable_set_t *set, 
				    const floc_t *p_floc);

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

extern char **target_environment (file_t *p_target);

extern struct pattern_var *create_pattern_var (char *psz_target, 
					       char *psz_suffix);

extern int export_all_variables;

#define MAKELEVEL_NAME "MAKELEVEL"
#define MAKELEVEL_LENGTH (sizeof (MAKELEVEL_NAME) - 1)

#endif /*VARIABLE_H*/
