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


#include "common.h"

#include <string.h>	/* strdup() */
#include <errno.h>	/* EINVAL   */

#include "malloc.h"
#include "error.h"
#include "clsyncmgr.h"

typedef void *callback_info_t;
int control_answer(clsyncmgr_t *ctx_p, callback_info_t cb, const char *const answer)
{
	return 0;
}

int control_execute(clsyncmgr_t *ctx_p, callback_info_t cb, const char *const _cmd)
{
	char *cmd = strdup(_cmd);
	char *ptr = cmd;
	char *label, *cmd_id_s, *args = NULL;
	int i = 0;

	do {
		char *next;

		if (i < 2) {
			next = strpbrk(ptr, " \t");
			if (next != NULL) {
				char *p;

				 p = next;
				*p = 0;
				while (*p == ' ' || *p == '\t') p++;
			}
		}
		switch (i) {
			case 0:
				label    = strdup(ptr);
				i++;
				break;
			case 1:
				cmd_id_s = strdup(ptr);
				i++;
				break;
			case 2:
				args     = strdup(ptr);
				i++;
				break;
		}

		ptr = next;
	} while (ptr);

	if (i < 2) {
		control_answer(ctx_p, cb, "Not enough arguments. Syntax: label cmd_id arguments");
		return EINVAL;
	}

	free(cmd);
	return 0;
}

#if 0

int control_run(clsyncmgr_t *ctx_p)
{
	debug(2, "socket path: <%s>", ctx_p->socketpath);

	return 0;
}

int control_cleanup(ctx_t *ctx_p)
{
	debug(2, "");

	if (ctx_p->socketpath != NULL) {
		unlink(ctx_p->socketpath);
		closecontrol(ctx_p);
		// TODO: kill pthread_control and join
//		pthread_join(pthread_control, NULL);
	}
	return 0;
}

#endif
