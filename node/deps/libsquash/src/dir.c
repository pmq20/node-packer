/*
 * Copyright (c) 2012 Dave Vasilevsky <dave@vasilevsky.ca>
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR(S) ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL THE AUTHOR(S) BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 * NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */
#include "squash.h"

#include "squash/dir.h"

#include "squash/fs.h"

#include <string.h>
#include <stdlib.h>

/* Read some directory metadata, updating the dir structure as necessary */
static sqfs_err sqfs_dir_md_read(sqfs *fs, sqfs_dir *dir, void *buf,
		size_t size);

/* Fast forwards to a directory header. */
typedef sqfs_err sqfs_dir_header_f(sqfs *fs, sqfs_md_cursor *cur,
	struct squashfs_dir_index *index, short *stop, void *arg);
static sqfs_err sqfs_dir_ff_header(sqfs *fs, sqfs_inode *inode, sqfs_dir *dir,
	sqfs_dir_header_f func, void *arg);

/* Fast forward a directory to the given offset. Return error if it doesn't
	 exist. */
static sqfs_err sqfs_dir_ff_offset(sqfs *fs, sqfs_inode *inode, sqfs_dir *dir,
	sqfs_off_t offset);


static sqfs_err sqfs_dir_md_read(sqfs *fs, sqfs_dir *dir, void *buf,
		size_t size) {
	dir->offset += size;
	return sqfs_md_read(fs, &dir->cur, buf, size);
}


sqfs_err sqfs_dir_open(sqfs *fs, sqfs_inode *inode, sqfs_dir *dir,
		off_t offset) {
	if (!S_ISDIR(inode->base.mode))
		return SQFS_ERR;

	memset(dir, 0, sizeof(*dir));
	dir->cur.block = inode->xtra.dir.start_block +
		fs->sb->directory_table_start;
	dir->cur.offset = inode->xtra.dir.offset;
	dir->offset = 0;
	dir->total = inode->xtra.dir.dir_size - 3;
	
	if (offset) {
		/* Fast forward to the given offset */
		sqfs_err err = sqfs_dir_ff_offset(fs, inode, dir, offset);
		if (err)
			return err;
	}
	
	return SQFS_OK;
}


void sqfs_dentry_init(sqfs_dir_entry *entry, char *namebuf) {
	entry->name = namebuf;
}

sqfs_off_t sqfs_dentry_offset(sqfs_dir_entry *entry) {
	return entry->offset;
}

sqfs_off_t sqfs_dentry_next_offset(sqfs_dir_entry *entry) {
	return entry->next_offset;
}

int sqfs_dentry_type(sqfs_dir_entry *entry) {
	return entry->type;
}
sqfs_mode_t	sqfs_dentry_mode(sqfs_dir_entry *entry) {
	return sqfs_mode(sqfs_dentry_type(entry));
}

sqfs_inode_id sqfs_dentry_inode(sqfs_dir_entry *entry) {
	return entry->inode;
}

sqfs_inode_num sqfs_dentry_inode_num(sqfs_dir_entry *entry) {
	return entry->inode_number;
}

size_t sqfs_dentry_name_size(sqfs_dir_entry *entry) {
	return entry->name_size;
}

const char *sqfs_dentry_name(sqfs_dir_entry *entry) {
	if (!entry->name)
		return NULL;
	
	entry->name[sqfs_dentry_name_size(entry)] = '\0';
	return entry->name;
}

short sqfs_dentry_is_dir(sqfs_dir_entry *entry) {
	return S_ISDIR(sqfs_dentry_mode(entry));
}



short sqfs_dir_next(sqfs *fs, sqfs_dir *dir, sqfs_dir_entry *entry,
		sqfs_err *err) {
	struct squashfs_dir_entry e;
	
	*err = SQFS_OK;
	entry->offset = dir->offset;
	
	while (dir->header.count == 0) {
		if (dir->offset >= dir->total)
			return 0;
		
		if ((*err = sqfs_dir_md_read(fs, dir, &dir->header, sizeof(dir->header))))
			return 0;
		++(dir->header.count); /* biased by one */
	}
	
	if ((*err = sqfs_dir_md_read(fs, dir, &e, sizeof(e))))
		return 0;
	--(dir->header.count);
	
	entry->type = e.type;
	entry->name_size = e.size + 1;
	entry->inode = ((uint64_t)dir->header.start_block << 16) + e.offset;
	/* e.inode_number is signed */
	entry->inode_number = dir->header.inode_number + (int16_t)e.inode_number;
	
	*err = sqfs_dir_md_read(fs, dir, entry->name, sqfs_dentry_name_size(entry));
	if (*err)
		return 0;
	
	entry->next_offset = dir->offset;

	return 1;
}


