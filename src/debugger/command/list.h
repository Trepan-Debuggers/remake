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
	dbg_errmsg(_("We don't seem to have a target to get parent of."));
	return debug_cmd_error;
      }
    
      p = p->p_parent;
      if (!p) {
	dbg_errmsg(_("We don't seem to have a parent target."));
	return debug_cmd_error;
      }
      p_target = p->p_target;
      psz_target = p_target->name;
    } else {
	dbg_errmsg(_("We don't seem to have a target stack to get parent of."));
	return debug_cmd_error;
    }
  } else {
    unsigned int u_lineno=0;
    f2l_entry_t entry_type;
    if (get_uint(psz_arg, &u_lineno, false)) {
      if (p_stack) {
        p_target = target_for_file_and_line(p_stack->p_target->floc.filenm,
                                            u_lineno, &entry_type);
        if (!p_target) {
          dbg_errmsg("Can't find target or pattern on line %s.\n" 
                     "Use 'info lines' to get a list of and pattern lines.", 
                     psz_arg);
          return debug_cmd_error;
        }
        psz_target = p_target->name;
      } else {
	dbg_errmsg(_("We don't seem to have a target stack to get parent of."));
	return debug_cmd_error;
      }
    } else 
      p_target = get_target(&psz_arg, &psz_target);
  }

  if (!p_target) {
    dbg_errmsg(_("Trouble getting a target name for %s."), psz_target);
    return debug_cmd_error;
  }
  print_floc_prefix(&p_target->floc);
  target_cmd = CALLOC(char, strlen(psz_target) + 1 + strlen(DEPENDS_COMMANDS));
  sprintf(target_cmd, "%s%s", psz_target, DEPENDS_COMMANDS);
  return dbg_cmd_target(target_cmd);
}

static void
dbg_cmd_list_init(unsigned int c) 
{
  short_command[c].func = &dbg_cmd_list;
  short_command[c].use = _("list [TARGET|LINE-NUMBER]");
  short_command[c].doc = 
    _("List target dependencies and commands for TARGET or LINE NUMBER.\n"
"Without a target name or line number, use the current target.\n"
"A target name of '-' will use the parent target on the target stack.\n"
 );
}


/* 
 * Local variables:
 * eval: (c-set-style "gnu")
 * indent-tabs-mode: nil
 * End:
 */
