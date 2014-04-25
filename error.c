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

/*
 * This file implements way to output debugging information. It's supposed
 * to be slow but convenient functions.
 */

#include <stdlib.h>
#include <execinfo.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <stdarg.h>
#include <syslog.h>
#include "error.h"
#include "common.h"

static int printf_stderr(const char *fmt, ...) {
	va_list args;
	int rc;

	va_start(args, fmt);
	rc = vfprintf(stderr, fmt, args);
	va_end(args);

	return rc;
}

static int printf_stdout(const char *fmt, ...) {
	va_list args;
	int rc;

	va_start(args, fmt);
	rc = vfprintf(stdout, fmt, args);
	va_end(args);

	return rc;
}

static int vprintf_stderr(const char *fmt, va_list args) {
	return vfprintf(stderr, fmt, args);
}

static int vprintf_stdout(const char *fmt, va_list args) {
	return vfprintf(stdout, fmt, args);
}


static void flush_stderr(int level) {
	fprintf(stderr, "\n");
	fflush(stderr);
}

static void flush_stdout(int level) {
	fprintf(stdout, "\n");
	fflush(stdout);
}


static char _syslog_buffer[BUFSIZ];
size_t _syslog_buffer_filled = 0;
static int syslog_buf(const char *fmt, ...) {
	int len;
	va_list args;

	va_start(args, fmt);
	len = vsnprintf (
		&_syslog_buffer[_syslog_buffer_filled],
		BUFSIZ - _syslog_buffer_filled,
		fmt,
		args
	);
	va_end(args);

	if (len>0)
		_syslog_buffer_filled += len;

	return 0;
}
static int vsyslog_buf(const char *fmt, va_list args) {
	int len;

	len = vsnprintf (
		&_syslog_buffer[_syslog_buffer_filled],
		BUFSIZ - _syslog_buffer_filled,
		fmt,
		args
	);

	if (len>0)
		_syslog_buffer_filled += len;

	return 0;
}
static void syslog_flush(int level) {
	syslog(level, "%s", _syslog_buffer);
}

typedef int *(*outfunct_t)(const char *format, ...);
typedef int *(*voutfunct_t)(const char *format, va_list ap);
typedef void *(*flushfunct_t)(int level);

static outfunct_t outfunct[] = {
	[OM_STDERR]	= (outfunct_t)printf_stderr,
	[OM_STDOUT]	= (outfunct_t)printf_stdout,
	[OM_SYSLOG]	= (outfunct_t)syslog_buf,
};

static voutfunct_t voutfunct[] = {
	[OM_STDERR]	= (voutfunct_t)vprintf_stderr,
	[OM_STDOUT]	= (voutfunct_t)vprintf_stdout,
	[OM_SYSLOG]	= (voutfunct_t)vsyslog_buf,
};

static flushfunct_t flushfunct[] = {
	[OM_STDERR]	= (flushfunct_t)flush_stderr,
	[OM_STDOUT]	= (flushfunct_t)flush_stdout,
	[OM_SYSLOG]	= (flushfunct_t)syslog_flush,
};

void _critical(outputmethod_t method, const char *const function_name, const char *fmt, ...) {
	{
		va_list args;

		outfunct[method]("Critical: %s(): ", function_name);
		va_start(args, fmt);
		voutfunct[method](fmt, args);
		va_end(args);
		flushfunct[method](LOG_CRIT);
	}

	{
		void  *buf[BACKTRACE_LENGTH];
		char **strings;
		int backtrace_len = backtrace((void **)buf, BACKTRACE_LENGTH);

		strings = backtrace_symbols(buf, backtrace_len);
		if (strings == NULL) {
			outfunct[method]("_critical(): Got error, but cannot print the backtrace. Current errno: %u: %s\n",
				errno, strerror(errno));
			flushfunct[method](LOG_CRIT);
			exit(EXIT_FAILURE);
		}

		for (int j = 1; j < backtrace_len; j++) {
			outfunct[method]("        %s", strings[j]);
			flushfunct[method](LOG_CRIT);
		}
	}

	exit(errno);

	return;
}

void _error(outputmethod_t method, const char *const function_name, const char *fmt, ...) {
	va_list args;

	outfunct[method]("Error: %s(): ", function_name);
	va_start(args, fmt);
	voutfunct[method](fmt, args);
	va_end(args);
	flushfunct[method](LOG_ERR);

	return;
}

void _info(outputmethod_t method, const char *const function_name, const char *fmt, ...) {
	va_list args;

	outfunct[method]("Info: %s(): ", function_name);
	va_start(args, fmt);
	voutfunct[method](fmt, args);
	va_end(args);
	flushfunct[method](LOG_INFO);

	return;
}

void _warning(outputmethod_t method, const char *const function_name, const char *fmt, ...) {
	va_list args;

	outfunct[method]("Warning: %s(): ", function_name);
	va_start(args, fmt);
	voutfunct[method](fmt, args);
	va_end(args);
	flushfunct[method](LOG_WARNING);

	return;
}

void _debug(outputmethod_t method, int debug_level, const char *const function_name, const char *fmt, ...) {
	va_list args;

	outfunct[method]("Debug%u: %s(): ", debug_level, function_name);
	va_start(args, fmt);
	voutfunct[method](fmt, args);
	va_end(args);
	flushfunct[method](LOG_DEBUG);

	return;
}