static sqfs_err sqfs_dir_ff_header(sqfs *fs, sqfs_inode *inode,
		sqfs_dir *dir, sqfs_dir_header_f func, void *arg) {
	struct squashfs_dir_index idx;
	sqfs_md_cursor cur = inode->next;
	size_t count = inode->xtra.dir.idx_count;

	if (count == 0)
		return SQFS_OK;
	
	while (count--) {
		sqfs_err err;
		short stop = 0;
		
		if ((err = sqfs_md_read(fs, &cur, &idx, sizeof(idx))))
			return err;
		
		if ((err = func(fs, &cur, &idx, &stop, arg)))
			return err;
		if (stop)
			break;
		
		dir->cur.block = idx.start_block + fs->sb->directory_table_start;
		dir->offset = idx.index;
	}

	dir->cur.offset = (dir->cur.offset + dir->offset) % SQUASHFS_METADATA_SIZE;
	return SQFS_OK;
}


/* Helper for sqfs_dir_ff_offset */
static sqfs_err sqfs_dir_ff_offset_f(sqfs *fs, sqfs_md_cursor *cur,
		struct squashfs_dir_index *index, short *stop, void *arg) {
	sqfs_off_t offset = *(sqfs_off_t*)arg;
	
	if (index->index >= offset) {
		*stop = 1;
		return SQFS_OK;
	}
	
	return sqfs_md_read(fs, cur, NULL, index->size + 1); /* skip name */
}

static sqfs_err sqfs_dir_ff_offset(sqfs *fs, sqfs_inode *inode, sqfs_dir *dir,
		sqfs_off_t offset) {
	sqfs_err err;
	sqfs_dir_entry entry;
	
	err = sqfs_dir_ff_header(fs, inode, dir, sqfs_dir_ff_offset_f, &offset);
	if (err)
		return err;
	
	sqfs_dentry_init(&entry, NULL);
	while (dir->offset < offset && sqfs_dir_next(fs, dir, &entry, &err))
		; /* pass */
	
	if (err)
		return err;
	return dir->offset == offset ? SQFS_OK : SQFS_ERR;
}


/* Helper for sqfs_dir_lookup */
typedef struct {
	const char *cmp;
	size_t cmplen;
	char *name;
} sqfs_dir_ff_name_t;

static sqfs_err sqfs_dir_ff_name_f(sqfs *fs, sqfs_md_cursor *cur,
		struct squashfs_dir_index *index, short *stop, void *arg) {
	sqfs_err err;
	sqfs_dir_ff_name_t *args = (sqfs_dir_ff_name_t*)arg;
	size_t name_size = index->size + 1;
	int order;

	if ((err = sqfs_md_read(fs, cur, args->name, name_size)))
		return err;
	args->name[name_size] = '\0';
	
	order = strncmp(args->name, args->cmp, args->cmplen);
	if (order > 0 || (order == 0 && name_size > args->cmplen))
		*stop = 1;
	
	return SQFS_OK;
}

sqfs_err sqfs_dir_lookup(sqfs *fs, sqfs_inode *inode,
		const char *name, size_t namelen, sqfs_dir_entry *entry, short *found) {
	sqfs_err err;
	sqfs_dir dir;
	sqfs_dir_ff_name_t arg;

	*found = 0;

	if ((err = sqfs_dir_open(fs, inode, &dir, 0)))
		return err;
	
	/* Fast forward to header */
	arg.cmp = name;
	arg.cmplen = namelen;
	arg.name = entry->name;
	if ((err = sqfs_dir_ff_header(fs, inode, &dir, sqfs_dir_ff_name_f, &arg)))
		return err;
	
	/* Iterate to find the right entry */
	while (sqfs_dir_next(fs, &dir, entry, &err)) {
		int order = strncmp(sqfs_dentry_name(entry), name, namelen);
		if (order == 0 && sqfs_dentry_name_size(entry) == namelen)
			*found = 1;
		if (order >= 0)
			break;
	}
	
	return err;
}

static sqfs_err clear_dot_dot(char *path)
{
	char *dot_dot, *dot_dot_left;
	
	// we might get a path like /a/b/../../c/d
	// in which case we need to interpret dot-dot's
	while ((dot_dot = strstr(path, "/../"))) {
		dot_dot_left = dot_dot - 1;
		do {
			if (dot_dot_left < path) {
				return SQFS_ERR;
			}
			if ('/' == *dot_dot_left) {
				break;
			}
			--dot_dot_left;
		} while (1);
		memmove(dot_dot_left, dot_dot + 3, strlen(dot_dot + 3) + 1);
	}
	// what about /a/b/c/d/..
	if (strlen(path) >= 3 && 0 == strcmp(path + strlen(path) - 3, "/..")) {
		dot_dot_left = path + strlen(path) - 4;
		do {
			if (dot_dot_left < path) {
				return SQFS_ERR;
			}
			if ('/' == *dot_dot_left) {
				break;
			}
			--dot_dot_left;
		} while (1);
		*dot_dot_left = '\0';
	}
	return SQFS_OK;
}

