.. index:: target
.. _target:

Target (Examining Targets)
--------------------------

**target** [*target-name*] [*info1* [*info2*...]]

Show information about a *target-name*.

*target-name* be the name of a target or it can be a variable like `@`
(the current target) or `<` (first dependency). If *target-name* is
omitted use the current target.

When `remake` enters the debugger, `remake` spontaneously prints the
line and target name that is under consideration and the location of
this target.  Likewise, when you select a target frame, the default
target name is changed.


The following attributes names can be given after a target name:

* attributes: rule attributes
   - precious,
   - rule search,
   - and pattern stem
* commands:   shell commands that need to be run to update the target
* depends: all target dependencies, i.e. order and non-order
* expand: like 'commands', but Makefile variables are expanded
* nonorder:" non-order dependencies", i.e. dependencies that are not ordered
* order: "order dependencies", i.e. dependencies that have to be run in a particular order
* previous: previous target name when there are multiple double-colons
* state: target status:
   - successully updated
   - needs to be updated
   - failed to be udated
   - invalid, an error of some sort occurred
* time: last modification time and whether file has been updated. If
  the target is not up to date you will see the message "File is very
  old." If a target is "phony", i.e. doesn't have file associated
  with it, the message "File does not exist."  will appear instead of
  the time. In some cases you may see "Modification time never
  checked."
* variables: automatically set variables such as `@` or  `<`

Note that there are other automatic variables defined based on
these. In particular those that have a `D` or `F` suffix, e.g. `$(@@D)`,
or `$(*F)`. These however are not listed here but can shown in a
`print` command or figured out from their corresponding
single-letter variable name.

.. seealso::

   :ref:`print <print>`.
