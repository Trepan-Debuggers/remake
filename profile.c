/* Compile with:
   gcc -g3 -I . -c -DSTANDALONE profile.c -o test-profile.o
   make mock.o
   gcc mock.o version.o alloc.o globals.o misc.o output.o enter_file.o hash.o strcache.o test-profile.o -o test-profile
*/
#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <errno.h>

#include "profile.h"
#include "hash.h"

/*! \brief Node for an item in the target call stack */
typedef struct profile_call profile_call_t;
typedef struct profile_call   {
  file_t         *p_target;
  profile_call_t *p_next;
} profile_call_t;

struct profile_entry   {
  uint64_t elapsed_time;   /* rutime in milliseconds */
  const char *name;
  gmk_floc floc;           /* location in Makefile - for tracing */
  profile_call_t *calls;   /* List of targets this target calls.  */
};
typedef struct profile_entry profile_entry_t;

static struct hash_table profile_table;

static unsigned long
profile_table_entry_hash_1 (const void *key)
{
  return_ISTRING_HASH_1 (((profile_entry_t *) key)->name);
}

static unsigned long
profile_table_entry_hash_2 (const void *key)
{
  return_ISTRING_HASH_2 (((profile_entry_t *) key)->name);
}

static int
profile_hash_cmp (const void *x, const void *y)
{
  return_ISTRING_COMPARE (((profile_entry_t *) x)->name,
                          ((profile_entry_t *) y)->name);
}

static profile_entry_t *
add_profile_entry (const file_t *target)
{
  /* Look up the string in the hash.  If it's there, return it.  */
  profile_entry_t **slot = (profile_entry_t **) hash_find_slot (&profile_table,
								&(target->name));
  profile_entry_t *profile_entry  = *slot;
  profile_entry_t *new;

  if (!HASH_VACANT (profile_entry))
    return profile_entry;

  /* Not there yet so add it to a buffer, then into the hash table.  */
  new = xcalloc (sizeof (profile_entry_t));
  new->name = target->name;
  memcpy(&(new->floc), &(target->floc), sizeof(gmk_floc));

  hash_insert_at (&profile_table, new, slot);
  new->calls = NULL;
  return new;
}

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
  hash_init (&profile_table, 1000, profile_table_entry_hash_1,
	     profile_table_entry_hash_2, profile_hash_cmp);
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

#if 0
static unsigned int next_file_num = 0;
static unsigned int next_fn_num   = 1;
#endif

extern void
add_target(file_t *target, file_t *prev) {
  profile_entry_t *p = add_profile_entry(target);
  p->elapsed_time = target->elapsed_time;
  if (prev) {
    profile_entry_t *q = add_profile_entry(prev);
    if (q) {
      profile_call_t *new = CALLOC(profile_call_t, 1);
      new->p_target = target;
      new->p_next = q->calls;
      q->calls = new;
    }
  }
}

static void
print_profile_entry (const void *item)
{
  const profile_entry_t *p = item;
  profile_call_t *c;
  printf("name: %s, file: %s, line: %lu, time: %lu\n",
	 p->name, p->floc.filenm, p->floc.lineno, p->elapsed_time);
  for (c = p->calls; c; c = c->p_next) {
    printf("calls: %s\n", c->p_target->name);
  }
}

/* Print all profile entries */
static void
print_profile(struct hash_table *hash_table)
{
  hash_map (hash_table, print_profile_entry);
}

static void
callgrind_profile_entry (const void *item)
{
  const profile_entry_t *p = item;
  profile_call_t *c;
  if (p->floc.filenm) fprintf(callgrind_fd, "fl=%s\n\n", p->floc.filenm);
  fprintf(callgrind_fd, "fn=%s\n", p->name);
  fprintf(callgrind_fd, "%lu %lu\n", p->floc.lineno,
	  p->elapsed_time == 0 ? 1 : p->elapsed_time);
  for (c = p->calls; c; c = c->p_next) {
    if (c->p_target->floc.filenm)
      fprintf(callgrind_fd, "cfi=%s\n", c->p_target->floc.filenm);
    fprintf(callgrind_fd, "cfn=%s\n", c->p_target->name);
    fprintf(callgrind_fd, "calls=1 %lu\n", p->floc.lineno);
    fprintf(callgrind_fd, "%lu %lu\n", p->floc.lineno,
	    c->p_target->elapsed_time == 0 ? 1 : c->p_target->elapsed_time);
  }
  fprintf(callgrind_fd, "\n");
}

/* Print all profile entries */
void
callgrind_profile_data(struct hash_table *hash_table)
{
  hash_map (hash_table, callgrind_profile_entry);
}

extern void
close_callgrind(void) {
  print_profile(&profile_table);
  callgrind_profile_data(&profile_table);
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
    target2->prev = target;
    target2->elapsed_time = 500;
    add_target(target2, NULL);

    target3 = enter_file("all-recursive");
    target3->floc.filenm = "Makefile";
    target3->floc.lineno = 5;
    target3->prev = target2;
    target3->elapsed_time = 1000;
    add_target(target3, target2);

    close_callgrind();
  }
  return rc;
}
#endif
