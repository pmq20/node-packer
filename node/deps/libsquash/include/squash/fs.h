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
#ifndef SQFS_FS_H
#define SQFS_FS_H

#include "squash/common.h"

#include "squash/squashfs_fs.h"

#include "squash/cache.h"
#include "squash/decompress.h"
#include "squash/table.h"

struct sqfs {
	sqfs_fd_t fd;
	size_t offset;
	struct squashfs_super_block *sb;
	sqfs_table id_table;
	sqfs_table frag_table;
	sqfs_table export_table;
	sqfs_cache md_cache;
	sqfs_cache data_cache;
	sqfs_cache frag_cache;
	sqfs_cache blockidx;
	sqfs_decompressor decompressor;
        const char *root_alias;
        const char *root_alias2;
};

struct sqfs_inode {
	struct squashfs_base_inode base;
	int nlink;
	
	sqfs_md_cursor next;
	
	union {
		struct {
			int major, minor;
		} dev;
		size_t symlink_size;
		struct {
			uint64_t start_block;
			uint64_t file_size;
			uint32_t frag_idx;
			uint32_t frag_off;
		} reg;
		struct {
			uint32_t start_block;
			uint16_t offset;
			uint32_t dir_size;
			uint16_t idx_count;
			uint32_t parent_inode;
		} dir;
	} xtra;
};

void sqfs_version_supported(int *min_major, int *min_minor, int *max_major,
	int *max_minor);

/* Number of groups of size 'group' required to hold size 'total' */
size_t sqfs_divceil(uint64_t total, size_t group);


sqfs_err sqfs_init(sqfs *fs, sqfs_fd_t fd, size_t offset);
void sqfs_destroy(sqfs *fs);

/* Ok to call these even on incompletely constructed filesystems */
void sqfs_version(sqfs *fs, int *major, int *minor);
sqfs_compression_type sqfs_compression(sqfs *fs);


void sqfs_md_header(uint16_t hdr, short *compressed, uint16_t *size);
void sqfs_data_header(uint32_t hdr, short *compressed, uint32_t *size);

sqfs_err sqfs_block_read(sqfs *fs, sqfs_off_t pos, short compressed, uint32_t size,
	size_t outsize, sqfs_block **block);
void sqfs_block_dispose(sqfs_block *block);

sqfs_err sqfs_md_block_read(sqfs *fs, sqfs_off_t pos, size_t *data_size,
	sqfs_block **block);
sqfs_err sqfs_data_block_read(sqfs *fs, sqfs_off_t pos, uint32_t hdr,
	sqfs_block **block);

/* Don't dispose after getting block, it's in the cache */
sqfs_err sqfs_md_cache(sqfs *fs, sqfs_off_t *pos, sqfs_block **block);
sqfs_err sqfs_data_cache(sqfs *fs, sqfs_cache *cache, sqfs_off_t pos,
	uint32_t hdr, sqfs_block **block);

void sqfs_md_cursor_inode(sqfs_md_cursor *cur, sqfs_inode_id id, sqfs_off_t base);

sqfs_err sqfs_md_read(sqfs *fs, sqfs_md_cursor *cur, void *buf, size_t size);


sqfs_err sqfs_inode_get(sqfs *fs, sqfs_inode *inode, sqfs_inode_id id);

sqfs_mode_t sqfs_mode(int inode_type);
sqfs_err sqfs_id_get(sqfs *fs, uint16_t idx, sqfs_id_t *id);

/* Puts up to *size characters of the link name into buf. Always null-
 * terminates the buffer. Pass null as buf to have the size returned. */
sqfs_err sqfs_readlink(sqfs *fs, sqfs_inode *inode, char *buf, size_t *size);

/* Find inode_id by inode_num */
int sqfs_export_ok(sqfs *fs);
sqfs_err sqfs_export_inode(sqfs *fs, sqfs_inode_num n, sqfs_inode_id *i);

/* Find the root inode */
sqfs_inode_id sqfs_inode_root(sqfs *fs);

#endif
