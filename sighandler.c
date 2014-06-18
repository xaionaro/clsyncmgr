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

#include "sighandler.h"
#include "clsyncmgr.h"
#include "error.h"
#include <pthread.h>	/* pthread_create()	*/
#include <signal.h>	/* sigset_t		*/
#include <stdlib.h>	/* exit()		*/
#include <errno.h>	/* ETIME		*/

pthread_t pthread_sighandler;

enum sigusr_enum {
	SIGUSR_PTHREAD_GC	= 10,
//	SIGUSR_			= 12,
//	SIGUSR_			= 16
};

int sighandler_loop(sighandler_arg_t *sighandler_arg_p) {
	int ret;

	clsyncmgr_t *ctx_p	 = sighandler_arg_p->ctx_p;
	sigset_t *sigset_p       = sighandler_arg_p->sigset_p;

	debug(2, "starting signal handler.");

	while (1) {
		int signal;

		debug(3, "waiting for signal");
		ret = sigwait(sigset_p, &signal);

		if (ctx_p->state == STATE_STARTING) {

			switch(signal) {
				case SIGALRM:
					exit(ETIME);
				case SIGTERM:
				case SIGINT:
					// TODO: remove the exit() from here. Main thread should exit itself
					exit(0);
					break;
				default:
					warning("Got signal %i, but the main loop is not started, yet. Ignoring the signal.", signal);
					break;
			}
			continue;
		}

		debug(3, "got signal %i. ctx_p->state == %i.", signal, ctx_p->state);

		if (ret) {
			// TODO: handle an error here
		}

		switch (signal) {
			case SIGALRM:
			case SIGTERM:
			case SIGINT:
				clsyncmgr_switch_state(ctx_p, STATE_TERM);
				break;
			case SIGHUP:
				clsyncmgr_switch_state(ctx_p, STATE_REHASH);
				break;
			case SIGUSR_PTHREAD_GC:
				clsyncmgr_switch_state(ctx_p, STATE_PTHREAD_GC);
				break;
			default:
				error("Unknown signal: %i. Exit.\n", signal);
				clsyncmgr_switch_state(ctx_p, STATE_TERM);
				break;
		}
		if ((ctx_p->state == STATE_TERM) || (ctx_p->state == STATE_EXIT))
			break;
	}

	debug(2, "signal handler closed.");
	return 0;
}


int sighandler_run(clsyncmgr_t *ctx_p)
{
	int ret;
	static sighandler_arg_t sighandler_arg = {0};

	debug(3, "starting");

	// Creating signal handler thread
	static sigset_t sigset_sighandler;
	sigemptyset(&sigset_sighandler);
	sigaddset(&sigset_sighandler, SIGALRM);
	sigaddset(&sigset_sighandler, SIGHUP);
	sigaddset(&sigset_sighandler, SIGTERM);
	sigaddset(&sigset_sighandler, SIGINT);

	sigaddset(&sigset_sighandler, SIGUSR_PTHREAD_GC);

	ret = pthread_sigmask(SIG_BLOCK, &sigset_sighandler, NULL);
	if(ret) return ret;

	sighandler_arg.ctx_p		=  ctx_p;
	sighandler_arg.sigset_p		= &sigset_sighandler;
	ret = pthread_create(&pthread_sighandler, NULL, (void *(*)(void *))sighandler_loop, &sighandler_arg);
	if(ret) return ret;

	sigset_t sigset_parent;
	sigemptyset(&sigset_parent);
	pthread_sigmask(SIG_UNBLOCK, &sigset_parent, NULL);

	return 0;
}

int sighandler_stop(clsyncmgr_t *ctx_p)
{
	void *ret;

	pthread_kill(pthread_sighandler, SIGTERM);
	pthread_join(pthread_sighandler, &ret);
	return (int)(long)ret;
}

