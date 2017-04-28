#include "debug.h"
#include "file.h"
#include "mock.h"
#include "trace.h"

#include <assert.h>

char *program = "remake-mock";

void
die (int status)
{
  exit (status);
}

debug_return_t enter_debugger (target_stack_node_t *p,
			       file_t *p_target, int errcode,
			       debug_enter_reason_t reason)
{
  printf("%p %p %d %u\n", p, p_target, errcode, reason);
  return continue_execution;
}

/*! Show target information: location and name. */
extern void
print_file_target_prefix (const file_t *p_target)
{
  printf("%p\n", p_target);
}


void
fatal (const floc_t *flocp, const char *fmt, ...)
{

  if (flocp && flocp->filenm)
    fprintf (stderr, "%s:%lu: *** ", flocp->filenm, flocp->lineno);
  else if (makelevel == 0)
    fprintf (stderr, "%s: *** ", program);
  else
    fprintf (stderr, "%s[%u]: *** ", program, makelevel);

  fputs (_(".  Stop.\n"), stderr);
  fmt;
}

/* Look up a file record for file NAME and return it.
   Create a new record if one doesn't exist.  NAME will be stored in the
   new record so it should be constant or in the strcache etc.
 */

file_t  *
enter_file (const char *name, const floc_t *p_floc)
{
  struct file *f;
  struct file *new;
  struct file **file_slot;
  struct file file_key;

  assert (*name != '\0');

#if defined(VMS) && !defined(WANT_CASE_SENSITIVE_TARGETS)
  if (*name != '.')
    {
      const char *n;
      char *lname, *ln;
      lname = xstrdup (name);
      for (n = name, ln = lname; *n != '\0'; ++n, ++ln)
        if (isupper ((unsigned char)*n))
          *ln = tolower ((unsigned char)*n);
        else
          *ln = *n;

      *ln = '\0';
      name = strcache_add (lname);
      free (lname);
    }
#endif

  file_key.hname = name;
  file_slot = (struct file **) hash_find_slot (&files, &file_key);
  f = *file_slot;
  if (! HASH_VACANT (f) && !f->double_colon) {
    if (f->floc.filenm == '\0' && f->floc.lineno == 0 && p_floc &&
	p_floc->filenm != '\0' && p_floc->lineno != 0) {
      f->floc.filenm = strdup(p_floc->filenm);
      f->floc.lineno = p_floc->lineno;
    }
    return f;
  }

  new = xcalloc (sizeof (struct file));
  new->name = new->hname = name;
  new->update_status = -1;
  new->tracing = BRK_NONE;
  if (p_floc) {
    new->floc = *p_floc;
  } else {
    new->floc.lineno = 0;
    new->floc.filenm = NULL;
  }

  if (HASH_VACANT (f))
    {
      new->last = new;
      hash_insert_at (&files, new, file_slot);
    }
  else
    {
      /* There is already a double-colon entry for this file.  */
      new->double_colon = f;
      f->last->prev = new;
      f->last = new;
    }

  return new;
}
