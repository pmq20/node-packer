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
#include <sys/stat.h>

int squash_stat(sqfs_err *error, sqfs *fs, const char *path, struct stat *buf)
{
	*error = SQFS_OK;
	sqfs_inode node;
	bool found;


	*error = sqfs_inode_get(fs, &node, sqfs_inode_root(fs));
	if (SQFS_OK != *error)	{
		return -1;
	}

	*error = sqfs_lookup_path(fs, &node, path, &found);
	if (SQFS_OK != *error)	{
		return -1;
	}

	if (!found)	{
		*error = SQFS_NOENT;
		return -1;
	}

	if(S_ISLNK(node.base.mode)){

		char buf[SQUASHFS_PATH_LEN];//is enough for path?

		ssize_t linklength = squash_readlink(error, fs, path, buf, sizeof(buf));

		if(linklength > 0 && *error == SQFS_OK){
			if(buf[0] == '/'){//is Absolute Path
				//find node from /
				*error = sqfs_inode_get(fs, &node, sqfs_inode_root(fs));
				if (SQFS_OK != *error)	{
					return -1;
				}

				*error = sqfs_lookup_path(fs, &node, buf, &found);
				if (SQFS_OK != *error)	{
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
				*error = sqfs_inode_get(fs, &node, sqfs_inode_root(fs));
				if (SQFS_OK != *error)	{
					return -1;
				}

				*error = sqfs_lookup_path(fs, &node, buf, &found);
				if (SQFS_OK != *error)	{
					return -1;
				}
			}
		}
		else{
			return -1;
		}
	}

	*error = sqfs_stat(fs, &node, buf);
	if (SQFS_OK != *error)
	{
		return -1;
	}


	return 0;
}

int squash_lstat(sqfs_err *error, sqfs *fs, const char *path, struct stat *buf)
{
	*error = SQFS_OK;
	sqfs_inode node;
	bool found;

	*error = sqfs_inode_get(fs, &node, sqfs_inode_root(fs));
	if (SQFS_OK != *error)
	{
		return -1;
	}
	*error = sqfs_lookup_path(fs, &node, path, &found);
	if (SQFS_OK != *error)
	{
		return -1;
	}
	if (!found)
	{
		*error = SQFS_NOENT;
		return -1;
	}
	*error = sqfs_stat(fs, &node, buf);
	if (SQFS_OK != *error)
	{
		return -1;
	}

	return 0;
}

int squash_fstat(sqfs_err *error, sqfs *fs, int vfd, struct stat *buf)
{
	*error = SQFS_OK;
	if (!SQUASH_VALID_VFD(vfd))
	{
		*error = SQFS_INVALFD;
		return -1;
	}
	*buf = SQUASH_VFD_FILE(vfd)->st;
	return 0;
}
