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
#include "control.h"
#include "terminal.h"

#include <sys/types.h>	/* opendir()	*/
#include <dirent.h>	/* opendir()	*/
#include <string.h>	/* strdup()	*/
#include <stdio.h>	/* snprintf()	*/
#include <errno.h>	/* errno	*/
#include <libclsync.h>

int clsyncmgr_proc_clsync(socket_sockthreaddata_t *thread, sockcmd_t *sockcmd_p)
{
//	ctx_t *ctx_p = thread->arg;

	return 0;
}

int proc_clsync_gotinfo(socket_sockthreaddata_t *thread, sockcmd_t *sockcmd_p, void *_clsyncproc_p)
{
	clsyncproc_t *clsyncproc_p = _clsyncproc_p;

	clsyncmgr_t *ctx_p = thread->arg;
	sockcmd_dat_info_t *cmddat  =    sockcmd_p->data;
	clsyncproc_data_t  *procdat = clsyncproc_p->data;

	strncpy(procdat->label, cmddat->label, sizeof(procdat->label));
	g_hash_table_insert(ctx_p->sock_unix_ht_label, strdup(procdat->label), clsyncproc_p);

	debug(4, "New label: \"%s\"", procdat->label);
	return 0;
}

int clsyncmgr_socketpath_connect(const char **const socketpath_pp, clsyncmgr_t *ctx_p)
{
	const char *socketpath = *socketpath_pp;

	debug(5, "<%s>", socketpath);

	if (g_hash_table_lookup(ctx_p->sock_unix_ht_path, socketpath) != NULL) {
		debug(4, "already connected to <%s>, skipping", socketpath);
		return 0;
	}

	clsyncproc_t *clsyncproc_p = clsync_connect_unix(socketpath, clsyncmgr_proc_clsync, 0);
	if (clsyncproc_p == NULL) {
		warning("Cannot connect to <%s> (errno %i: %s)", socketpath, errno, strerror(errno));
		return errno;
	}

	clsyncproc_p->data = xcalloc(1, sizeof(clsyncproc_data_t));

	debug(3, "connected to <%s>", socketpath);
	socket_send_cb(clsyncproc_p->sock_p, SOCKCMD_REQUEST_INFO, proc_clsync_gotinfo, clsyncproc_p);
	g_hash_table_insert(ctx_p->sock_unix_ht_path,  strdup(socketpath), clsyncproc_p);

	return 0;
}

int clsyncmgr_socketpath_disconnect(const char **const socketpath_pp, clsyncmgr_t *ctx_p)
{
	debug(4, "<%s>", *socketpath_pp);

	return 0;
}

int clsyncmgr_socketpath_add(clsyncmgr_t *ctx_p, const char *const socketpath)
{
	char **socketpath_pp = dynamic_add(&ctx_p->socketpaths);
	*socketpath_pp = strdup(socketpath);

	return 0;
}

int clsyncmgr_socketpath_remove_all(clsyncmgr_t *ctx_p)
{
	debug(3, "");

	dynamic_foreach(&ctx_p->socketpaths, clsyncmgr_socketpath_disconnect, ctx_p);
	dynamic_reset(&ctx_p->socketpaths, free);
	return 0;
}

int clsyncmgr_watchdir_lookup(const char **const watchdir_pp, clsyncmgr_t *ctx_p)
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

				clsyncmgr_socketpath_connect((const char **)&buf, ctx_p);
				break;
			}
		}
	}

	if (errno)
		error("Cannot read an entry from directory \"%s\"", *watchdir_pp);

	closedir(dir);
	return 0;
}

int clsyncmgr_watchdir_disconnect(const char **const watchdir_pp, clsyncmgr_t *ctx_p)
{
	debug(4, "<%s>", *watchdir_pp);

	return 0;
}

int clsyncmgr_watchdir_add(clsyncmgr_t *ctx_p, const char *const watchdir)
{
	char **watchdir_pp = dynamic_add(&ctx_p->watchdirs);
	*watchdir_pp = strdup(watchdir);

	return 0;
}

int clsyncmgr_watchdir_remove_all(clsyncmgr_t *ctx_p)
{
	debug(3, "");

	dynamic_foreach(&ctx_p->watchdirs, clsyncmgr_watchdir_disconnect, ctx_p);
	dynamic_reset(&ctx_p->watchdirs, free);
	return 0;
}

int clsyncmgr_switch_state(clsyncmgr_t *ctx_p, state_t state_new)
{
	debug(2, "new state: %u", state_new);
	ctx_p->state = state_new;
	return 0;
}

void clsyncmgr_idle(clsyncmgr_t *ctx_p)
{
	debug(3, "");

	dynamic_foreach(&ctx_p->watchdirs, clsyncmgr_watchdir_lookup, ctx_p);

	return;
}

int clsyncmgr(clsyncmgr_t *ctx_p)
{
	errno = 0;
	ctx_p->pthread_root = pthread_self();

	debug(3, "starting");
	clsyncmgr_switch_state(ctx_p, STATE_STARTING);
	sighandler_run(ctx_p);
	ctx_p->sock_unix_ht_path  = g_hash_table_new_full(g_str_hash, g_str_equal, free, 0);
	ctx_p->sock_unix_ht_label = g_hash_table_new_full(g_str_hash, g_str_equal, free, 0);

	if ((errno=dynamic_foreach(&ctx_p->socketpaths, clsyncmgr_socketpath_connect, ctx_p)))
		error("Got error while connecting to sockets");

	if (!errno)
		if (!ctx_p->flags[FL_EXECUTE])
			terminal_run(ctx_p);
/*
	if (!errno)
		control_run(ctx_p);
*/
	if (!errno) {
		clsyncmgr_switch_state(ctx_p, STATE_RUNNING);
		if (ctx_p->flags[FL_EXECUTE]) {
			control_execute(ctx_p, ctx_p->execute);
		} else {
			while (ctx_p->state == STATE_RUNNING) {
				clsyncmgr_idle(ctx_p);
				sleep(1);
				break;
			}
		}
		clsyncmgr_switch_state(ctx_p, STATE_CLEANUP);
	}

	debug(3, "cleanup");

	if (!ctx_p->flags[FL_EXECUTE])
		terminal_cleanup(ctx_p);
//	control_cleanup(ctx_p);

	sighandler_stop(ctx_p);
	clsyncmgr_watchdir_remove_all(ctx_p);
	g_hash_table_destroy(ctx_p->sock_unix_ht_path);
	g_hash_table_destroy(ctx_p->sock_unix_ht_label);

	clsyncmgr_switch_state(ctx_p, STATE_EXIT);
	debug(3, "finished");
	return 0;
}

