/* Continue until the next command to be executed. */
#define DEPENDS_COMMANDS " depends commands"
static debug_return_t 
dbg_cmd_list(char *psz_arg)
{
  const char *psz_target = NULL;
  char   *target_cmd = NULL;
  file_t *p_target;

  if (psz_arg && 0 == strcmp(psz_arg, "-")) {
    /* Show info for parent target. */
    if (p_stack_top) {
      /* We have a target stack  */
      target_stack_node_t *p=p_stack;

      if (!p) {
	printf(_("We don't seem to have a target to get parent of.\n"));
	return debug_cmd_error;
      }
    
      p = p->p_parent;
      if (!p) {
	printf(_("We don't seem to have a parent target.\n"));
	return debug_cmd_error;
      }
      p_target = p->p_target;
      psz_target = p_target->name;
    } else {
	printf(_("We don't seem to have a target stack to get parent of.\n"));
	return debug_cmd_error;
    }
  } else {
    p_target = get_target(&psz_arg, &psz_target);
  }

  if (!p_target) {
    printf(_("Trouble getting a target name.\n"));
    return debug_cmd_error;
  }
  target_cmd = CALLOC(char, strlen(psz_target) + 1 + strlen(DEPENDS_COMMANDS));
  sprintf(target_cmd, "%s%s", psz_target, DEPENDS_COMMANDS);
  return dbg_cmd_target(target_cmd);
}

static void
dbg_cmd_list_init(void) 
{
  short_command['l'].func = &dbg_cmd_list;
  short_command['l'].use = _("list [TARGET]");
  short_command['l'].doc = 
    _("List target dependencies and commands. Without a target name we\n"
"use the current target. A target name of '-' will use the parent target on\n"
"the target stack.\n"
 );
}


/* 
 * Local variables:
 * eval: (c-set-style "gnu")
 * indent-tabs-mode: nil
 * End:
 */
