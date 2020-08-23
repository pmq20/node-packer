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
 *
 * Copyright (c) 2017 Minqi Pan <pmq2001@gmail.com>
 *                    Shengyuan Liu <sounder.liu@gmail.com>
 *
 * This file is part of libsquash, distributed under the MIT License
 * For full terms see the included LICENSE file
 */

#ifndef SQFS_SQUASH_H
#define SQFS_SQUASH_H

#include <string.h>
#include <errno.h>
#include <assert.h>

#include "squash/dir.h"
#include "squash/file.h"
#include "squash/fs.h"
#include "squash/traverse.h"
#include "squash/util.h"
#include "squash/private.h"
#include "squash/fdtable.h"
#include "squash/dirent.h"

#define SQUASH_SEEK_SET 0 /* set file offset to offset */
#define SQUASH_SEEK_CUR 1 /* set file offset to current plus offset */
#define SQUASH_SEEK_END 2 /* set file offset to EOF plus offset */

#define SQUASH_VALID_VFD(vfd) ((vfd) < squash_global_fdtable.nr && NULL != squash_global_fdtable.fds[(vfd)])
#define SQUASH_VFD_FILE(vfd) (squash_global_fdtable.fds[(vfd)])

extern sqfs_err squash_errno;

sqfs_err squash_start();
ssize_t squash_readlink_inode(sqfs *fs, sqfs_inode *node, char *buf, size_t bufsize);
sqfs_err squash_follow_link(sqfs *fs, const char *path, sqfs_inode *node);
struct squash_file * squash_find_entry(void *ptr);

/*
 * Obtains information about the file pointed to by path of a SquashFS fs.
 * The buf argument is a pointer to a stat structure as defined by
 * <sys/stat.h> and into which information is placed concerning the file.
 * Upon successful completion a value of 0 is returned.
 * Otherwise, a value of -1 is returned and
 * error is set to the reason of the error.
 */
int squash_stat(sqfs *fs, const char *path, struct stat *buf);

/*
 * Acts like squash_stat() except in the case where the named file
 * is a symbolic link; squash_lstat() returns information about the link,
 * while squash_stat() returns information about the file the link references.
 */
int squash_lstat(sqfs *fs, const char *path, struct stat *buf);

/*
 * Obtains the same information as squash_stat()
 * about an open file known by the virtual file descriptor vfd.
 */
int squash_fstat(int vfd, struct stat *buf);

/*
 * Opens the file name specified by path of fs for reading.
 * If successful, squash_open() returns a non-negative integer,
 * termed a vfd(virtual file descriptor).
 * It returns -1 on failure and sets error to the reason of the error.
 * The file pointer (used to mark the current position within the file)
 * is set to the beginning of the file.
 * The returned vfd should later be closed by squash_close().
 */
int squash_open(sqfs *fs, const char *path);

int squash_open_inner(sqfs *fs, const char *path, short follow_link);

/*
 * Deletes a vfd(virtual file descriptor) from
 * the per-process object reference table.
 * Upon successful completion, a value of 0 is returned.
 * Otherwise, a value of -1 is returned and error is set to
 * the reason of the error.
 */
int squash_close(int vfd);

/*
 * Attempts to read nbyte bytes of data from the object
 * referenced by vfs into the buffer pointed to by buf,
 * starting at a position given by the pointer
 * associated with vfd (see squash_lseek),
 * which is then incremented by the number of bytes actually read upon return.
 * When successful it returns the number of bytes actually read
 * and placed in the buffer;
 * upon reading end-of-file, zero is returned;
 * Otherwise, a value of -1 is returned and error is set to
 * the reason of the error.
 */
ssize_t squash_read(int vfd, void *buf, sqfs_off_t nbyte);

/*
 * Repositions the offset of vfs to the argument offset,
 * according to the directive whence.
 * If whence is SQUASH_SEEK_SET then the offset is set to offset bytes;
 * if whence is SQUASH_SEEK_CUR, the offset is set to
 * its current location plus offset bytes;
 * if whence is SQUASH_SEEK_END, the offset is set to
 * the size of the file and
 * subsequent reads of the data return bytes of zeros.
 * The argument fildes` must be an open virtual file descriptor.
 * Upon successful completion,
 * it returns the resulting offset location as measured in bytes
 * from the beginning of the file.
 * Otherwise, a value of -1 is returned and
 * error is set to the reason of the error.
 */
