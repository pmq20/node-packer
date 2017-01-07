/*
 * Copyright (c) 2016-2017 Minqi Pan <pmq2001@gmail.com>
 *                         Shengyuan Liu <sounder.liu@gmail.com>
 *
 * This file is part of libsquash, distributed under the MIT License
 * For full terms see the included LICENSE file
 */

#ifndef ENCLOSE_IO_INTERCEPT_H_1B3D379C
#define ENCLOSE_IO_INTERCEPT_H_1B3D379C
#ifndef __cplusplus
#ifndef _WIN32

#define ENCLOSE_IO_PP_NARG(...) \
    ENCLOSE_IO_PP_NARG_(__VA_ARGS__,ENCLOSE_IO_PP_RSEQ_N())
#define ENCLOSE_IO_PP_NARG_(...) \
    ENCLOSE_IO_PP_ARG_N(__VA_ARGS__)
#define ENCLOSE_IO_PP_ARG_N( \
     _1, _2, _3, _4, _5, _6, _7, _8, _9,_10, \
    _11,_12,_13,_14,_15,_16,_17,_18,_19,_20, \
    _21,_22,_23,_24,_25,_26,_27,_28,_29,_30, \
    _31,_32,_33,_34,_35,_36,_37,_38,_39,_40, \
    _41,_42,_43,_44,_45,_46,_47,_48,_49,_50, \
    _51,_52,_53,_54,_55,_56,_57,_58,_59,_60, \
    _61,_62,_63,  N, ...) N
#define ENCLOSE_IO_PP_RSEQ_N() \
    63,62,61,60,                   \
    59,58,57,56,55,54,53,52,51,50, \
    49,48,47,46,45,44,43,42,41,40, \
    39,38,37,36,35,34,33,32,31,30, \
    29,28,27,26,25,24,23,22,21,20, \
    19,18,17,16,15,14,13,12,11,10, \
     9, 8, 7, 6, 5, 4, 3, 2, 1, 0

#define chdir(x) enclose_io_chdir((x))
int enclose_io_chdir(const char *path);

#define getcwd(x, y) enclose_io_getcwd((x), (y))
char *enclose_io_getcwd(char *buf, size_t size);

#define getwd(x) enclose_io_getwd((x))
char *enclose_io_getwd(char *buf);

#define stat(x, y)	enclose_io_stat((x), (y))
int enclose_io_stat(const char *path, struct stat *buf);

#define lstat(x, y)	enclose_io_lstat((x), (y))
int enclose_io_lstat(const char *path, struct stat *buf);

#define fstat(x, y)	enclose_io_fstat((x), (y))
int enclose_io_fstat(int fildes, struct stat *buf);

#define open(...)	enclose_io_open(ENCLOSE_IO_PP_NARG(__VA_ARGS__), __VA_ARGS__)
int enclose_io_open(int nargs, const char *pathname, int flags, ...);

#define close(x)	enclose_io_close((x))
int enclose_io_close(int fildes);

#define read(x, y, z)	enclose_io_read((x), (y), (z))
ssize_t enclose_io_read(int fildes, void *buf, size_t nbyte);

#define lseek(x, y, z)	enclose_io_lseek((x), (y), (z))
off_t enclose_io_lseek(int fildes, off_t offset, int whence);

#define readlink(x, y, z) enclose_io_readlink((x), (y), (z))
ssize_t enclose_io_readlink(const char *path, char *buf, size_t bufsize);

#define opendir(x)	enclose_io_opendir((x))
DIR * enclose_io_opendir(const char *filename);

#define closedir(x)	enclose_io_closedir((x))
int enclose_io_closedir(DIR *dirp);

#define readdir(x)	enclose_io_readdir((x))
struct dirent * enclose_io_readdir(DIR *dirp);

#define telldir(x)	enclose_io_telldir((x))
long enclose_io_telldir(DIR *dirp);

#define seekdir(x, y)	enclose_io_seekdir((x), (y))
void enclose_io_seekdir(DIR *dirp, long loc);

#define rewinddir(x)	enclose_io_rewinddir((x))
void enclose_io_rewinddir(DIR *dirp);

#define dirfd(x)	enclose_io_dirfd((x))
int enclose_io_dirfd(DIR *dirp);

#define scandir(x, y, z, d)	enclose_io_scandir((x), (y), (z), (d))
int enclose_io_scandir(const char *dirname, struct dirent ***namelist,
	int (*select)(const struct dirent *),
	int (*compar)(const struct dirent **, const struct dirent **));

#endif
#endif
#endif
