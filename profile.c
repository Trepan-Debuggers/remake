/* Compile with:
   gcc -g3 -I . -c -DSTANDALONE profile.c -o test-profile.o
   gcc mock.o version.o alloc.o globals.o misc.o output.o enter_file.o hash.o strcache.o test-profile.o -o test-profile
*/
#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <errno.h>

#include "profile.h"

#define CALLGRIND_FILE_PREFIX "callgrind.out."
#define CALLGRIND_FILE_TEMPLATE CALLGRIND_FILE_PREFIX "%d"

/* + 10 is more than enough since 2**64 ~= 10**9 */
#define CALLGRIND_FILENAME_LEN sizeof(CALLGRIND_FILE_PREFIX) + 20

char callgrind_fname[CALLGRIND_FILENAME_LEN];
static FILE *callgrind_fd;

#define CALLGRIND_PREAMBLE_TEMPLATE1 "version: 1\n\
creator: %s\n"

#define CALLGRIND_PREAMBLE_TEMPLATE2 "pid: %u\n\
desc: Trigger: %s\n\
\n\
positions: line\n\
events: Target\n\
\n"

extern bool
init_callgrind(const char *creator, const char *const *argv,
	       const char *program_status) {
  const pid_t callgrind_pid = getpid();
  size_t len = sprintf(callgrind_fname, CALLGRIND_FILE_TEMPLATE, callgrind_pid);
  unsigned int i;
  if (len >= CALLGRIND_FILENAME_LEN) {
    printf("Error in generating name\n");
    return false;
  }
  callgrind_fd = fopen(callgrind_fname, "w");
  if (NULL == callgrind_fd) {
    printf("Error in opening callgrind file %s\n", callgrind_fname);
    return false;
  }
  fprintf(callgrind_fd, CALLGRIND_PREAMBLE_TEMPLATE1,
	  creator);
  fprintf(callgrind_fd, "cmd:");
  for (i = 0; argv[i]; i++) {
      fprintf(callgrind_fd, " %s", argv[i]);
    }
  fprintf(callgrind_fd, "\n");
  fprintf(callgrind_fd, CALLGRIND_PREAMBLE_TEMPLATE2, callgrind_pid,
	  program_status);
  return true;
}

static unsigned int next_file_num = 0;
static unsigned int next_fn_num   = 1;

extern void
add_file(file_t *target) {
  fprintf(callgrind_fd,
	  "fl=(%d) %s\n",
	  next_file_num++,
	  target->floc.filenm);
  target->file_profiled = 1;
}


extern void
add_target(file_t *target) {
  file_t *target_prev = target->prev;
  file_t *target_file = lookup_file(target->floc.filenm);
  if (target_prev) {
    file_t *prev_filename = lookup_file(target_prev->floc.filenm);
    if (prev_filename && !prev_filename->file_profiled) {
      add_file(prev_filename);
    }
  }
  if (target_file && !target_file->file_profiled) {
      add_file(target);
  }

  fprintf(callgrind_fd,
	  "fn=(%d) %s\n%lu %lu\n",
	  next_fn_num,
	  target->name,
	  target->floc.lineno,
	  target->elapsed_time);
  if (target_prev && target_prev->profiled_fn) {
    fprintf(callgrind_fd,
	    "cfn=(%lu)\n\n",
	    target_prev->profiled_fn);
  }
  target->profiled_fn = next_fn_num;
  target->file_profiled = 1;
  next_fn_num++;
}

extern void
close_callgrind(void) {
  printf("Created callgrind profiling data file: %s\n", callgrind_fname);
  fclose(callgrind_fd);
}

#ifdef STANDALONE
#include "config.h"
#include "types.h"
int main(int argc, const char * const* argv) {
  bool rc = init_callgrind(PACKAGE_TARNAME " " PACKAGE_VERSION, argv,
			   "Program termination");
  init_hash_files();
  if (rc) {
    file_t *target = enter_file("Makefile");
    file_t *target2, *target3;
    target->floc.filenm = "Makefile";

    target2 = enter_file("all");
    target2->floc.filenm = "Makefile";
    target2->floc.lineno = 5;
    target2->elapsed_time = 500;
    target2->prev = target;
    add_target(target2);

    target3 = enter_file("all-recursive");
    target3->floc.filenm = "Makefile";
    target3->floc.lineno = 5;
    target3->elapsed_time = 500;
    target3->prev = target2;
    add_target(target3);

    close_callgrind();
  }
  return rc;
}
#endif