off_t squash_lseek(int vfd, off_t offset, int whence);

/*
 * Places the contents of the symbolic link path of a SquashFS fs
 * in the buffer buf, which has size bufsize.
 * It does not append a NUL character to buf.
 * If it succeeds the call returns the count of characters placed in the buffer;
 * otherwise -1 is returned and error is set to the reason of the error.
 */
ssize_t squash_readlink(sqfs *fs, const char *path, char *buf, size_t bufsize);

/*
 * Opens the directory named by filename of a SquashFS fs,
 * associates a directory stream with it and returns a pointer
 * to be used to identify the directory stream in subsequent operations.
 * The pointer NULL is returned if filename cannot be accessed,
 * or if it cannot allocate enough memory to hold the whole thing,
 * and sets error to the reason of the error.
 * The returned resource should later be closed by squash_closedir().
 */
SQUASH_DIR * squash_opendir(sqfs *fs, const char *filename);

SQUASH_DIR * squash_opendir_inner(sqfs *fs, const char *filename, short follow_link);

/*
 * Closes the named directory stream and
 * frees the structure associated with the dirp pointer,
 * returning 0 on success.
 * On failure, -1 is returned and error is set to the reason of the error.
 */
int squash_closedir(SQUASH_DIR *dirp);

/*
 * Returns a pointer to the next directory entry.
 * It returns NULL upon reaching the end of the directory or on error. 
 * In the event of an error, error is set to the reason of the error.
 */
struct SQUASH_DIRENT * squash_readdir(SQUASH_DIR *dirp);

/*
 * Returns the current location associated with the named directory stream.
 */
long squash_telldir(SQUASH_DIR *dirp);

/*
 * Sets the position of the next squash_readdir() operation
 * on the directory stream.
 * The new position reverts to the one associated with the directory stream
 * when the squash_telldir() operation was performed.
 */
void squash_seekdir(SQUASH_DIR *dirp, long loc);

/*
 * Resets the position of the named directory stream to
 * the beginning of the directory.
 */
void squash_rewinddir(SQUASH_DIR *dirp);

/*
 * Returns the integer virtual file descriptor
 * associated with the named directory stream.
 * On failure, -1 is returned and error is set to the reason of the error.
 */
int squash_dirfd(SQUASH_DIR *dirp);

/*
 * Reads the directory dirname of a SquashFS fs and
 * builds an array of pointers to directory entries using malloc.
 * If successful it returns the number of entries in the array; 
 * otherwise -1 is returned and error is set to the reason of the error.
 * A pointer to the array of directory entries is stored
 * in the location referenced by namelist (even if the number of entries is 0),
 * which should later be freed via free() by freeing each pointer
 * in the array and then the array itself.
 * The select argument is a pointer to a user supplied subroutine which is
 * called by scandir to select which entries are to be included in the array.
 * The select routine is passed a pointer to a directory entry
 * and should return a non-zero value if the directory entry
 * is to be included in the array.
 * If select is NULL, then all the directory entries will be included.
 * The compar argument is a pointer to a user supplied subroutine
 * which is passed to qsort to sort the completed array.
 * If this pointer is NULL, then the array is not sorted.
 */
int squash_scandir(sqfs *fs, const char *dirname, struct SQUASH_DIRENT ***namelist,
	int (*select)(const struct SQUASH_DIRENT *),
	int (*compar)(const struct SQUASH_DIRENT **, const struct SQUASH_DIRENT **));

/*
 * Extracts the file `path` from `fs` to a temporary file
 * inside the temporary folder.
 * Upon successful completion the path of the extracted temporary file
 * is returned.
 * Otherwise, a value of `NULL` is returned
 * and `errno` is set to the reason of the error.
 * The returned path is referenced by an internal cache and must not be freed.
 */
#ifdef _WIN32
#define SQUASH_OS_PATH const wchar_t*
#else
#define SQUASH_OS_PATH const char*
#endif
SQUASH_OS_PATH squash_tmpdir();
SQUASH_OS_PATH squash_tmpf(SQUASH_OS_PATH tmpdir, const char *ext_name);
SQUASH_OS_PATH squash_extract(sqfs *fs, const char *path, const char *ext_name);
void squash_extract_clear_cache();

#endif
