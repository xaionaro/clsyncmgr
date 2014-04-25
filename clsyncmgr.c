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

#include <sys/types.h>	/* opendir()	*/
#include <dirent.h>	/* opendir()	*/
#include <string.h>	/* strdup()	*/
#include <stdio.h>	/* snprintf()	*/
#include <errno.h>	/* errno	*/
#include <libclsync.h>

int clsyncmgr_proc_clsync(socket_sockthreaddata_t *arg, sockcmd_t *sockcmd_p) {
	return 0;
}

int clsyncmgr_watchdir_lookup(const char **const watchdir_pp, clsyncmgr_t *glob_p)
{
	struct dirent *dirent;
	debug(4, "<%s>", *watchdir_pp);

	

	DIR *dir = opendir(*watchdir_pp);
	if (dir == NULL) {
		error("Cannot open directory \"%s\"", *watchdir_pp);
		return errno;
	}

	while (errno=0, (dirent=readdir(dir)) != NULL) {
		switch (dirent->d_type) {
			case DT_SOCK: {
				char buf[BUFSIZ];
				snprintf(buf, BUFSIZ, "%s/%s", *watchdir_pp, dirent->d_name);
				debug(5, "found socket: <%s>", buf);

				if(g_hash_table_lookup(glob_p->sock_unix, buf) != NULL) {
					debug(5, "already connected to <%s>, skipping", buf);
					continue;
				}

				clsyncproc_t *clsyncproc_p = clsync_connect_unix(buf, clsyncmgr_proc_clsync, 0);
				if (clsyncproc_p == NULL) {
					warning("Cannot connect to <%s> (errno %i: %s)", buf, errno, strerror(errno));
					continue;
				}
				debug(5, "connected to <%s>", buf);
				g_hash_table_insert(glob_p->sock_unix, strdup(buf), NULL);
				break;
			}
		}
	}

	if (errno)
		error("Cannot read an entry from directory \"%s\"", *watchdir_pp);

	closedir(dir);
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
	debug(2, "new state: %u", state_new);
	glob_p->state = state_new;
	return 0;
}

void clsyncmgr_idle(clsyncmgr_t *glob_p) {
	debug(3, "");

	dynamic_foreach(&glob_p->watchdirs, clsyncmgr_watchdir_lookup, glob_p);

	return;
}

int clsyncmgr(clsyncmgr_t *glob_p)
{
	glob_p->pthread_root = pthread_self();

	debug(3, "starting");
	clsyncmgr_switch_state(glob_p, STATE_STARTING);
	sighandler_run(glob_p);
	glob_p->sock_unix = g_hash_table_new_full(g_str_hash, g_str_equal, free, 0);

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
	g_hash_table_destroy(glob_p->sock_unix);

	clsyncmgr_switch_state(glob_p, STATE_EXIT);
	debug(3, "finished");
	return 0;
}

