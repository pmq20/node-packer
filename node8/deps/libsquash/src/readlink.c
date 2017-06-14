/*
 * Copyright (c) 2017 Minqi Pan <pmq2001@gmail.com>
 *                    Shengyuan Liu <sounder.liu@gmail.com>
 *
 * This file is part of libsquash, distributed under the MIT License
 * For full terms see the included LICENSE file
 */

#include "squash.h"

ssize_t squash_readlink_inode(sqfs *fs, sqfs_inode *node, char *buf, size_t bufsize) {
        sqfs_err error;
        size_t want = 0;
        sqfs_md_cursor cur;

        if (!S_ISLNK(node->base.mode)) {
                errno = EINVAL;
                return -1;
        }

        want = node->xtra.symlink_size;

        if (want > bufsize - 1) {
                errno = ENAMETOOLONG;
                return -1; //bufsize is too small
        }
        else if (want == 0) {
                errno = EBADF;
                return -1;
        }
        cur = node->next;
        error = sqfs_md_read(fs, &cur, buf, want);
        if (SQFS_OK != error) {
                errno = EIO;
                return -1;
        }
        buf[want] = '\0';
        if (fs->root_alias &&
            strlen(buf) >= strlen(fs->root_alias) &&
            buf == strstr(buf, fs->root_alias)) {
                char *buf_ptr = buf + strlen(fs->root_alias) - 1;
                assert('/' == buf_ptr[0]); // still is Absolute Path
                memmove(buf, buf_ptr, strlen(buf_ptr) + 1);
                want = strlen(buf);
        } else if (fs->root_alias2 &&
            strlen(buf) >= strlen(fs->root_alias2) &&
            buf == strstr(buf, fs->root_alias2)) {
                char *buf_ptr = buf + strlen(fs->root_alias2) - 1;
                assert('/' == buf_ptr[0]); // still is Absolute Path
                memmove(buf, buf_ptr, strlen(buf_ptr) + 1);
                want = strlen(buf);
        }
        return want;
}

ssize_t squash_readlink(sqfs *fs, const char *path, char *buf, size_t bufsize) {
    sqfs_err error;
    sqfs_inode node;
    short found;

    assert(buf && path && fs);
    memset(&node, 0, sizeof(sqfs_inode));

    found = 0;

    error = sqfs_inode_get(fs, &node, sqfs_inode_root(fs));
    if (SQFS_OK != error)
    	goto failure;
    error = sqfs_lookup_path(fs, &node, path, &found);
    if (SQFS_OK != error)
    	goto failure;

    if (found) {
        return squash_readlink_inode(fs, &node, buf, bufsize);
    } else {
        errno = ENOENT;
	goto failure;
    }

failure:
	if (!errno) {
		errno = ENOENT;
	}
	return -1;
}
