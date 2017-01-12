/*
 * Copyright (c) 2017 Minqi Pan <pmq2001@gmail.com>
 *                    Shengyuan Liu <sounder.liu@gmail.com>
 *
 * This file is part of libsquash, distributed under the MIT License
 * For full terms see the included LICENSE file
 */

#include "squash.h"
#include <stdlib.h>


struct squash_fdtable squash_global_fdtable;

int squash_open(sqfs *fs, const char *path)
{
	sqfs_err error;
	struct squash_file *file = malloc(sizeof(struct squash_file));
	bool found;
	int fd;

	// try locating the file and fetching its stat
	if (NULL == file)
	{
		errno = ENOMEM;
		return -1;
	}
	error = sqfs_inode_get(fs, &file->node, sqfs_inode_root(fs));
	if (SQFS_OK != error)
	{
		goto failure;
	}
	error = sqfs_lookup_path_inner(fs, &file->node, path, &found, true);
	if (SQFS_OK != error)
	{
		goto failure;
	}
	if (!found)
	{
		errno = ENOENT;
		goto failure;
	}
	error = sqfs_stat(fs, &file->node, &file->st);
	if (SQFS_OK != error)
	{
		goto failure;
	}
	file->fs = fs;
	file->pos = 0;

	// get a dummy fd from the system
	fd = dup(0);
	// make sure that our global fd table is large enough
	size_t nr = fd + 1;
	if (squash_global_fdtable.nr < nr)
	{
		// we secretly extend the requested size
		// in order to minimize the number of realloc calls
		nr *= 10;
		squash_global_fdtable.fds = realloc(squash_global_fdtable.fds,
						nr * sizeof(struct squash_file *));
		if (NULL == squash_global_fdtable.fds)
		{
			errno = ENOMEM;
			goto failure;
		}
		memset(squash_global_fdtable.fds + squash_global_fdtable.nr,
			0,
			(nr - squash_global_fdtable.nr) * sizeof(struct squash_file *));
		squash_global_fdtable.nr = nr;
	}

	// insert the fd into the global fd table
	squash_global_fdtable.fds[fd] = file;
	return fd;

failure:
	free(file);
	return -1;
}

int squash_close(int vfd)
{
	if (!SQUASH_VALID_VFD(vfd))
	{
		errno = EBADF;
		return -1;
	}
	close(vfd);
	free(squash_global_fdtable.fds[vfd]);
	squash_global_fdtable.fds[vfd] = NULL;
	return 0;
}

ssize_t squash_read(int vfd, void *buf, sqfs_off_t nbyte)
{
	sqfs_err error;
	if (!SQUASH_VALID_VFD(vfd))
	{
		errno = EBADF;
		return -1;
	}
	struct squash_file *file = squash_global_fdtable.fds[vfd];

	error = sqfs_read_range(file->fs, &file->node, file->pos, &nbyte, buf);
	if (SQFS_OK != error)
	{
		return -1;
	}
	file->pos += nbyte;
	return nbyte;
}

off_t squash_lseek(int vfd, off_t offset, int whence)
{
	if (!SQUASH_VALID_VFD(vfd))
	{
		errno = EBADF;
		return -1;
	}
	struct squash_file *file = squash_global_fdtable.fds[vfd];
	if (SQUASH_SEEK_SET == whence)
	{
		file->pos = offset;
	}
	else if (SQUASH_SEEK_CUR == whence)
	{
		file->pos += offset;
	}
	else if (SQUASH_SEEK_END == whence)
	{
		assert(S_ISREG(file->node.base.mode));
		file->pos = file->node.xtra.reg.file_size;
	}
	return file->pos;
}
