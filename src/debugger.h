#include "debug.h"
#include "trace.h"

/* Common debugger command function prototype */
typedef debug_return_t (*dbg_cmd_t) (char *psz_args);


typedef struct {
  const char *long_name;	/* long name of command. */
  const char short_name;	/* Index into short_cmd array. */
} long_cmd_t;

/* A structure which contains information on the commands this program
   can understand. */

typedef struct {
  dbg_cmd_t func;       /* Function to call to do the job. */
  const char *doc;	/* Documentation for this function.  */
  const char *use;	/* short command usage.  */
  uint8_t id;	        /* index into global commands, and short_command arrays.
                           255 is uninitialized.
                        */
} short_cmd_t;
