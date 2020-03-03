.. index:: source
.. _source:

Read and Run Debugger Commands from a File (source)
---------------------------------------------------

**source** *file-glob*

Read debugger commands from the glob expansion of *file-glob*;

Examples:
+++++++++

::

        source /home/rocky/remake-dbgr.cmds  # absolute path
        source ./remake-dbgr.cmds            # relative path
        source remake-dbgr.cmds              # relative path - same as above
        source ~/remake-dbgr.cmds            # "~" is glob expanded
        source ~/[r]emake-dbgr.cmds          # Same as above
        source ~/remake-dbgr.* # Includes the above, but is an error if not unique
