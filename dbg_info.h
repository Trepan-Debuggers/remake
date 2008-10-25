/* $Id: dbg_info.h,v 1.11 2006/03/19 12:17:44 rockyb Exp $
Copyright (C) 2008 R. Bernstein <rocky@gnu.org>
This file is part of GNU Make (remake variant).

GNU Make is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2, or (at your option)
any later version.

GNU Make is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with GNU Make; see the file COPYING.  If not, write to
the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
Boston, MA 02111-1307, USA.  */

/** \file dbg_info.h 
 *
 *  \brief debugger info headers
*/

#ifndef DBG_INFO_H
#define DBG_INFO_H

#include "dbg_cmd.h"
#include "trace.h"

extern debug_enter_reason_t last_stop_reason;
extern const char *WARRANTY;

debug_return_t dbg_cmd_info (char *psz_arg);

#endif /* DBG_INFO_H*/
