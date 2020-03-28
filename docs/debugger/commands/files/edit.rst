.. index:: edit
.. _edit:

Edit Makefile (`edit`)
----------------------

**edit**

Edit Make at the current target location.

The editing program of your choice is invoked with the current line set to
the active line in the program.

You can customize to use any editor you want by using the `EDITOR`
environment variable. The only restriction is that your editor, .e.g.
`ex`, recognizes the following command-line syntax:

::

   ex +*number* *filename*

The optional numeric value *+number* specifies the number of the
line in the file where to start editing.  For example, to configure
`remake` to use the `emacs` editor, you could use these commands
with the in a POSIX shell:

::

   EDITOR=/usr/bin/emacs
   export EDITOR
   remake ...

.. seealso::

   :ref:`list <list>`
