/* 
   Delete some breakpoints. Arguments are breakpoint numbers with spaces 
   in between."To delete all breakpoints, give no argument.
*/
static debug_return_t 
dbg_cmd_delete (char *psz_args)
{
  int i_brkpt;
  char *psz_word;

  if (!psz_args || !*psz_args) {
    while(i_breakpoints) 
      remove_breakpoint(1);
    return debug_readloop;
  }
  
  psz_word = get_word(&psz_args);
  while ( psz_word && *psz_word ) {
    if (get_int(psz_word, &i_brkpt, true)) {
      remove_breakpoint(i_brkpt);
    }
    psz_word = get_word(&psz_args);
  }

  return debug_readloop;
}

