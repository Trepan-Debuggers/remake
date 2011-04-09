#ifndef REMAKE_DBG_SUBCMD_H
#define REMAKE_DBG_SUBCMD_H

typedef struct {
  const char *name;	  /* name of subcommand command. */
  const char *short_doc;  /* short description of subcommand */
  const char *doc;	  /* full description of subcommand */ 
  void *var;	          /* address of variable setting. NULL if no
			     setting. */
  bool b_onoff;           /* True if on/off variable, false if int. 
			     FIXME: generalize into enumeration.
			   */
  unsigned int min_abbrev; /* Fewest number of characters needed
                              to match name. */
} subcommand_var_info_t;

extern void dbg_help_subcmd_entry(const char *psz_subcmd_name, 
				  const char *psz_fmt,
				  subcommand_var_info_t *p_subcmd, 
				  bool full_info);
#endif /* DBG_CMD_H*/
/* 
 * Local variables:
 *  c-file-style: "gnu"
 *  indent-tabs-mode: nil
 * End:
 */
