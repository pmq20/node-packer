/*
 * Copyright (c) 2017 Minqi Pan <pmq2001@gmail.com>
 *                    Shengyuan Liu <sounder.liu@gmail.com>
 *
 * This file is part of libsquash, distributed under the MIT License
 * For full terms see the included LICENSE file
 */

#ifndef ENCLOSE_IO_COMMON_H_39323079
#define ENCLOSE_IO_COMMON_H_39323079

#include "squash.h"

#include <stdlib.h>
#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <stdarg.h>
#include <assert.h>

#ifndef _WIN32
#include <sys/param.h>
#include <sys/uio.h>
#include <unistd.h>
#include <dirent.h>
#endif

extern sqfs *enclose_io_fs;
extern sqfs_name enclose_io_cwd;
extern const uint8_t enclose_io_memfs[];

#define IS_ENCLOSE_IO_PATH(pathname) (0 == strncmp((pathname), "/__enclose_io_memfs__", 21))

#endif
