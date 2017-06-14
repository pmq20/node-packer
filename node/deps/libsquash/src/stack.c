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
#include "squash/stack.h"

#include <stdlib.h>

/* Ensure a capacity of cap */
static sqfs_err sqfs_stack_capacity(sqfs_stack *s, size_t cap) {
	char *items;
	if (cap <= s->capacity)
		return SQFS_OK;
	
	items = realloc(s->items, cap * s->value_size);
	if (!items)
		return SQFS_ERR;
	
	s->items = items;
	s->capacity = cap;
	return SQFS_OK;
}

/* Calculate the next capacity to use */
#define CAPACITY_DEFAULT 8
#define CAPACITY_RATIO 3 / 2
static size_t sqfs_stack_next_capacity(size_t cap) {
	size_t n;
	
	if (cap == 0)
		return CAPACITY_DEFAULT;
	
	n = cap * CAPACITY_RATIO;
	if (n <= cap)
		return cap + 1;
	return n;
}

/* Grow by one */
static sqfs_err sqfs_stack_grow(sqfs_stack *s) {
	if (s->size == s->capacity) {
		sqfs_err err = sqfs_stack_capacity(s,
			sqfs_stack_next_capacity(s->capacity));
		if (err)
			return err;
	}
	s->size++;
	return SQFS_OK;
}


sqfs_err sqfs_stack_create(sqfs_stack *s, size_t vsize, size_t initial,
		sqfs_stack_free_t freer) {
	s->value_size = vsize;
	s->freer = freer;
	s->items = NULL;
	s->capacity = s->size = 0;
	return sqfs_stack_capacity(s, initial);
}

void sqfs_stack_init(sqfs_stack *s) {
	s->items = NULL;
	s->capacity = 0;
}

void sqfs_stack_destroy(sqfs_stack *s) {
	while (sqfs_stack_pop(s))
		; /* pass */
	free(s->items);
	sqfs_stack_init(s);
}

sqfs_err sqfs_stack_push(sqfs_stack *s, void *vout) {
	sqfs_err err = sqfs_stack_grow(s);
	if (err)
		return err;
	return sqfs_stack_top(s, vout);
}

short sqfs_stack_pop(sqfs_stack *s) {
	void *v;
	
	if (s->size == 0)
		return 0;
	
	sqfs_stack_top(s, &v);
	if (s->freer)
		s->freer(v);
	s->size--;
	return 1;
}

size_t sqfs_stack_size(sqfs_stack *s) {
	return s->size;
}

sqfs_err sqfs_stack_at(sqfs_stack *s, size_t i, void *vout) {
	if (i >= s->size)
		return SQFS_ERR;
	
	*(void**)vout = s->items + i * s->value_size;
	return SQFS_OK;
}

sqfs_err sqfs_stack_top(sqfs_stack *s, void *vout) {
	if (s->size == 0)
		return SQFS_ERR;
	
	return sqfs_stack_at(s, s->size - 1, vout);
}
