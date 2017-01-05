/*
 * Copyright (c) 2016-2017 Minqi Pan <pmq2001@gmail.com>
 *                         Shengyuan Liu <sounder.liu@gmail.com>
 *
 * This file is part of libsquash, distributed under the MIT License
 * For full terms see the included LICENSE file
 */

#include "squash.h"


/**
 *
 */
ssize_t squash_readlink(sqfs *fs, const char *path, char *buf, size_t bufsize)
{
	sqfs_err error;
	assert(buf && path && fs);

	sqfs_inode node;
	memset(&node, 0, sizeof(sqfs_inode));

	bool found = false;

	error = sqfs_inode_get(fs, &node, sqfs_inode_root(fs));
	if(SQFS_OK != error)
		return -1;
	error = sqfs_lookup_path(fs, &node, path, &found);
	if(SQFS_OK != error)
		return -1;

	if(found){
		size_t want = 0;
		sqfs_md_cursor cur;

		if (!S_ISLNK(node.base.mode))
			return SQFS_ERR;

		want = node.xtra.symlink_size;

		if (want > bufsize - 1) {
			errno = ENAMETOOLONG;
			return -1; //bufsize is too small
		}
		cur = node.next;
		error = sqfs_md_read(fs, &cur, buf, want);
		if(SQFS_OK != error)
			return -1;
		buf[want] = '\0';
		return want;
	}
	else{
		errno = ENOENT;
		return -1;
	}
}
