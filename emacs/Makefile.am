#   Copyright (C) 2004, 2005, 2007, 2008 Rocky Bernstein rocky@gnu.org
#
#   This is free software; you can redistribute it and/or modify it under
#   the terms of the GNU General Public License as published by the Free
#   Software Foundation; either version 2, or (at your option) any later
#   version.
#
#   Bash is distributed in the hope that it will be useful, but WITHOUT ANY
#   WARRANTY; without even the implied warranty of MERCHANTABILITY or
#   FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
#   for more details.
#   
#   You should have received a copy of the GNU General Public License along
#   with Bash; see the file COPYING.  If not, write to the Free Software
#   Foundation, 59 Temple Place, Suite 330, Boston, MA 02111 USA.
#$Id: Makefile.am,v 1.5 2008/01/11 13:59:55 rockyb Exp $

EXTRA_DIST = elk-test.el mdb.el mdb.el.in mdb-test.el.in
ELCFILES  = mdb.elc
if INSTALL_EMACS_LISP
lisp_LISP = mdb.el 

check:
	$(EMACS) -batch -q -l $(top_builddir)/emacs/mdb-test.el $(srcdir)/elk-test.el
endif
test: check
