/*
 * Copyright (c) 2016-2017 Minqi Pan <pmq2001@gmail.com>
 *                         Shengyuan Liu <sounder.liu@gmail.com>
 *
 * This file is part of libsquash, distributed under the MIT License
 * For full terms see the included LICENSE file
 */

#include "squash.h"
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <assert.h>

sqfs_err squash_dir_realloc(SQUASH_DIR *dir, size_t nr)
{
	if (dir->nr >= nr)
	{
		return SQFS_OK;
	}
	nr *= 10; // we internally extend the requested size to minimize the number of realloc calls
	dir->entries = realloc(dir->entries, nr * sizeof(*dir->entries));
	if (NULL == dir->entries)
	{
		return SQFS_NOMEM;
	}
	dir->nr = nr;
	return SQFS_OK;
}

SQUASH_DIR *squash_opendir(sqfs_err *error, sqfs *fs, const char *filename)
{
	*error = SQFS_OK;
	bool found;
	SQUASH_DIR *dir = malloc(sizeof(SQUASH_DIR));
	if (NULL == dir)
	{
		*error = SQFS_NOMEM;
		return NULL;
	}
	memcpy(dir->magic, SQUASH_DIR_MAGIC, SQUASH_DIR_MAGIC_LEN);
	dir->magic[SQUASH_DIR_MAGIC_LEN] = '\0';
	dir->fs = fs;
	dir->entries = NULL;
	dir->nr = 0;
	dir->fd = squash_open(error, fs, filename);
	dir->actual_nr = dir->loc = 0;
	if (-1 == dir->fd)
	{
		goto failure;
	}
	*error = sqfs_inode_get(fs, &dir->node, sqfs_inode_root(fs));
	if (SQFS_OK != *error)
	{
		goto failure;
	}
	*error = sqfs_lookup_path(fs, &dir->node, filename, &found);
	if (SQFS_OK != *error)
	{
		goto failure;
	}
	if (!found)
	{
		*error = SQFS_NOENT;
		goto failure;
	}
	*error = sqfs_dir_open(fs, &dir->node, &dir->dir, 0);
	if (SQFS_OK != *error)
	{
		goto failure;
	}
	return dir;
failure:
	free(dir);
	return NULL;
}

int squash_closedir(sqfs_err *error, SQUASH_DIR *dirp)
{
	*error = SQFS_OK;
	assert(-1 != dirp->fd);
	int ret = squash_close(error, dirp->fd);
	if (0 != ret)
	{
		return -1;
	}
	free(dirp->entries);
	free(dirp);
	return 0;
}

struct dirent * squash_readdir(sqfs_err *error, SQUASH_DIR *dirp)
{
	*error = SQFS_OK;
	squash_dir_realloc(dirp, dirp->loc + 1);
	while (dirp->actual_nr < dirp->loc + 1)
	{
		sqfs_dentry_init(&dirp->entries[dirp->actual_nr].entry,
			dirp->entries[dirp->actual_nr].name);
		dirp->entries[dirp->actual_nr].not_eof = sqfs_dir_next(
			dirp->fs,
			&dirp->dir,
			&dirp->entries[dirp->actual_nr].entry,
			error);
		if (SQFS_OK != *error)
		{
			return NULL;
		}
		else if (!dirp->entries[dirp->actual_nr].not_eof)
		{
			return NULL;
		}
		else {
			sqfs_dir_entry *entry = &dirp->entries[dirp->actual_nr].entry;
			struct dirent *sysentry = &dirp->entries[dirp->actual_nr].sysentry;
			sysentry->d_ino = entry->inode_number;
			size_t minsize = entry->name_size;
			if (SQUASHFS_NAME_LEN < minsize) {
				minsize = SQUASHFS_NAME_LEN;
			}
#ifdef _WIN32
			sysentry->d_name = dirp->entries[dirp->actual_nr].name;
#else
			if (sizeof(sysentry->d_name) < minsize) {
				minsize = sizeof(sysentry->d_name);
			}
			memcpy(sysentry->d_name, dirp->entries[dirp->actual_nr].name, minsize);
#endif
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

int squash_dirfd(sqfs_err *error, SQUASH_DIR *dirp)
{
	*error = SQFS_OK;
	if (-1 == dirp->fd)
	{
		*error = SQFS_ERR;
		return -1;
	}
	return dirp->fd;
}