sqfs_err squash_follow_link(sqfs *fs, const char *path, sqfs_inode *node) {

	sqfs_err error;
	short found;
	char base_path[SQUASHFS_PATH_LEN];
	char new_path[SQUASHFS_PATH_LEN];
	int inode_num;

	if(strlen(path) > SQUASHFS_PATH_LEN){
		errno = ENAMETOOLONG;
		return SQFS_ERR;
	}

	strncpy(base_path, path, SQUASHFS_PATH_LEN);
	inode_num = 0;
	do {
		char buf_link[SQUASHFS_PATH_LEN]; // is enough for path?
		ssize_t link_length = squash_readlink_inode(fs, node, buf_link, sizeof(buf_link));
		if (link_length > 0) {
			if (buf_link[0] == '/') { // is Absolute Path
				// find node from /
				error = sqfs_inode_get(fs, node, sqfs_inode_root(fs));
				if (SQFS_OK != error) {
					return error;
				}
				error = sqfs_lookup_path(fs, node, buf_link, &found);
				strncpy(new_path, buf_link, SQUASHFS_PATH_LEN);
				if (SQFS_OK != error) {
					return error;
				} else if (!found) {
					errno = ENOENT;
					return SQFS_ERR;
				}
			} else { // is Relative Path
				size_t pos = strlen(base_path) - 1;
				// find the last /  "/a/b/cb"
				while (base_path[pos--] != '/') { }

				memcpy(new_path, base_path, pos + 2);
				memcpy(new_path + pos + 2, buf_link, link_length);
				new_path[pos + 2 + link_length] = '\0';
				
				error = clear_dot_dot(new_path);
				if (SQFS_OK != error) {
					errno = ENOENT;
					return SQFS_ERR;
				}
				
				// find node from /
				error = sqfs_inode_get(fs, node, sqfs_inode_root(fs));
				if (SQFS_OK != error) {
					return error;
				}
				error = sqfs_lookup_path(fs, node, new_path, &found);
				if (SQFS_OK != error) {
					return error;
				} else if (!found) {
					errno = ENOENT;
					return SQFS_ERR;
				}
			}

			inode_num++;
			if (inode_num > SQUASHFS_MAX_LINK_LEVEL) {
				errno = ELOOP;
				return SQFS_ERR;
			}
			strncpy(base_path, new_path, SQUASHFS_PATH_LEN);

		} else {
			return SQFS_ERR;
		}


	} while (S_ISLNK(node->base.mode));

	return SQFS_OK;
}

sqfs_err sqfs_lookup_path_inner(sqfs *fs, sqfs_inode *inode, const char *incoming_path,
		short *found, short follow_link) {
	sqfs_err err, ret;
	sqfs_name buf;
	sqfs_path path_here;
	sqfs_dir_entry entry;
	char *path, *path0;
	short is_last_component;
	memset(&buf, 0, sizeof(sqfs_name));
	memset(&entry, 0, sizeof(sqfs_dir_entry));

	*found = 0;
	sqfs_dentry_init(&entry, buf);

	path = strdup(incoming_path);
	if (NULL == path) {
		return SQFS_ERR;
	}
	path0 = path;

	err = clear_dot_dot(path);
	if (SQFS_OK != err) {
		ret = SQFS_OK; /* not found */
		goto exit;
	}
	
	while (*path) {
		const char *name;
		size_t size;
		
		/* Find next path component */
		while (*path == '/') /* skip leading slashes */
			++path;
		
		name = path;
		while (*path && *path != '/')
			++path;
		size = path - name;
		if (0 == size) {
			/* we're done */
			break;
		}
		if (1 == size && '.' == *name) {
			/* interpret dot */
			continue;
		}
		if ((err = sqfs_dir_lookup(fs, inode, name, size, &entry, found))) {
			ret = err;
			goto exit;
		}
		if (!(*found)) {
			ret = SQFS_OK; /* not found */
			goto exit;
		}
		if ((err = sqfs_inode_get(fs, inode, sqfs_dentry_inode(&entry)))) {
			ret = err;
			goto exit;
		}
		if (!*path) {
			is_last_component = 1;
		} else if (path0 + strlen(path0) - 1 == path) {
			assert('/' == *path);
			is_last_component = 1;
		} else {
			is_last_component = 0;
		}

		if (S_ISLNK(inode->base.mode)) {
			if (!(is_last_component && !follow_link)) {
				size_t size_here = path - path0;
				if (size_here > SQUASHFS_PATH_LEN) {
					size_here = SQUASHFS_PATH_LEN;
				}
				memcpy(path_here, path0, size_here);
				path_here[size_here] = '\0';
				err = squash_follow_link(fs, path_here, inode);
				if (SQFS_OK != err) {
					ret = err;
					goto exit;
				}
			}
		}
	}
	
	*found = 1;
	ret = SQFS_OK;
exit:
	free(path0);
	return ret;
}

sqfs_err sqfs_lookup_path(sqfs *fs, sqfs_inode *inode, const char *path,
		short *found) {
	return sqfs_lookup_path_inner(fs, inode, path, found, 0);
}
