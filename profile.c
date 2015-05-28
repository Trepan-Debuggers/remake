/* Compile with:
   gcc -g3 -I . -c -DSTANDALONE profile.c -o test-profile.o
   gcc mock.o version.o alloc.o flags.o enter_file.o hash.o strcache.o test-profile.o -o test-profile
*/
#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <errno.h>

#include "filedef.h"

#define CALLGRIND_FILE_PREFIX "callgrind.out."
#define CALLGRIND_FILE_TEMPLATE CALLGRIND_FILE_PREFIX "%d"
#define CALLGRIND_FILENAME_LEN sizeof(CALLGRIND_FILE_PREFIX) + 10
extern char callgrind_fname[sizeof(CALLGRIND_FILENAME_LEN)];
char callgrind_fname[sizeof(CALLGRIND_FILENAME_LEN)];
static FILE *callgrind_fd;

#define CALLGRIND_PREAMBLE_TEMPLATE "version: 1\n\
creator: %s\n\
cmd:  %s\n\
pid: %u\n\
\n\
desc: Trigger: %s\n\
\n\
positions: line\n\
events: Target\n\
\n"

extern bool
init_callgrind(const char *creator, const char *invocation,
	       const char *program_status) {
  const pid_t callgrind_pid = getpid();
  size_t len = snprintf(callgrind_fname, CALLGRIND_FILENAME_LEN,
			CALLGRIND_FILE_TEMPLATE, callgrind_pid);
  if (len < 0 || len >= CALLGRIND_FILENAME_LEN) {
    printf("Error in generating name\n");
    return false;
  }
  callgrind_fd = fopen(callgrind_fname, "w");
  if (NULL == callgrind_fd) {
    printf("Error in opening callgrind file %s\n", callgrind_fname);
    return false;
  }
  fprintf(callgrind_fd, CALLGRIND_PREAMBLE_TEMPLATE,
	  creator, invocation, callgrind_pid,
	  program_status);
  return true;
}

static unsigned int next_file_num = 0;
static unsigned int next_fn_num   = 0;
extern void
add_target(file_t *target) {
  fprintf(callgrind_fd,
	  "fl=(%d) %s\nfn(%d) %s\n%lu %lu\n\n",
	  next_file_num++,
	  target->floc.filenm,
	  next_fn_num++,
	  target->name,
	  target->floc.lineno,
	  (unsigned long) 500);
}

extern void
close_callgrind(void) {
  fclose(callgrind_fd);
}

#ifdef STANDALONE
#include "filedef.h"
#include "types.h"
int main(int argc, const char **argv) {
  bool rc = init_callgrind("remake 4.1+dbg1.0", "remake foo",
			   "Program termination");
  if (rc) {
    init_hash_files();
    file_t *target = enter_file("Makefile");
    printf("%s\n", callgrind_fname);

    close_callgrind();
  }
  return rc;
}
#endif
