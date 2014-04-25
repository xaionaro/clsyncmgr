/*
    clsyncmgr - intermediate daemon to aggregate clsync's sockets

    Copyright (C) 2014  Dmitry Yu Okunev <dyokunev@ut.mephi.ru> 0x8E30679C

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "clsyncmgr.h"
#include "sighandler.h"

#include <libclsync.h>

int clsyncmgr_watchdir_remove_all(clsyncmgr_t *glob_p)
{
	return 0;
}

int clsyncmgr_watchdir_add(clsyncmgr_t *glob_p, char *watchdir)
{
	return 0;
}

int clsyncmgr(clsyncmgr_t *glob_p) {
	sighandler_run(glob_p);

	return 0;
}

