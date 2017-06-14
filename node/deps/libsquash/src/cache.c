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
#include "squash/cache.h"

#include "squash/fs.h"

#include <stdlib.h>

sqfs_err sqfs_cache_init(sqfs_cache *cache, size_t size, size_t count,
		sqfs_cache_dispose dispose) {
	cache->size = size;
	cache->count = count;
	cache->dispose = dispose;
	cache->next = 0;

	MUTEX_INIT(&cache->mutex);
	
	cache->idxs = calloc(count, sizeof(sqfs_cache_idx));
	cache->buf = calloc(count, size);
	if (cache->idxs && cache->buf)
		return SQFS_OK;
	
	sqfs_cache_destroy(cache);
	return SQFS_ERR;
}

static void *sqfs_cache_entry(sqfs_cache *cache, size_t i) {
	return cache->buf + i * cache->size;
}

void sqfs_cache_destroy(sqfs_cache *cache) {
	MUTEX_DESTORY(&cache->mutex);

	if (cache->buf && cache->idxs) {
		size_t i;
		for (i = 0; i < cache->count; ++i) {
			if (cache->idxs[i] != SQFS_CACHE_IDX_INVALID)
				cache->dispose(sqfs_cache_entry(cache, i));
		}
	}
	free(cache->buf);
	free(cache->idxs);
}

void *sqfs_cache_get(sqfs_cache *cache, sqfs_cache_idx idx) {
	size_t i;
	void *ret = NULL;
	for (i = 0; i < cache->count; ++i) {
		if (cache->idxs[i] == idx){
			ret = sqfs_cache_entry(cache, i);
			break;
		}

	}
	return ret;
}

void *sqfs_cache_add(sqfs_cache *cache, sqfs_cache_idx idx) {
	size_t i = (cache->next++);
	cache->next %= cache->count;
	
	if (cache->idxs[i] != SQFS_CACHE_IDX_INVALID)
		cache->dispose(sqfs_cache_entry(cache, i));
	
	cache->idxs[i] = idx;
	return sqfs_cache_entry(cache, i);
}

static void sqfs_block_cache_dispose(void *data) {
	sqfs_block_cache_entry *entry = (sqfs_block_cache_entry*)data;
	sqfs_block_dispose(entry->block);
}

sqfs_err sqfs_block_cache_init(sqfs_cache *cache, size_t count) {
	return sqfs_cache_init(cache, sizeof(sqfs_block_cache_entry), count,
		&sqfs_block_cache_dispose);
}
