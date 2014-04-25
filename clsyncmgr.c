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
#include "error.h"

#include <string.h>	/* strdup() */
#include <libclsync.h>

int clsyncmgr_watchdir_lookup(const char **const watchdir_pp, clsyncmgr_t *glob_p)
{
	debug(4, "<%s>", *watchdir_pp);

	return 0;
}

int clsyncmgr_watchdir_disconnect(const char **const watchdir_pp, clsyncmgr_t *glob_p)
{
	debug(4, "<%s>", *watchdir_pp);

	return 0;
}

int clsyncmgr_watchdir_add(clsyncmgr_t *glob_p, const char *const watchdir)
{
	char **watchdir_pp = dynamic_add(&glob_p->watchdirs);
	*watchdir_pp = strdup(watchdir);

	return 0;
}

int clsyncmgr_watchdir_remove_all(clsyncmgr_t *glob_p)
{
	debug(3, "");

	dynamic_foreach(&glob_p->watchdirs, clsyncmgr_watchdir_disconnect, glob_p);
	dynamic_reset(&glob_p->watchdirs, free);
	return 0;
}

int clsyncmgr_switch_state(clsyncmgr_t *glob_p, state_t state_new)
{
	debug(3, "new state: %u", state_new);
	glob_p->state = state_new;
	return 0;
}

void clsyncmgr_idle(clsyncmgr_t *glob_p) {

	dynamic_foreach(&glob_p->watchdirs, clsyncmgr_watchdir_lookup, glob_p);

	return;
}

int clsyncmgr(clsyncmgr_t *glob_p)
{
	glob_p->pthread_root = pthread_self();

	debug(3, "starting");
	clsyncmgr_switch_state(glob_p, STATE_STARTING);

	sighandler_run(glob_p);

	clsyncmgr_switch_state(glob_p, STATE_RUNNING);
	while(glob_p->state == STATE_RUNNING) {
		clsyncmgr_idle(glob_p);
		sleep(1);
		break;
	}
	clsyncmgr_switch_state(glob_p, STATE_CLEANUP);

	debug(3, "cleanup");

	sighandler_stop(glob_p);
	clsyncmgr_watchdir_remove_all(glob_p);

	clsyncmgr_switch_state(glob_p, STATE_EXIT);
	debug(3, "finished");
	return 0;
}

