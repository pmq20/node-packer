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

int squash_stat(sqfs *fs, const char *path, struct stat *buf)
{
	sqfs_err error;
	sqfs_inode node;
	bool found;

	error = sqfs_inode_get(fs, &node, sqfs_inode_root(fs));
	if (SQFS_OK != error)	{
		return -1;
	}

	error = sqfs_lookup_path(fs, &node, path, &found);
	if (SQFS_OK != error)	{
		return -1;
	}

	if (!found)	{
		errno = ENOENT;
		return -1;
	}

	if(S_ISLNK(node.base.mode)){

		char buf[SQUASHFS_PATH_LEN];//is enough for path?

		ssize_t linklength = squash_readlink(fs, path, buf, sizeof(buf));

		if(linklength > 0){
			if(buf[0] == '/'){//is Absolute Path
				//find node from /
				error = sqfs_inode_get(fs, &node, sqfs_inode_root(fs));
				if (SQFS_OK != error)	{
					return -1;
				}

				error = sqfs_lookup_path(fs, &node, buf, &found);
				if (SQFS_OK != error)	{
					return -1;
				}
			}
			else{//is Relative Path
				size_t pos = strlen(path) - 1;
				//find the last /  "/a/b/cb"
				while(path[pos--] != '/'){
					//pos--;
				}

				char newpath[SQUASHFS_PATH_LEN];

				memcpy(newpath, path, pos + 2);
				memcpy(newpath + pos + 2, buf, linklength);
				newpath[pos + 2 + linklength] = '\0';
				//find node from /
				error = sqfs_inode_get(fs, &node, sqfs_inode_root(fs));
				if (SQFS_OK != error)	{
					return -1;
				}

				error = sqfs_lookup_path(fs, &node, buf, &found);
				if (SQFS_OK != error)	{
					return -1;
				}
			}
		}
		else{
			return -1;
		}
	}

	error = sqfs_stat(fs, &node, buf);
	if (SQFS_OK != error)
	{
		return -1;
	}

	return 0;
}

int squash_lstat(sqfs *fs, const char *path, struct stat *buf)
{
	sqfs_err error;
	sqfs_inode node;
	bool found;

	error = sqfs_inode_get(fs, &node, sqfs_inode_root(fs));
	if (SQFS_OK != error)
	{
		return -1;
	}
	error = sqfs_lookup_path(fs, &node, path, &found);
	if (SQFS_OK != error)
	{
		return -1;
	}
	if (!found)
	{
		errno = ENOENT;
		return -1;
	}
	error = sqfs_stat(fs, &node, buf);
	if (SQFS_OK != error)
	{
		return -1;
	}

	return 0;
}

int squash_fstat(sqfs *fs, int vfd, struct stat *buf)
{
	if (!SQUASH_VALID_VFD(vfd))
	{
		errno = EBADF;
		return -1;
	}
	*buf = SQUASH_VFD_FILE(vfd)->st;
	return 0;
}
