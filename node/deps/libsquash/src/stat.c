/*
 * Copyright (c) 2017 Minqi Pan <pmq2001@gmail.com>
 *                    Shengyuan Liu <sounder.liu@gmail.com>
 *
 * This file is part of libsquash, distributed under the MIT License
 * For full terms see the included LICENSE file
 */

#include "squash.h"

int squash_stat(sqfs *fs, const char *path, struct stat *buf)
{
	sqfs_err error;
	sqfs_inode node;
	short found;
	
	error = sqfs_inode_get(fs, &node, sqfs_inode_root(fs));
	if (SQFS_OK != error) {
		return -1;
	}
	error = sqfs_lookup_path_inner(fs, &node, path, &found, 1);
	if (SQFS_OK != error) {
		return -1;
	}
	if (!found) {
		errno = ENOENT;
		return -1;
	}
	error = sqfs_stat(fs, &node, buf);
	if (SQFS_OK != error) {
		return -1;
	}

	return 0;
}

int squash_lstat(sqfs *fs, const char *path, struct stat *buf)
{
	sqfs_err error;
	sqfs_inode node;
	short found;

	error = sqfs_inode_get(fs, &node, sqfs_inode_root(fs));
	if (SQFS_OK != error) {
		return -1;
	}
	error = sqfs_lookup_path(fs, &node, path, &found);
	if (SQFS_OK != error) {
		return -1;
	}
	if (!found) {
		errno = ENOENT;
		return -1;
	}
	error = sqfs_stat(fs, &node, buf);
	if (SQFS_OK != error) {
		return -1;
	}

	return 0;
}

int squash_fstat(int vfd, struct stat *buf)
{
	if (!SQUASH_VALID_VFD(vfd))
	{
		errno = EBADF;
		return -1;
	}
	*buf = SQUASH_VFD_FILE(vfd)->st;
	return 0;
}
