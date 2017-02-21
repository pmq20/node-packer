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
#ifndef SQFS_CACHE_H
#define SQFS_CACHE_H

#include "squash/common.h"

/* Really simplistic cache
 *  - Linear search
 *  - Linear eviction
 *  - No thread safety
 *  - Misses are caller's responsibility
 */
#define SQFS_CACHE_IDX_INVALID 0

typedef uint64_t sqfs_cache_idx;
typedef void (*sqfs_cache_dispose)(void* data);

typedef struct {
	sqfs_cache_idx *idxs;
	uint8_t *buf;
	
	sqfs_cache_dispose dispose;
	
	size_t size, count;
	size_t next; /* next block to evict */
	
	MUTEX mutex;
} sqfs_cache;

sqfs_err sqfs_cache_init(sqfs_cache *cache, size_t size, size_t count,
	sqfs_cache_dispose dispose);
void sqfs_cache_destroy(sqfs_cache *cache);

void *sqfs_cache_get(sqfs_cache *cache, sqfs_cache_idx idx);
void *sqfs_cache_add(sqfs_cache *cache, sqfs_cache_idx idx);


typedef struct {
	sqfs_block *block;
	size_t data_size;
} sqfs_block_cache_entry;

sqfs_err sqfs_block_cache_init(sqfs_cache *cache, size_t count);

#endif
