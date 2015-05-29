#include "filedef.h"
extern int file_hash_cmp (const void *x, const void *y);
extern void init_hash_files (void);
extern file_t *lookup_file (const char *name);
extern file_t *enter_file (const char *name);
