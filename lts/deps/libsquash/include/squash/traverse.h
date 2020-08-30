/*
 * Copyright (c) 2014 Dave Vasilevsky <dave@vasilevsky.ca>
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
#ifndef SQFS_TRAVERSE_H
#define SQFS_TRAVERSE_H

#include "squash/common.h"

#include "squash/dir.h"
#include "squash/stack.h"

typedef struct {
	short dir_end;
	sqfs_dir_entry entry;
	char *path;
	
	
	/* private */
	int state;	
	sqfs *fs;
	sqfs_name namebuf;
	sqfs_stack stack;
	
	size_t path_size, path_cap;
	size_t path_last_size;
} sqfs_traverse;

/* Begin a recursive traversal of a filesystem tree.
   Every sub-item of the given inode will be traversed in-order, but not
   this inode itself. */
sqfs_err sqfs_traverse_open(sqfs_traverse *trv, sqfs *fs, sqfs_inode_id iid);
sqfs_err sqfs_traverse_open_inode(sqfs_traverse *trv, sqfs *fs,
	sqfs_inode *inode);

/* Clean up at any point during or after a traversal */
void sqfs_traverse_close(sqfs_traverse *trv);

/* Get the next item in the traversal. An item may be:
   - A directory entry, in which case trv->entry will be filled
	 - A marker that a directory is finished, in which case trv->dir_end will
     be 1.
   Returns 0 if there are no more items. */
short sqfs_traverse_next(sqfs_traverse *trv, sqfs_err *err);

/* Don't recurse into the directory just returned. */
sqfs_err sqfs_traverse_prune(sqfs_traverse *trv);

#endif
