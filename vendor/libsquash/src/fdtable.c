/*
 * Copyright (c) 2016-2017 Minqi Pan and Shengyuan Liu
 *
 * This file is part of libsquash, distributed under the MIT License
 * For full terms see the included LICENSE file
 */

#include "squash.h"
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>

struct squash_fdtable squash_global_fdtable;

sqfs_err squash_fdtable_realloc(size_t nr)
{
	if (squash_global_fdtable.nr >= nr)
	{
		return SQFS_OK;
	}
	nr *= 10; // we internally extend the requested size to minimize the number of realloc calls
	squash_global_fdtable.fds = realloc(squash_global_fdtable.fds, nr * sizeof(struct squash_file *));
	if (NULL == squash_global_fdtable.fds)
	{
		return SQFS_NOMEM;
	}
	memset(squash_global_fdtable.fds + squash_global_fdtable.nr,
			0,
			(nr - squash_global_fdtable.nr) * sizeof(struct squash_file *));
	squash_global_fdtable.nr = nr;
	return SQFS_OK;
}
