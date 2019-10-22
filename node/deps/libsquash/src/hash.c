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
#include "squash/hash.h"

#include <stdlib.h>
#include <string.h>

static sqfs_err sqfs_hash_add_internal(sqfs_hash *h, int doubling,
		sqfs_hash_key k, sqfs_hash_value v) {
	size_t hash = (k & (h->capacity - 1));	
	sqfs_hash_bucket *b = malloc(sizeof(sqfs_hash_bucket) + h->value_size);
	if (!b)
		return SQFS_ERR;
	b->key = k;
	memcpy(&b->value, v, h->value_size);
	b->next = h->buckets[hash];
	h->buckets[hash] = b;
	++h->size;
	
	return SQFS_OK;
}

static sqfs_err sqfs_hash_double(sqfs_hash *h) {
	sqfs_hash_bucket **ob = h->buckets;
	size_t oc = h->capacity;
	size_t i;
	sqfs_err err;
	
	if ((err = sqfs_hash_init(h, h->value_size, oc * 2)))
		return err;
	
	for (i = 0; i < oc; ++i) {
		sqfs_hash_bucket *b = ob[i];
		while (b) {
			sqfs_hash_bucket *n;
			if (!err)
				err = sqfs_hash_add_internal(h, 1, b->key, &b->value);
			n = b->next;
			free(b);
			b = n;
		}
	}
	
	free(ob);
	return err;
}

sqfs_err sqfs_hash_init(sqfs_hash *h, size_t vsize, size_t initial) {
	memset(h, 0, sizeof(*h));
	if ((initial & (initial - 1))) /* not power of two? */
		return SQFS_ERR;
	
	h->buckets = calloc(initial, sizeof(sqfs_hash_bucket*));
	if (!h->buckets)
		return SQFS_ERR;
	h->capacity = initial;
	h->size = 0;
	h->value_size = vsize;
	return SQFS_OK;
}
 
void sqfs_hash_destroy(sqfs_hash *h) {
	size_t i;
	for (i = 0; i < h->capacity; ++i) {
		sqfs_hash_bucket *b = h->buckets[i];
		while (b) {
			sqfs_hash_bucket *n = b->next;
			free(b);
			b = n;
		}
	}
	free(h->buckets);
}

sqfs_hash_value sqfs_hash_get(sqfs_hash *h, sqfs_hash_key k) {
	size_t hash = (k & (h->capacity - 1));
	sqfs_hash_bucket *b = h->buckets[hash];
	while (b) {
		if (b->key == k)
			return &b->value;
		b = b->next;
	}
	return NULL;
}

sqfs_err sqfs_hash_add(sqfs_hash *h, sqfs_hash_key k, sqfs_hash_value v) {
	if (h->size >= h->capacity) {
		sqfs_err err = sqfs_hash_double(h);
		if (err)
			return err;
	}
	return sqfs_hash_add_internal(h, 0, k, v);
}

sqfs_err sqfs_hash_remove(sqfs_hash *h, sqfs_hash_key k) {
	size_t hash = (k & (h->capacity - 1));
	sqfs_hash_bucket **bp = &h->buckets[hash];
	while (*bp) {
		if ((*bp)->key == k) {
			sqfs_hash_bucket *b = *bp;
			*bp = b->next;
			free(b);
			--h->size;
			return SQFS_OK;
		}
		bp = &(*bp)->next;
	}
	return SQFS_OK;
}
