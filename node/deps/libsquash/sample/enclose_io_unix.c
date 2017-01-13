/*
 * Copyright (c) 2017 Minqi Pan <pmq2001@gmail.com>
 *                    Shengyuan Liu <sounder.liu@gmail.com>
 *
 * This file is part of libsquash, distributed under the MIT License
 * For full terms see the included LICENSE file
 */

#include "enclose_io_common.h"
#ifndef _WIN32
#define ENCLOSE_IO_GEN_EXPANDED_NAME(path)	sqfs_name enclose_io_expanded_name; \
						memcpy(enclose_io_expanded_name, enclose_io_cwd, enclose_io_cwd_len); \
						size_t memcpy_len = strlen(path); \
						if (SQUASHFS_NAME_LEN - enclose_io_cwd_len < memcpy_len) { memcpy_len = SQUASHFS_NAME_LEN - enclose_io_cwd_len; } \
						memcpy(&enclose_io_expanded_name[enclose_io_cwd_len], (path), memcpy_len); \
						enclose_io_expanded_name[enclose_io_cwd_len + memcpy_len] = '\0'

int enclose_io_chdir(const char *path)
{
	int ret;
	if (IS_ENCLOSE_IO_PATH(path)) {
		struct stat st;
		ret = squash_stat(enclose_io_fs, path, &st);
		if (0 == ret && S_ISDIR(st.st_mode)) {
			size_t memcpy_len = strlen(path);
			if (SQUASHFS_NAME_LEN - 1 < memcpy_len){
				memcpy_len = SQUASHFS_NAME_LEN - 1;
			}
			memcpy(enclose_io_cwd, path, memcpy_len);
			while ('/' == enclose_io_cwd[memcpy_len - 1]) {
				memcpy_len--;
			}
			enclose_io_cwd[memcpy_len] = '/';
			enclose_io_cwd[memcpy_len + 1] = '\0';
			enclose_io_cwd_len = strlen(enclose_io_cwd);
			enclose_io_cwd_inside = 1;
			return 0;
		} else {
			return -1;
		}
	} else {
		ret = chdir(path);
		if (0 == ret) {
			enclose_io_cwd_inside = 0;
		}
		return ret;
	}
}

char *enclose_io_getcwd(char *buf, size_t size)
{
	if (enclose_io_cwd_inside) {
		size_t memcpy_len = enclose_io_cwd_len;
		if (NULL == buf) {
			buf = malloc((memcpy_len + 1) * sizeof(char));
			if (NULL == buf) {
				return NULL;
			}
		} else {
			if (size - 1 < memcpy_len) {
				memcpy_len = size - 1;
			}
		}
		memcpy(buf, enclose_io_cwd, memcpy_len);
		buf[memcpy_len] = '\0';
		return buf;
	} else {
		return getcwd(buf, size);
	}
}

char *enclose_io_getwd(char *buf)
{
	return enclose_io_getcwd(buf, MAXPATHLEN);
}

int enclose_io_stat(const char *path, struct stat *buf)
{
	if (enclose_io_cwd_inside && '/' != *path) {
		ENCLOSE_IO_GEN_EXPANDED_NAME(path);
		return squash_stat(enclose_io_fs, enclose_io_expanded_name, buf);
	} else if (IS_ENCLOSE_IO_PATH(path)) {
		return squash_stat(enclose_io_fs, path, buf);
	} else {
		return stat(path, buf);
	}
}

int enclose_io_lstat(const char *path, struct stat *buf)
{
	if (enclose_io_cwd_inside && '/' != *path) {
		ENCLOSE_IO_GEN_EXPANDED_NAME(path);
		return squash_lstat(enclose_io_fs, enclose_io_expanded_name, buf);
	} else if (IS_ENCLOSE_IO_PATH(path)) {
		return squash_lstat(enclose_io_fs, path, buf);
	} else {
		return lstat(path, buf);
	}
}

int enclose_io_fstat(int fildes, struct stat *buf)
{
	if (SQUASH_VALID_VFD(fildes)) {
		return squash_fstat(fildes, buf);
	} else {
		return fstat(fildes, buf);
	}
}

