/*
* Copyright (c) 2017 Minqi Pan and Shengyuan Liu
*
* This file is part of libsquash, distributed under the MIT License
* For full terms see the included LICENSE file
*/

#include "squash.h"
#include <stdlib.h>

sqfs_err squash_start()
{
	squash_global_fdtable.nr = 0;
	squash_global_fdtable.fds = NULL;
	return SQFS_OK;
}

sqfs_err squash_halt()
{
	free(squash_global_fdtable.fds);
	return SQFS_OK;
}
