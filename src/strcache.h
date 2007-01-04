/* String caching  */
void strcache_init (void);
void strcache_print_stats (const char *prefix);
int strcache_iscached (const char *str);
const char *strcache_add (const char *str);
const char *strcache_add_len (const char *str, int len);
int strcache_setbufsize (int size);

