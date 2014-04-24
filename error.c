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

#include <stdlib.h>
#include <execinfo.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <stdarg.h>
#include "error.h"

void error(const char *fmt, ...) {
	{
		va_list args;

		fprintf(stderr, "Error: ");
		va_start(args, fmt);
		vfprintf(stderr, fmt, args);
		va_end(args);
		fprintf(stderr, "\n");
	}

	{
		void  *buf[BACKTRACE_LENGTH];
		char **strings;
		int backtrace_len = backtrace((void **)buf, BACKTRACE_LENGTH);

		strings = backtrace_symbols(buf, backtrace_len);
		if (strings == NULL) {
			fprintf(stderr, "error(): Got error, but cannot print the backtrace. Current errno: %u: %s\n",
				errno, strerror(errno));
			exit(EXIT_FAILURE);
		}

		for (int j = 1; j < backtrace_len; j++)
			fprintf(stderr, "%s\n", strings[j]);
	}

	exit(errno);
}
