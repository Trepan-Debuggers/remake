/* Copyright (C) 1988, 1989, 1990, 1991 Free Software Foundation, Inc.
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
    struct commands *cmds;	/* Commands to execute for this target */
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

    short int update_status;	/* Status of the last attempt to update,
				   or -1 if none has been made.  */

    enum			/* State of the commands.  */
      {		/* Note: It is important that cs_not_started be zero.  */
	cs_not_started,		/* Not yet started.  */
	cs_deps_running,	/* Dep commands running.  */
	cs_running,		/* Commands running.  */
	cs_finished		/* Commands finished.  */
      } command_state ENUM_BITFIELD (2);

    unsigned int double_colon:1;/* Nonzero for double-colon entry */
    unsigned int precious:1;	/* Non-0 means don't delete file on quit */
    unsigned int tried_implicit:1;/* Nonzero if have searched
				     for implicit rule for making
				     this file; don't search again.  */
    unsigned int updating:1;	/* Nonzero while updating deps of this file */
    unsigned int updated:1;	/* Nonzero if this file has been remade.  */
    unsigned int is_target:1;	/* Nonzero if file is described as target.  */
    unsigned int cmd_target:1;	/* Nonzero if file was given on cmd line.  */
    unsigned int phony:1;	/* Nonzero if this is a phony file
				   ie, a dependent of .PHONY.  */
    unsigned int intermediate:1;/* Nonzero if this is an intermediate file.  */
    unsigned int dontcare:1;	/* Nonzero if no complaint is to be made if
				   this target cannot be remade.  */
  };

/* Number of intermediate files entered.  */

extern unsigned int num_intermediates;

extern struct file *default_goal_file, *suffix_file, *default_file;


extern struct file *lookup_file (), *enter_file ();
extern void remove_intermediates (), snap_deps ();
extern void rename_file ();


extern time_t f_mtime ();
#define file_mtime_1(f, v) \
  ((f)->last_mtime != (time_t) 0 ? (f)->last_mtime : f_mtime ((f), v))
#define file_mtime(f) file_mtime_1 ((f), 1)
#define file_mtime_no_search(f) file_mtime_1 ((f), 0)


#define check_renamed(file) \
  while ((file)->renamed != 0) (file) = (file)->renamed /* No ; here.  */
