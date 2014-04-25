/*
    clsync - file tree sync utility based on fanotify and inotify

    Copyright (C) 2013  Dmitry Yu Okunev <dyokunev@ut.mephi.ru> 0x8E30679C

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

#ifndef __CLSYNCMGR_MALLOC_H
#define __CLSYNCMGR_MALLOC_H

#include <stdlib.h>

#define dynamic_T(type)\
struct {\
	type   *dat;\
	size_t  num;\
	size_t  num_allocated;\
}

typedef dynamic_T(void) dynamic_t;

typedef void (*freefunct_t)(void *);
typedef int (*dynamic_procfunct_t)(void *);

extern void *xmalloc(size_t size);
extern void *xcalloc(size_t nmemb, size_t size);
extern void *xrealloc(void *oldptr, size_t size);

extern void *_dynamic_add    (dynamic_t *, size_t membsize);
extern void  _dynamic_foreach(dynamic_t *, dynamic_procfunct_t funct,     size_t membsize);
extern void  _dynamic_reset  (dynamic_t *, freefunct_t         freefunct);
#define dynamic_add(a)              _dynamic_add    ((void *)(a), sizeof( *((a)->dat) ))
#define dynamic_foreach(a, funct)   _dynamic_foreach((void *)(a), (dynamic_procfunct_t)funct, sizeof( *((a)->dat) ))
#define dynamic_reset(a, freefunct) _dynamic_reset  ((void *)(a), freefunct)

#endif

