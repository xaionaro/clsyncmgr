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

#ifndef __CLSYNCMGR_CLSYNCMGR_H
#define __CLSYNCMGR_CLSYNCMGR_H

#include "common.h"
#include "malloc.h"
#include <pthread.h>	/* pthread_t  */
#include <glib.h>	/* GHashTable */

enum state_enum {
	STATE_UNKNOWN = 0,
	STATE_STARTING,
	STATE_RUNNING,
	STATE_REHASH,
	STATE_CLEANUP,
	STATE_TERM,
	STATE_EXIT,
	STATE_PTHREAD_GC,
};
typedef enum state_enum state_t;

struct clsyncproc_data {
	char label[MAXLABELLENGTH];
};
typedef struct clsyncproc_data clsyncproc_data_t;

struct clsyncmgr {
	int flags    [FLM_MAX];
	int flags_set[FLM_MAX];

	char *config_path;
	char *config_block;

	uid_t uid;
	gid_t gid;

	char *socketpath;
	int socket;
	mode_t socketmod;
	uid_t  socketuid;
	gid_t  socketgid;

	char *pidfile;

	state_t state;

	pthread_t pthread_root;

	dynamic_T(char *) watchdirs;
	dynamic_T(char *) socketpaths;

	char *execute;

	GHashTable *sock_unix_ht_path;
	GHashTable *sock_unix_ht_label;
};
typedef struct clsyncmgr clsyncmgr_t;

extern int clsyncmgr_switch_state(clsyncmgr_t *ctx_p, state_t state_new);
extern int clsyncmgr_watchdir_add(clsyncmgr_t *ctx_p, const char *const watchdir);
extern int clsyncmgr_watchdir_remove_all(clsyncmgr_t *ctx_p);
extern int clsyncmgr_socketpath_add(clsyncmgr_t *ctx_p, const char *const socketpath);
extern int clsyncmgr_socketpath_remove_all(clsyncmgr_t *ctx_p);
extern int clsyncmgr(clsyncmgr_t *ctx_p);

#endif

