/** \file libdebugger/command/help/source.h
 *
 *  \brief Help text for debugger command `source`.
 *
 */
#define source_HELP_TEXT \
  "Read debugger commands from the glob expansion of *file-glob*.\n"	\
  "\n"									\
  "*file-glob* should resolve after glob expansion to single file.\n"	\
  "\n"									\
  "Examples:\n"								\
  "--------\n"								\
  "\n"									\
  "   source /home/rocky/remake-dbgr.cmds  # absolute path\n"		\
  "   source ./remake-dbgr.cmds            # relative path\n"		\
  "   source remake-dbgr.cmds              # relative path - same as above\n" \
  "   source ~/remake-dbgr.cmds            # \"~\" is glob expanded\n"	\
  "   source ~/[r]emake-dbgr.cmds          # Same as above\n"		\
  "   source ~/remake-dbgr.* # Includes the above, but is an error if not unique\n"
