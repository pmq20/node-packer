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
#ifndef SQFS_HASH_H
#define SQFS_HASH_H

#include "squash/common.h"

/* Simple hashtable
 *	- Keys are integers
 *	- Values are opaque data
 *
 * Implementation
 *	- Hash function is modulus
 *	- Chaining for duplicates
 *	- Sizes are powers of two
 */
typedef uint32_t sqfs_hash_key;
typedef void *sqfs_hash_value;

typedef struct sqfs_hash_bucket {
	struct sqfs_hash_bucket *next;
	sqfs_hash_key key;
	char value[1]; /* extended to size */
} sqfs_hash_bucket;

typedef struct {
	size_t value_size;
	size_t capacity;
	size_t size;
	sqfs_hash_bucket **buckets;
} sqfs_hash;

sqfs_err sqfs_hash_init(sqfs_hash *h, size_t vsize, size_t initial);
void sqfs_hash_destroy(sqfs_hash *h);

sqfs_hash_value sqfs_hash_get(sqfs_hash *h, sqfs_hash_key k);

sqfs_err sqfs_hash_add(sqfs_hash *h, sqfs_hash_key k, sqfs_hash_value v);
sqfs_err sqfs_hash_remove(sqfs_hash *h, sqfs_hash_key k);

#endif
