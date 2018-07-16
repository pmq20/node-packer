/*
 * Copyright (c) 2017 Minqi Pan <pmq2001@gmail.com>
 *                    Shengyuan Liu <sounder.liu@gmail.com>
 *
 * This file is part of libsquash, distributed under the MIT License
 * For full terms see the included LICENSE file
 */

#include "squash.h"
#include <stdlib.h>

#include <assert.h>

SQUASH_DIR *squash_opendir_inner(sqfs *fs, const char *filename, short follow_link)
{
	sqfs_err error;
	short found;
	SQUASH_DIR *dir = calloc(1, sizeof(SQUASH_DIR));
	int *handle;
	
	if (NULL == dir)
	{
		errno = ENOMEM;
		return NULL;
	}
	dir->fs = fs;
	dir->entries = NULL;
	dir->nr = 0;
	dir->filename = strdup(filename);
	dir->fd = squash_open(fs, filename);
	if (-1 == dir->fd)
	{
		goto failure;
	}
	handle = (int *)(squash_global_fdtable.fds[dir->fd]->payload);

	MUTEX_LOCK(&squash_global_mutex);
	free(handle);
	squash_global_fdtable.fds[dir->fd]->payload = (void *)dir;
	MUTEX_UNLOCK(&squash_global_mutex);

	dir->actual_nr = 0;
	dir->loc = 0;
	error = sqfs_inode_get(fs, &dir->node, sqfs_inode_root(fs));
	if (SQFS_OK != error)
	{
		goto failure;
	}
	error = sqfs_lookup_path_inner(fs, &dir->node, filename, &found, follow_link);
	if (SQFS_OK != error)
	{
		goto failure;
	}
	if (!found)
	{
		errno = ENOENT;
		goto failure;
	}
	error = sqfs_dir_open(fs, &dir->node, &dir->dir, 0);
	if (SQFS_OK != error)
	{
		goto failure;
	}
	return dir;
failure:
	if (!errno) {
		errno = ENOENT;
	}
	free(dir);
	return NULL;
}

SQUASH_DIR *squash_opendir(sqfs *fs, const char *filename)
{
        return squash_opendir_inner(fs, filename, 1);
}

int squash_closedir(SQUASH_DIR *dirp)
{
	int ret;

	assert(-1 != dirp->fd);
	free(dirp->entries);
	free(dirp->filename);
	if (dirp->payload) {
		free(dirp->payload);
	}
	// dirp itself will be freed by squash_close as `payload`
	ret = squash_close(dirp->fd);
	if (0 != ret) {
		if (!errno) {
			errno = EBADF;
		}
		return -1;
	}
	return 0;
}

struct SQUASH_DIRENT * squash_readdir(SQUASH_DIR *dirp)
{
	sqfs_err error;
	size_t nr = dirp->loc + 1;
	if (dirp->nr < nr) {
		// we secretly extend the requested size
		// in order to minimize the number of realloc calls
		nr *= 10;
		dirp->entries = realloc(dirp->entries, nr * sizeof(*dirp->entries));
		if (NULL == dirp->entries)
		{
			errno = ENOMEM;
			return NULL;
		}
		dirp->nr = nr;
	}

	while (dirp->actual_nr < dirp->loc + 1)
	{
		sqfs_dentry_init(&dirp->entries[dirp->actual_nr].entry,
			dirp->entries[dirp->actual_nr].name);
		dirp->entries[dirp->actual_nr].not_eof = sqfs_dir_next(
			dirp->fs,
			&dirp->dir,
			&dirp->entries[dirp->actual_nr].entry,
			&error);
		if (SQFS_OK != error)
		{
			return NULL;
		}
		else if (!dirp->entries[dirp->actual_nr].not_eof)
		{
			return NULL;
		}
		else {
			sqfs_dir_entry *entry = &dirp->entries[dirp->actual_nr].entry;
			struct SQUASH_DIRENT *sysentry = &dirp->entries[dirp->actual_nr].sysentry;
			size_t minsize;
			
			sysentry->d_ino = entry->inode_number;
			minsize = entry->name_size;
			if (SQUASHFS_NAME_LEN < minsize) {
				minsize = SQUASHFS_NAME_LEN;
			}
			if (sizeof(sysentry->d_name) < minsize) {
				minsize = sizeof(sysentry->d_name);
			}
			memcpy(sysentry->d_name, dirp->entries[dirp->actual_nr].name, minsize);
#ifndef __linux__
			sysentry->d_namlen = minsize;
#endif
			sysentry->d_name[minsize] = '\0';
			// TODO special treatment of L types
			switch (entry->type)
			{	
			case SQUASHFS_DIR_TYPE:
			case SQUASHFS_LDIR_TYPE:
				sysentry->d_type = DT_DIR;
				break;
			case SQUASHFS_REG_TYPE:
			case SQUASHFS_LREG_TYPE:
				sysentry->d_type = DT_REG;
				break;
			case SQUASHFS_SYMLINK_TYPE:
			case SQUASHFS_LSYMLINK_TYPE:
				sysentry->d_type = DT_LNK;
				break;
			case SQUASHFS_BLKDEV_TYPE:
			case SQUASHFS_LBLKDEV_TYPE:
				sysentry->d_type = DT_BLK;
				break;
			case SQUASHFS_CHRDEV_TYPE:
			case SQUASHFS_LCHRDEV_TYPE:
				sysentry->d_type = DT_CHR;
				break;
			case SQUASHFS_FIFO_TYPE:
			case SQUASHFS_LFIFO_TYPE:
				sysentry->d_type = DT_FIFO;
				break;
			case SQUASHFS_SOCKET_TYPE:
			case SQUASHFS_LSOCKET_TYPE:
				sysentry->d_type = DT_SOCK;
				break;
			default:
				sysentry->d_type = DT_UNKNOWN;
				break;
			}
			dirp->actual_nr += 1;
		}
	}
	if (!dirp->entries[dirp->loc].not_eof)
	{
		return NULL;
	}
	dirp->loc += 1;
	return &dirp->entries[dirp->loc - 1].sysentry;
}

long squash_telldir(SQUASH_DIR *dirp)
{
	return dirp->loc;
}

void squash_seekdir(SQUASH_DIR *dirp, long loc)
{
	dirp->loc = loc;
}

void squash_rewinddir(SQUASH_DIR *dirp)
{
	dirp->loc = 0;
}

int squash_dirfd(SQUASH_DIR *dirp)
{
	return dirp->fd;
}
