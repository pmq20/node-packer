/*
 * Copyright (c) 2017 Minqi Pan <pmq2001@gmail.com>
 *                    Shengyuan Liu <sounder.liu@gmail.com>
 *
 * This file is part of libsquash, distributed under the MIT License
 * For full terms see the included LICENSE file
 */

#ifndef DIRENT_H_245C4278
#define DIRENT_H_245C4278

#include <sys/types.h>

#define SQUASH_DIR_MAGIC_LEN 18
#define MAX_DIR_ENT 1024

typedef struct {
	sqfs *fs;
	int	fd;	/* virtual file descriptor associated with directory */
	sqfs_inode node;
	sqfs_dir dir;
	// CAUTION: this is a big struct, mind the SQUASHFS_NAME_LEN size
	struct {
		sqfs_dir_entry entry;
		struct SQUASH_DIRENT sysentry;
		sqfs_name name;
		short not_eof;
	} *entries;
	size_t nr; /* allocated size for entries */
	int actual_nr; /* actual number of entries read */
	long loc;	/* offset in current buffer */
        char *filename;
        void *payload;
} SQUASH_DIR;

#endif /* end of include guard: DIRENT_H_245C4278 */
