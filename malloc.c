/*
    clsyncmgr - intermediate daemon to aggregate clsync's sockets

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

#include <string.h>	/* memset() */
#include <errno.h>	/* EINVAL   */
#include "configuration.h"
#include "malloc.h"
#include "error.h"

void *xmalloc(size_t size) {
#ifdef PARANOID
	size++;	// Just in case
#endif

	void *ret = malloc(size);

	if(ret == NULL)
		critical("malloc(%u): Cannot allocate memory", size);

#ifdef PARANOID
	memset(ret, 0, size);
#endif
	return ret;
}

void *xcalloc(size_t nmemb, size_t size) {
#ifdef PARANOID
	nmemb++; // Just in case
	size++;	 // Just in case
#endif

	void *ret = calloc(nmemb, size);

	if(ret == NULL)
		critical("calloc(%u, %u): Cannot allocate memory", nmemb, size);

	return ret;
}

void *xrealloc(void *oldptr, size_t size) {
#ifdef PARANOID
	size++;	// Just in case
#endif

	void *ret = realloc(oldptr, size);

	if(ret == NULL)
		critical("realloc(%p, %u): Cannot allocate memory", oldptr, size);

	return ret;
}

void *_dynamic_add  (dynamic_t *data, size_t membsize)
{
	void *ret;

	if (data->num_allocated <= data->num) {
		data->num_allocated += ALLOC_PORTION;
		data->dat = xrealloc(data->dat, data->num_allocated*membsize);
	}

	ret = &(((char *)data->dat)[ membsize * data->num++ ]);
	memset(ret, 0, membsize);
	return ret;
}

int   _dynamic_foreach(dynamic_t *data, dynamic_procfunct_t funct, void *arg, size_t membsize)
{
	int rc = 0;

	if (funct == NULL)
		return EINVAL;

	{
		int i;
		i=0;
		while (i < data->num) {
			if ((rc=funct( &(((char *)data->dat)[i*membsize]), arg )))
				break;
			i++;
		}
	}

	return rc;
}

void  _dynamic_reset(dynamic_t *data, freefunct_t freefunct)
{
	if (freefunct != NULL) {
		int i;
		i=0;
		while (i < data->num) {
			freefunct( ((char **)data->dat)[i] );
			i++;
		}
	}

	free(data->dat);
	memset(data, 0, sizeof(*data));
	return;
}

