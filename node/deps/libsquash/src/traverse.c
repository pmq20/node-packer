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
#include "squash/traverse.h"

#include "squash/fs.h"

#include <stdlib.h>
#include <string.h>


#define TRAVERSE_PATH_SEPARATOR "/"

/* Default initial capacity of trv.path */
#define TRAVERSE_DEFAULT_PATH_CAP 32


enum {
	/* These states may be set on entry to sqfs_traverse_next(), with real
	   work to do. */
	TRAVERSE_DESCEND,		 	/* Descend into the current entry (a dir) */
	TRAVERSE_NAME_REMOVE, /* Remove the name from the end of the stored path */
	
	/* End states */
	TRAVERSE_ERROR,
	TRAVERSE_FINISHED,
	
	/* Internal */
	TRAVERSE_ASCEND,			/* Done with a directory, ascend a level */
	TRAVERSE_NAME_ADD,		/* Add a name to the end of the stored path */
	TRAVERSE_GET_ENTRY		/* Get the next entry at the same level */
} sqfs_traverse_state;

/* The struct stored in trv.stack */
typedef struct {
	sqfs_dir dir;
	size_t name_size;
} sqfs_traverse_level;


/* Make our structure safe */
static void sqfs_traverse_init(sqfs_traverse *trv);

/* Path manipulation functions */
static sqfs_err sqfs_traverse_path_init(sqfs_traverse *trv);
static sqfs_err sqfs_traverse_path_add(sqfs_traverse *trv,
	const char *str, size_t size);
static sqfs_err sqfs_traverse_path_add_name(sqfs_traverse *trv);
static sqfs_err sqfs_traverse_path_add_sep(sqfs_traverse *trv);
static void sqfs_traverse_path_remove(sqfs_traverse *trv, size_t size);
static void sqfs_traverse_path_remove_name(sqfs_traverse *trv);
static void sqfs_traverse_path_remove_sep(sqfs_traverse *trv);
/* Set the size of the last path component */
static void sqfs_traverse_path_set_name_size(sqfs_traverse *trv, size_t size);
/* Add nul-terminator */
static void sqfs_traverse_path_terminate(sqfs_traverse *trv);

/* Descend into new directories, and ascend back */
static sqfs_err sqfs_traverse_descend_inode(sqfs_traverse *trv,
	sqfs_inode *inode);
static sqfs_err sqfs_traverse_descend(sqfs_traverse *trv, sqfs_inode_id iid);
static sqfs_err sqfs_traverse_ascend(sqfs_traverse *trv);


static void sqfs_traverse_init(sqfs_traverse *trv) {
	sqfs_dentry_init(&trv->entry, trv->namebuf);
	sqfs_stack_init(&trv->stack);
	trv->state = TRAVERSE_ERROR;
	trv->path = NULL;
}

sqfs_err sqfs_traverse_open_inode(sqfs_traverse *trv, sqfs *fs,
		sqfs_inode *inode) {
	sqfs_err err;
	
	sqfs_traverse_init(trv);	
	if ((err = sqfs_traverse_path_init(trv)))
		goto error;
	err = sqfs_stack_create(&trv->stack, sizeof(sqfs_traverse_level), 0, NULL);
	if (err)
		goto error;
	
	trv->fs = fs;
	if ((err = sqfs_traverse_descend_inode(trv, inode)))
		goto error;
	
	sqfs_traverse_path_set_name_size(trv, 0); /* The root has no name */
	trv->state = TRAVERSE_NAME_REMOVE;
	return SQFS_OK;
	
error:
	sqfs_traverse_close(trv);
	return err;
}

sqfs_err sqfs_traverse_open(sqfs_traverse *trv, sqfs *fs, sqfs_inode_id iid) {
	sqfs_err err;
	sqfs_inode inode;
	
	if ((err = sqfs_inode_get(fs, &inode, iid)))
		return err;
	
	return sqfs_traverse_open_inode(trv, fs, &inode);
}

void sqfs_traverse_close(sqfs_traverse *trv) {
	sqfs_stack_destroy(&trv->stack);
	free(trv->path);
	sqfs_traverse_init(trv);
}


short sqfs_traverse_next(sqfs_traverse *trv, sqfs_err *err) {
	sqfs_traverse_level *level;
	short found;
	
	*err = SQFS_OK;
	while (1) {
		switch (trv->state) {
			case TRAVERSE_GET_ENTRY:
				if ((*err = sqfs_stack_top(&trv->stack, &level)))
					goto error;
				
				found = sqfs_dir_next(trv->fs, &level->dir, &trv->entry, err);
				if (*err)
					goto error;
				if (found)
					trv->state = TRAVERSE_NAME_ADD;
				else
					trv->state = TRAVERSE_ASCEND;
				break;
			
			case TRAVERSE_NAME_ADD:
				if ((*err = sqfs_traverse_path_add_name(trv)))
					goto error;
				if (sqfs_dentry_is_dir(&trv->entry))
					trv->state = TRAVERSE_DESCEND;
				else
					trv->state = TRAVERSE_NAME_REMOVE;
				trv->dir_end = 0;
				return 1;
			
			case TRAVERSE_NAME_REMOVE:
				sqfs_traverse_path_remove_name(trv);
				trv->state = TRAVERSE_GET_ENTRY;
				break;
			
			case TRAVERSE_DESCEND:
				*err = sqfs_traverse_descend(trv, sqfs_dentry_inode(&trv->entry));
				if (*err)
					goto error;
				trv->state = TRAVERSE_GET_ENTRY;
				break;
			
			case TRAVERSE_ASCEND:
				if ((*err = sqfs_traverse_ascend(trv)))
					goto error;
				if (sqfs_stack_size(&trv->stack) > 0) {
					trv->dir_end = 1;
					trv->state = TRAVERSE_NAME_REMOVE;
					return 1;
				}
				trv->state = TRAVERSE_FINISHED;
				break;
			
			case TRAVERSE_FINISHED:
				return 0;
			
			case TRAVERSE_ERROR:
				*err = SQFS_ERR;
				goto error;
		}
	}
	
error:
	trv->state = TRAVERSE_ERROR;
	return 0;
}