int enclose_io_open(int nargs, const char *pathname, int flags, ...)
{
	if (enclose_io_cwd_inside && '/' != *pathname) {
		ENCLOSE_IO_GEN_EXPANDED_NAME(pathname);
		return squash_open(enclose_io_fs, enclose_io_expanded_name);
	} else if (IS_ENCLOSE_IO_PATH(pathname)) {
		return squash_open(enclose_io_fs, pathname);
	} else {
		if (2 == nargs) {
			return open(pathname, flags);
		} else {
			va_list args;
			va_start(args, flags);
			mode_t mode = va_arg(args, mode_t);
			va_end(args);
			return open(pathname, flags, mode);
		}
	}
}

int enclose_io_close(int fildes)
{
	if (SQUASH_VALID_VFD(fildes)) {
		return squash_close(fildes);
	} else {
		return close(fildes);
	}
}

ssize_t enclose_io_read(int fildes, void *buf, size_t nbyte)
{
	if (SQUASH_VALID_VFD(fildes)) {
		return squash_read(fildes, buf, nbyte);
	} else {
		return read(fildes, buf, nbyte);
	}
}

off_t enclose_io_lseek(int fildes, off_t offset, int whence)
{
	if (SQUASH_VALID_VFD(fildes)) {
		return squash_lseek(fildes, offset, whence);
	} else {
		return lseek(fildes, offset, whence);
	}
}

ssize_t enclose_io_readlink(const char *path, char *buf, size_t bufsize)
{
	if (enclose_io_cwd_inside && '/' != *path) {
		ENCLOSE_IO_GEN_EXPANDED_NAME(path);
		return squash_readlink(enclose_io_fs, enclose_io_expanded_name, buf, bufsize);
	} else if (IS_ENCLOSE_IO_PATH(path)) {
		return squash_readlink(enclose_io_fs, path, buf, bufsize);
	} else {
		return readlink(path, buf, bufsize);
	}
}

DIR * enclose_io_opendir(const char *filename)
{
	if (enclose_io_cwd_inside && '/' != *filename) {
		ENCLOSE_IO_GEN_EXPANDED_NAME(filename);
		return (DIR *)squash_opendir(enclose_io_fs, enclose_io_expanded_name);
	} else if (IS_ENCLOSE_IO_PATH(filename)) {
		return (DIR *)squash_opendir(enclose_io_fs, filename);
	} else {
		return opendir(filename);
	}
}

int enclose_io_closedir(DIR *dirp)
{
	if (SQUASH_VALID_DIR(dirp)) {
		return squash_closedir((SQUASH_DIR *)dirp);
	} else {
		return closedir(dirp);
	}
}

struct dirent * enclose_io_readdir(DIR *dirp)
{
	if (SQUASH_VALID_DIR(dirp)) {
		return squash_readdir((SQUASH_DIR *)dirp);
	} else {
		return readdir(dirp);
	}
}

long enclose_io_telldir(DIR *dirp)
{
	if (SQUASH_VALID_DIR(dirp)) {
		return squash_telldir((SQUASH_DIR *)dirp);
	} else {
		return telldir(dirp);
	}
}

void enclose_io_seekdir(DIR *dirp, long loc)
{
	if (SQUASH_VALID_DIR(dirp)) {
		return squash_seekdir((SQUASH_DIR *)dirp, loc);
	} else {
		return seekdir(dirp, loc);
	}
}

void enclose_io_rewinddir(DIR *dirp)
{
	if (SQUASH_VALID_DIR(dirp)) {
		return squash_rewinddir((SQUASH_DIR *)dirp);
	} else {
		return rewinddir(dirp);
	}
}

int enclose_io_dirfd(DIR *dirp)
{
	if (SQUASH_VALID_DIR(dirp)) {
		return squash_dirfd((SQUASH_DIR *)dirp);
	} else {
		return dirfd(dirp);
	}
}

int enclose_io_scandir(const char *dirname, struct dirent ***namelist,
	int (*select)(const struct dirent *),
	int (*compar)(const struct dirent **, const struct dirent **))
{
	if (enclose_io_cwd_inside && '/' != *dirname) {
		ENCLOSE_IO_GEN_EXPANDED_NAME(dirname);
		return squash_scandir(enclose_io_fs, enclose_io_expanded_name, namelist, select, compar);
	} else if (IS_ENCLOSE_IO_PATH(dirname)) {
		return squash_scandir(enclose_io_fs, dirname, namelist, select, compar);
	} else {
		return scandir(dirname, namelist, select, compar);
	}
}
#endif
