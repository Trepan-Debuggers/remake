/* Structure describing a running or dead child process.  */

struct child
  {
    struct child *next;		/* Link in the chain.  */

    struct file *file;		/* File being remade.  */

    char **environment;		/* Environment for commands.  */

    char **command_lines;	/* Array of variable-expanded cmd lines.  */
    unsigned int command_line;	/* Index into above.  */
    char *command_ptr;		/* Ptr into command_lines[command_line].  */

    int pid;			/* Child process's ID number.  */
    unsigned int remote:1;	/* Nonzero if executing remotely.  */

    unsigned int noerror:1;	/* Nonzero if commands contained a `-'.  */

    unsigned int good_stdin:1;	/* Nonzero if this child has a good stdin.  */
    unsigned int deleted:1;	/* Nonzero if targets have been deleted.  */
  };

extern struct child *children;

extern void new_job ();
extern void wait_for_children ();
extern void push_signals_blocked_p (), pop_signals_blocked_p ();

extern char **construct_command_argv ();
extern void child_execute_job ();
extern void exec_command ();

extern unsigned int job_slots_used;
