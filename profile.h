#include <time.h>
#include "filedef.h"
extern bool init_callgrind(const char *creator, const char *const *argv,
			   const char *program_status);
extern void add_target(file_t *target, file_t *prev);
extern void close_callgrind(void);
