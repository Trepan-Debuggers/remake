/* Definition of target file data structures for GNU Make.
Copyright (C) 1988, 89, 90, 91, 92, 93, 94 Free Software Foundation, Inc.
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
the Free Software Foundation, 675 Mass Ave, Cambridge, MA 02139, USA.  */

/* Structure that represents the info on one file
   that the makefile says how to make.
   All of these are chained together through `next'.  */

struct file
  {
    struct file *next;
    char *name;
    struct dep *deps;
    struct commands *cmds;	/* Commands to execute for this target.  */
    int command_flags;		/* Flags OR'd in for cmds; see commands.h.  */
    char *stem;			/* Implicit stem, if an implicit
    				   rule has been used */
    struct dep *also_make;	/* Targets that are made by making this.  */
    time_t last_mtime;		/* File's modtime, if already known.  */
    struct file *prev;		/* Previous entry for same file name;
				   used when there are multiple double-colon
				   entries for the same file.  */

    /* File that this file was renamed to.  After any time that a
       file could be renamed, call `check_renamed' (below).  */
    struct file *renamed;

    /* List of variable sets used for this file.  */
    struct variable_set_list *variables;

    /* Immediate dependent that caused this target to be remade,
       or nil if there isn't one.  */
    struct file *parent;

    /* For a double-colon entry, this is the first double-colon entry for
       the same file.  Otherwise this is null.  */
    struct file *double_colon;

    short int update_status;	/* Status of the last attempt to update,
				   or -1 if none has been made.  */

    enum			/* State of the commands.  */
      {		/* Note: It is important that cs_not_started be zero.  */
	cs_not_started,		/* Not yet started.  */
	cs_deps_running,	/* Dep commands running.  */
	cs_running,		/* Commands running.  */
	cs_finished		/* Commands finished.  */
      } command_state ENUM_BITFIELD (2);

    unsigned int precious:1;	/* Non-0 means don't delete file on quit */
    unsigned int tried_implicit:1; /* Nonzero if have searched
				      for implicit rule for making
				      this file; don't search again.  */
    unsigned int updating:1;	/* Nonzero while updating deps of this file */
    unsigned int updated:1;	/* Nonzero if this file has been remade.  */
    unsigned int is_target:1;	/* Nonzero if file is described as target.  */
    unsigned int cmd_target:1;	/* Nonzero if file was given on cmd line.  */
    unsigned int phony:1;	/* Nonzero if this is a phony file
				   i.e., a dependency of .PHONY.  */
    unsigned int intermediate:1;/* Nonzero if this is an intermediate file.  */
    /* Nonzero, for an intermediate file,
       means remove_intermediates should not delete it.  */
    unsigned int secondary:1;
    unsigned int dontcare:1;	/* Nonzero if no complaint is to be made if
				   this target cannot be remade.  */
  };

/* Number of intermediate files entered.  */

extern unsigned int num_intermediates;

extern struct file *default_goal_file, *suffix_file, *default_file;


extern struct file *lookup_file (), *enter_file ();
extern void remove_intermediates (), snap_deps ();
extern void rename_file (), file_hash_enter ();
extern void set_command_state ();


/* Return the mtime of file F (a struct file *), caching it.
   The value is -1 if the file does not exist.  */
#define file_mtime(f) file_mtime_1 ((f), 1)
/* Return the mtime of file F (a struct file *), caching it.
   Don't search using vpath for the file--if it doesn't actually exist,
   we don't find it.
   The value is -1 if the file does not exist.  */
#define file_mtime_no_search(f) file_mtime_1 ((f), 0)
extern time_t f_mtime ();
#define file_mtime_1(f, v) \
  ((f)->last_mtime != (time_t) 0 ? (f)->last_mtime : f_mtime ((f), v))

/* Modtime value to use for `infinitely new'.  We used to get the current time
   from the system and use that whenever we wanted `new'.  But that causes
   trouble when the machine running make and the machine holding a file have
   different ideas about what time it is; and can also lose for `force'
   targets, which need to be considered newer than anything that depends on
   them, even if said dependents' modtimes are in the future.

   NOTE: This assumes 32-bit `time_t's, but I cannot think of a portable way
   to produce the largest representable integer of a given signed type.  */
#define NEW_MTIME	((time_t) 0x7fffffff)


#define check_renamed(file) \
  while ((file)->renamed != 0) (file) = (file)->renamed /* No ; here.  */
