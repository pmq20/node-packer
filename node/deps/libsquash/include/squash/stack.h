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
#ifndef SQFS_STACK_H
#define SQFS_STACK_H

#include "squash/common.h"

typedef void (*sqfs_stack_free_t)(void *v);

typedef struct {
	size_t value_size;
	size_t size;
	size_t capacity;
	char *items;
	sqfs_stack_free_t freer;
} sqfs_stack;

/* Ensures the struct is in a safe state */
void sqfs_stack_init(sqfs_stack *s);

sqfs_err sqfs_stack_create(sqfs_stack *s, size_t vsize, size_t initial,
	sqfs_stack_free_t freer);
void sqfs_stack_destroy(sqfs_stack *s);

sqfs_err sqfs_stack_push(sqfs_stack *s, void *vout);
short sqfs_stack_pop(sqfs_stack *s);

size_t sqfs_stack_size(sqfs_stack *s);
sqfs_err sqfs_stack_at(sqfs_stack *s, size_t i, void *vout);
sqfs_err sqfs_stack_top(sqfs_stack *s, void *vout);

#endif
