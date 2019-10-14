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
#ifndef SQFS_FILE_H
#define SQFS_FILE_H

#include "squash/common.h"

#include "squash/squashfs_fs.h"

#include "squash/cache.h"

sqfs_err sqfs_frag_entry(sqfs *fs, struct squashfs_fragment_entry *frag,
	uint32_t idx);

sqfs_err sqfs_frag_block(sqfs *fs, sqfs_inode *inode,
	size_t *offset, size_t *size, sqfs_block **block);

typedef uint32_t sqfs_blocklist_entry;
typedef struct {
	sqfs *fs;
	size_t remain;			/* How many blocks left in the file? */
	sqfs_md_cursor cur;	/* Points to next blocksize in MD */
	short started;

	uint64_t pos;
	
	uint64_t block;			/* Points to next data block location */
	sqfs_blocklist_entry header; /* Packed blocksize data */
	uint32_t input_size;				 /* Extracted size of this block */
} sqfs_blocklist;

size_t sqfs_blocklist_count(sqfs *fs, sqfs_inode *inode);

void sqfs_blocklist_init(sqfs *fs, sqfs_inode *inode, sqfs_blocklist *bl);
sqfs_err sqfs_blocklist_next(sqfs_blocklist *bl);


sqfs_err sqfs_read_range(sqfs *fs, sqfs_inode *inode, sqfs_off_t start,
	sqfs_off_t *size, void *buf);


/*** Block index for skipping to the middle of large files ***/

typedef struct {
	uint64_t data_block;	/* A data block where the file continues */
	uint32_t md_block;		/* A metadata block with blocksizes that continue from
													 data_block */
} sqfs_blockidx_entry;

sqfs_err sqfs_blockidx_init(sqfs_cache *cache);

/* Get a blocklist fast-forwarded to the correct location */
sqfs_err sqfs_blockidx_blocklist(sqfs *fs, sqfs_inode *inode,
	sqfs_blocklist *bl, sqfs_off_t start);

#endif
