/*
 * Copyright (c) 2017 Minqi Pan <pmq2001@gmail.com>
 *                    Shengyuan Liu <sounder.liu@gmail.com>
 *
 * This file is part of libsquash, distributed under the MIT License
 * For full terms see the included LICENSE file
 */

#ifndef ENCLOSE_IO_UNIX_H_E0229A03
#define ENCLOSE_IO_UNIX_H_E0229A03
#ifndef __cplusplus

#define chdir(...) enclose_io_chdir(__VA_ARGS__)
#define getcwd(...) enclose_io_getcwd(__VA_ARGS__)
#define stat(...)	enclose_io_stat(__VA_ARGS__)
#define fstat(...)	enclose_io_fstat(__VA_ARGS__)
#define open(...)	enclose_io_open(ENCLOSE_IO_PP_NARG(__VA_ARGS__), __VA_ARGS__)
#define close(...)	enclose_io_close(__VA_ARGS__)
#define read(...)	enclose_io_read((__VA_ARGS__)
#define lseek(...)	enclose_io_lseek(__VA_ARGS__)

#ifndef _WIN32

#ifdef dirfd
#undef dirfd
#endif
#define getwd(...) enclose_io_getwd(__VA_ARGS__)
#define lstat(...)	enclose_io_lstat(__VA_ARGS__)
#define readlink(...) enclose_io_readlink(__VA_ARGS__)
#define opendir(...)	enclose_io_opendir(__VA_ARGS__)
#define closedir(...)	enclose_io_closedir(__VA_ARGS__)
#define readdir(...)	enclose_io_readdir(__VA_ARGS__)
#define telldir(...)	enclose_io_telldir(__VA_ARGS__)
#define seekdir(...)	enclose_io_seekdir(__VA_ARGS__)
#define rewinddir(...)	enclose_io_rewinddir(__VA_ARGS__)
#define dirfd(...) enclose_io_dirfd(__VA_ARGS__)
#define scandir(...)	enclose_io_scandir(__VA_ARGS__)

#endif // !_WIN32

#endif //!__cplusplus
#endif
