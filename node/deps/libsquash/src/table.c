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
#include "squash/table.h"

#include "squash/fs.h"

#include <string.h>

sqfs_err sqfs_table_init(sqfs_table *table, sqfs_fd_t fd, sqfs_off_t start, size_t each,
		size_t count) {
	//size_t nblocks, bread;
	
	if (count == 0)
		return SQFS_OK;
	
	//nblocks = sqfs_divceil(each * count, SQUASHFS_METADATA_SIZE);

	//unused
	//bread = nblocks * sizeof(uint64_t);

	table->each = each;
	table->blocks = (uint64_t *)(fd + start);
	
	return SQFS_OK;
}

void sqfs_table_destroy(sqfs_table *table) {
	table->blocks = NULL;
}

sqfs_err sqfs_table_get(sqfs_table *table, sqfs *fs, size_t idx, void *buf) {
	sqfs_block *block;
	size_t pos = idx * table->each;
	size_t bnum = pos / SQUASHFS_METADATA_SIZE,
		off = pos % SQUASHFS_METADATA_SIZE;
	
	sqfs_off_t bpos = table->blocks[bnum];
	if (sqfs_md_cache(fs, &bpos, &block))
		return SQFS_ERR;
	
	memcpy(buf, (char*)(block->data) + off, table->each);
	/* BLOCK CACHED, DON'T DISPOSE */
	return SQFS_OK;
}
