/* Definitions for using variables in GNU Make.
Copyright (C) 1988, 1989, 1990, 1991, 1992 Free Software Foundation, Inc.
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

/* Codes in a variable definition saying where the definition came from.
   Increasing numeric values signify less-overridable definitions.  */
enum variable_origin
  {
    o_default,		/* Variable from the default set.  */
    o_env,		/* Variable from environment.  */
    o_file,		/* Variable given in a makefile.  */
    o_env_override,	/* Variable from environment, if -e.  */
    o_command,		/* Variable given by user.  */
    o_override, 	/* Variable from an `override' directive.  */
    o_automatic,	/* Automatic variable -- cannot be set.  */
    o_invalid		/* Core dump time.  */
  };

/* Structure that represents one variable definition.
   Each bucket of the hash table is a chain of these,
   chained through `next'.  */

struct variable
  {
    struct variable *next;	/* Link in the chain.  */
    char *name;			/* Variable name.  */
    char *value;		/* Variable value.  */
    enum variable_origin
      origin ENUM_BITFIELD (3);	/* Variable origin.  */
    unsigned int recursive:1;	/* Gets recursively re-evaluated.  */
    unsigned int expanding:1;	/* Nonzero if currently being expanded.  */
    unsigned int per_target:1;	/* Nonzero if a target-specific variable.  */
    enum variable_export
      {
	v_export,		/* Export this variable.  */
	v_noexport,		/* Don't export this variable.  */
	v_ifset,		/* Export it if it has a non-default value.  */
	v_default		/* Decide in target_environment.  */
      } export ENUM_BITFIELD (2);
  };

/* Structure that represents a variable set.  */

struct variable_set
  {
    struct variable **table;	/* Hash table of variables.  */
    unsigned int buckets;	/* Number of hash buckets in `table'.  */
  };

/* Structure that represents a list of variable sets.  */

struct variable_set_list
  {
    struct variable_set_list *next;	/* Link in the chain.  */
    struct variable_set *set;		/* Variable set.  */
  };

extern char *variable_buffer;
extern struct variable_set_list *current_variable_set_list;

/* expand.c */
extern char *variable_buffer_output PARAMS ((char *ptr, char *string, unsigned int length));
extern char *variable_expand PARAMS ((char *line));
extern char *allocated_variable_expand_for_file PARAMS ((char *line, struct file *file));
#define	allocated_variable_expand(line) \
  allocated_variable_expand_for_file (line, (struct file *) 0)
extern char *expand_argument PARAMS ((char *str, char *end));
extern char *variable_expand_string PARAMS ((char *line, char *string,
                                             long length));

/* function.c */
extern int handle_function PARAMS ((char **op, char **stringp));
extern int pattern_matches PARAMS ((char *pattern, char *percent, char *str));
extern char *subst_expand PARAMS ((char *o, char *text, char *subst, char *replace,
		unsigned int slen, unsigned int rlen, int by_word, int suffix_only));
extern char *patsubst_expand PARAMS ((char *o, char *text, char *pattern, char *replace,
		char *pattern_percent, char *replace_percent));

/* expand.c */
extern char *recursively_expand PARAMS ((struct variable *v));

/* variable.c */
extern struct variable_set_list *create_new_variable_set PARAMS ((void));
extern struct variable_set_list *push_new_variable_scope PARAMS ((void));
extern void pop_variable_scope PARAMS ((void));
extern void define_automatic_variables PARAMS ((void));
extern void initialize_file_variables PARAMS ((struct file *file));
extern void print_file_variables PARAMS ((struct file *file));
extern void print_variable_set PARAMS ((struct variable_set *set, char *prefix));
extern void merge_variable_set_lists PARAMS ((struct variable_set_list **setlist0, struct variable_set_list *setlist1));
extern struct variable *try_variable_definition PARAMS ((const struct floc *flocp, char *line, enum variable_origin origin));

extern struct variable *lookup_variable PARAMS ((char *name, unsigned int length));
extern struct variable *define_variable PARAMS ((char *name, unsigned int length, char *value,
		enum variable_origin origin, int recursive));
extern struct variable *define_variable_in_set PARAMS ((char *name, unsigned int length,
		char *value, enum variable_origin origin, int recursive, struct variable_set *set));
extern struct variable *define_variable_for_file PARAMS ((char *name, unsigned int length,
		char *value, enum variable_origin origin, int recursive, struct file *file));
extern char **target_environment PARAMS ((struct file *file));

extern int export_all_variables;
