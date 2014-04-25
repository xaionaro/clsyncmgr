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

#include <pthread.h> /* pthread_self() */

enum outputmethod {
	OM_STDERR,
	OM_STDOUT,
	OM_SYSLOG,

	OM_MAX
};
typedef enum outputmethod outputmethod_t;

extern void _critical(pthread_t thread, outputmethod_t method, const char *const function_name, const char *fmt, ...);
#define critical(...) _critical(pthread_self(), glob_p->flags[FL_OUTPUT_METHOD], __FUNCTION__, __VA_ARGS__)
#define critical_noglob(...) _critical(pthread_self(), OM_STDERR, __FUNCTION__, __VA_ARGS__)

extern void _error(pthread_t thread, outputmethod_t method, const char *const function_name, const char *fmt, ...);
#define error(...) if (!glob_p->flags[FL_QUIET]) _error(pthread_self(), glob_p->flags[FL_OUTPUT_METHOD], __FUNCTION__, __VA_ARGS__)

extern void _warning(pthread_t thread, outputmethod_t method, const char *const function_name, const char *fmt, ...);
#define warning(...) if (!glob_p->flags[FL_QUIET]) _warning(pthread_self(), glob_p->flags[FL_OUTPUT_METHOD], __FUNCTION__, __VA_ARGS__)

extern void _info(pthread_t thread, outputmethod_t method, const char *const function_name, const char *fmt, ...);
#define info(...) if (!glob_p->flags[FL_QUIET]) _info(pthread_self(), glob_p->flags[FL_OUTPUT_METHOD], __FUNCTION__, __VA_ARGS__)

extern void _debug(pthread_t thread, outputmethod_t method, int debug_level, const char *const function_name, const char *fmt, ...);
#define debug(debug_level, ...) if (unlikely(glob_p->flags[FL_DEBUG] >= debug_level)) _debug(pthread_self(), glob_p->flags[FL_OUTPUT_METHOD], debug_level, __FUNCTION__, __VA_ARGS__)