sqfs_err sqfs_traverse_prune(sqfs_traverse *trv) {
	trv->state = TRAVERSE_NAME_REMOVE;
	return SQFS_OK;
}


static sqfs_err sqfs_traverse_path_init(sqfs_traverse *trv) {
	trv->path_cap = TRAVERSE_DEFAULT_PATH_CAP;
	if (!(trv->path = malloc(trv->path_cap)))
		return SQFS_ERR;
	trv->path[0] = '\0';
	trv->path_size = 1; /* includes nul-terminator */
	return SQFS_OK;
}

static void sqfs_traverse_path_terminate(sqfs_traverse *trv) {
	trv->path[trv->path_size - 1] = '\0';
}

static sqfs_err sqfs_traverse_path_add(sqfs_traverse *trv,
		const char *str, size_t size) {
	size_t need = trv->path_size + size;
	if (need > trv->path_cap) {
		char *next_path;
		size_t next_cap = trv->path_cap;
		while (need > next_cap)
			next_cap *= 2;
		
		if (!(next_path = realloc(trv->path, next_cap)))
			return SQFS_ERR;
		
		trv->path = next_path;
		trv->path_cap = next_cap;
	}
	
	memcpy(trv->path + trv->path_size - 1, str, size);
	trv->path_size = need;
	sqfs_traverse_path_terminate(trv);
	return SQFS_OK;
}

static void sqfs_traverse_path_remove(sqfs_traverse *trv, size_t size) {
	if (trv->path_size > size)
		trv->path_size -= size;
	else
		trv->path_size = 1; /* only nul terminator left */
	
	sqfs_traverse_path_terminate(trv);
}

static sqfs_err sqfs_traverse_path_add_name(sqfs_traverse *trv) {
	trv->path_last_size = sqfs_dentry_name_size(&trv->entry);
	return sqfs_traverse_path_add(trv, sqfs_dentry_name(&trv->entry),
		trv->path_last_size);
}

static sqfs_err sqfs_traverse_path_add_sep(sqfs_traverse *trv) {
	return sqfs_traverse_path_add(trv, TRAVERSE_PATH_SEPARATOR,
		strlen(TRAVERSE_PATH_SEPARATOR));
}

static void sqfs_traverse_path_remove_name(sqfs_traverse *trv) {
	sqfs_traverse_path_remove(trv, trv->path_last_size);
}

static void sqfs_traverse_path_remove_sep(sqfs_traverse *trv) {
	sqfs_traverse_path_remove(trv, strlen(TRAVERSE_PATH_SEPARATOR));
}

static void sqfs_traverse_path_set_name_size(sqfs_traverse *trv, size_t size) {
	trv->path_last_size = size;
}


static sqfs_err sqfs_traverse_descend_inode(sqfs_traverse *trv,
		sqfs_inode *inode) {
	sqfs_err err;
	sqfs_traverse_level *level;
	short initial;
	
	initial = (sqfs_stack_size(&trv->stack) == 0);
	
	if ((err = sqfs_stack_push(&trv->stack, &level)))
		return err;	
	if ((err = sqfs_dir_open(trv->fs, inode, &level->dir, 0)))
		return err;
	
	if (initial) {
		/* Don't add the separator or store the size for the root directory */
		level->name_size = 0;
	} else {
		level->name_size = sqfs_dentry_name_size(&trv->entry);
		if ((err = sqfs_traverse_path_add_sep(trv)))
			return err;
	}
	
	return err;
}

static sqfs_err sqfs_traverse_descend(sqfs_traverse *trv, sqfs_inode_id iid) {
	sqfs_err err;
	sqfs_inode inode;
	
	if ((err = sqfs_inode_get(trv->fs, &inode, iid)))
		return err;
	
	return sqfs_traverse_descend_inode(trv, &inode);
}

static sqfs_err sqfs_traverse_ascend(sqfs_traverse *trv) {
	sqfs_err err;
	sqfs_traverse_level *level;
	
	if ((err = sqfs_stack_top(&trv->stack, &level)))
		return err;
	
	sqfs_traverse_path_remove_sep(trv); /* safe even if initial */
	sqfs_traverse_path_set_name_size(trv, level->name_size);
	
	sqfs_stack_pop(&trv->stack);
	return SQFS_OK;
}
