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

#ifndef __CLSYNCMGR_COMMON_H
#define __CLSYNCMGR_COMMON_H

#define PROGRAM "clsyncmgr"
#define AUTHOR "Dmitry Yu Okunev <dyokunev@ut.mephi.ru> 0x8E30679C"
#define VERSION_MAJ 0
#define VERSION_MIN 3

#include <unistd.h>	/* gid_t */
#include <sys/types.h>	/* gid_t */

#ifndef MIN
#define MIN(a,b) ((a)>(b)?(b):(a))
#endif

#ifndef MAX
#define MAX(a,b) ((a)>(b)?(a):(b))
#endif

#ifdef _GNU_SOURCE
#	ifndef likely
#		define likely(x)    __builtin_expect(!!(x), 1)
#	endif
#	ifndef unlikely
#		define unlikely(x)  __builtin_expect(!!(x), 0)
#	endif
#else
#	ifndef likely
#		define likely(x)   (x)
#	endif
#	ifndef unlikely
#		define unlikely(x) (x)
#	endif
#endif

#include "configuration.h"

#define FLM_MAX		(1<<10)
#define FLM_LONGOPTONLY	(1<<9)
#define FLM_CONFIGONLY	(1<<8)

#define _TEMPLATE(X, Y) X ## _ ## Y
#define TEMPLATE(X, Y) _TEMPLATE(X, Y)

enum flag {
	FL_HELP			= 'h',
	FL_SHOW_VERSION		= 'V',
	FL_QUIET		= 'q',
	FL_DEBUG		= 'D',
	FL_BACKGROUND		= 'b',
	FL_OUTPUT_METHOD	= 'O',
	FL_CONFIGFILE		= 'H',
	FL_CONFIGBLOCK		= 'K',
	FL_UID			= 'u',
	FL_GID			= 'g',
	FL_PIDFILE		= 'z',
	FL_SYSLOG		= 'Y',
	FL_SOCKETPATH		= 's',
	FL_WATCHDIR		= 'W',
	FL_CONNECT		= 'c',
	FL_EXECUTE		= 'e',

	FL_SOCKETAUTH		=  0|FLM_LONGOPTONLY,
	FL_SOCKETMOD		=  1|FLM_LONGOPTONLY,
	FL_SOCKETOWN		=  2|FLM_LONGOPTONLY,

};
typedef enum flag flag_t;

#endif

