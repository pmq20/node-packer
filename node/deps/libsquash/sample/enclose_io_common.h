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

#define __USE_XOPEN_EXTENDED

#include <stdlib.h>
#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <stdarg.h>
#include <assert.h>

#ifdef _WIN32
#include <direct.h>
#include <Shlwapi.h>
#else
#include <sys/param.h>
#include <sys/uio.h>
#include <unistd.h>
#include <ftw.h>
#include <dirent.h>
#include <dlfcn.h>
#endif

extern sqfs *enclose_io_fs;
extern sqfs_path enclose_io_cwd;
extern const uint8_t enclose_io_memfs[];

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

short enclose_io_is_path(char *pathname);
short enclose_io_is_path_w(wchar_t *pathname);
short enclose_io_is_relative_w(wchar_t *pathname);

#define ENCLOSE_IO_CONSIDER_MKDIR_WORKDIR_RETURN(PATH, RETURN1, RETURN2) \
	if (mkdir_workdir) { \
		sqfs_path mkdir_workdir_expanded; \
		size_t mkdir_workdir_len; \
		size_t memcpy_len; \
		struct stat mkdir_workdir_buf; \
		mkdir_workdir_len = strlen(mkdir_workdir); \
		memcpy(mkdir_workdir_expanded, mkdir_workdir, mkdir_workdir_len); \
		memcpy_len = strlen(PATH); \
		if (SQUASHFS_PATH_LEN - mkdir_workdir_len < memcpy_len) { \
			memcpy_len = SQUASHFS_PATH_LEN - mkdir_workdir_len; \
		} \
		memcpy(&mkdir_workdir_expanded[mkdir_workdir_len], (PATH), memcpy_len); \
		mkdir_workdir_expanded[mkdir_workdir_len + memcpy_len] = '\0'; \
		if (0 == stat(mkdir_workdir_expanded, &mkdir_workdir_buf)) { \
			return(RETURN2); \
		} \
	} \
	return(RETURN1)

#define ENCLOSE_IO_GEN_EXPANDED_NAME(path)	\
			enclose_io_cwd_len = strlen(enclose_io_cwd); \
			memcpy(enclose_io_expanded, enclose_io_cwd, enclose_io_cwd_len); \
			memcpy_len = strlen(path); \
			if (SQUASHFS_PATH_LEN - enclose_io_cwd_len < memcpy_len) { memcpy_len = SQUASHFS_PATH_LEN - enclose_io_cwd_len; } \
			memcpy(&enclose_io_expanded[enclose_io_cwd_len], (path), memcpy_len); \
			enclose_io_expanded[enclose_io_cwd_len + memcpy_len] = '\0'

//TODO maybe use WideCharToMultiByte
#define W_ENCLOSE_IO_PATH_CONVERT(path) \
			enclose_io_converted = (char *)enclose_io_converted_storage; \
			enclose_io_converted_length = wcstombs(enclose_io_converted_storage, (path), SQUASHFS_PATH_LEN); \
                        if ((size_t)-1 == enclose_io_converted_length) { enclose_io_converted_length = 0; } \
                        enclose_io_converted[enclose_io_converted_length] = '\0'; \
			if (strnlen(enclose_io_converted_storage, 4) >= 4 && (0 == strncmp(enclose_io_converted_storage, "\\\\?\\", 4) || 0 == strncmp(enclose_io_converted_storage, "//?/", 4))) { \
				if (strnlen(enclose_io_converted_storage, 6) >= 6 && ':' == enclose_io_converted_storage[5]) { \
					enclose_io_converted += 6; \
				} else { \
					enclose_io_converted += 4; \
				} \
			} else if (strnlen(enclose_io_converted_storage, 3) >= 3 && (0 == strncmp(enclose_io_converted_storage + 1, ":\\", 2) || 0 == strncmp(enclose_io_converted_storage + 1, ":/", 2))) { \
				enclose_io_converted += 2; \
			} \
			for (enclose_io_i = enclose_io_converted; *enclose_io_i; enclose_io_i++) { \
				if ('\\' == *enclose_io_i) { *enclose_io_i = '/'; } \
			}

#define ENCLOSE_IO_SET_LAST_ERROR do { \
			if (ENOMEM == errno) { \
				SetLastError(ERROR_NOT_ENOUGH_MEMORY); \
                                _doserrno = ERROR_NOT_ENOUGH_MEMORY; \
			} else if (ENOENT == errno) { \
				SetLastError(ERROR_FILE_NOT_FOUND); \
                                _doserrno = ERROR_FILE_NOT_FOUND; \
			} else if (EBADF == errno) { \
				SetLastError(ERROR_INVALID_HANDLE); \
                                _doserrno = ERROR_INVALID_HANDLE; \
			} else if (ENAMETOOLONG == errno) { \
				SetLastError(ERROR_BUFFER_OVERFLOW); \
                                _doserrno = ERROR_BUFFER_OVERFLOW; \
			} else { \
				SetLastError(ERROR_INVALID_FUNCTION); \
                                _doserrno = ERROR_INVALID_FUNCTION; \
			} \
		} while (0)

int enclose_io_dos_return(int statement);
short enclose_io_if(const char* path);
SQUASH_OS_PATH enclose_io_ifextract(const char* path, const char* ext_name);
void enclose_io_chdir_helper(const char *path);
int enclose_io_chdir(const char *path);
char *enclose_io_getcwd(char *buf, size_t size);
char *enclose_io_getwd(char *buf);
int enclose_io_stat(const char *path, struct stat *buf);
int enclose_io_fstat(int fildes, struct stat *buf);
int enclose_io_open(int nargs, const char *pathname, int flags, ...);
int enclose_io_close(int fildes);
ssize_t enclose_io_read(int fildes, void *buf, size_t nbyte);
off_t enclose_io_lseek(int fildes, off_t offset, int whence);

#ifdef _WIN32

#include "enclose_io_winapi.h"

int enclose_io__open(const char *pathname, int flags);
int enclose_io__wopen(const wchar_t *pathname, int flags, int mode);
int enclose_io_open_osfhandle(intptr_t osfhandle, int flags);
intptr_t enclose_io_get_osfhandle(int fd);
int enclose_io_wchdir(const wchar_t *path);
wchar_t *enclose_io_wgetcwd(wchar_t *buf, size_t size);
int enclose_io_fstati64(int fildes, struct _stati64 *buf);
__int64 enclose_io_lseeki64(int fildes, __int64 offset, int whence);

HANDLE
EncloseIOCreateFileW(
	LPCWSTR lpFileName,
	DWORD dwDesiredAccess,
	DWORD dwShareMode,
	LPSECURITY_ATTRIBUTES lpSecurityAttributes,
	DWORD dwCreationDisposition,
	DWORD dwFlagsAndAttributes,
	HANDLE hTemplateFile
);

NTSTATUS
EncloseIOpNtQueryDirectoryFile(
	HANDLE FileHandle,
	HANDLE Event,
	PIO_APC_ROUTINE ApcRoutine,
	PVOID ApcContext,
	PIO_STATUS_BLOCK IoStatusBlock,
	PVOID FileInformation,
	ULONG Length,
	FILE_INFORMATION_CLASS FileInformationClass,
	BOOLEAN ReturnSingleEntry,
	PUNICODE_STRING FileName,
	BOOLEAN RestartScan
);

BOOL
EncloseIOCloseHandle(
	HANDLE hObject
);

DWORD
EncloseIOGetFileAttributesW(
    LPCWSTR lpFileName
);

BOOL
EncloseIOGetFileAttributesExW(
    LPCWSTR lpFileName,
    GET_FILEEX_INFO_LEVELS fInfoLevelId,
    LPVOID lpFileInformation
);

NTSTATUS
EncloseIOpNtQueryInformationFile(
	HANDLE FileHandle,
	PIO_STATUS_BLOCK IoStatusBlock,
	PVOID FileInformation,
	ULONG Length,
	FILE_INFORMATION_CLASS FileInformationClass);

NTSTATUS
EncloseIOpNtQueryVolumeInformationFile(
	HANDLE FileHandle,
	PIO_STATUS_BLOCK IoStatusBlock,
	PVOID FsInformation,
	ULONG Length,
	FS_INFORMATION_CLASS FsInformationClass);

BOOL
EncloseIOReadFile(
	HANDLE       hFile,
	LPVOID       lpBuffer,
	DWORD        nNumberOfBytesToRead,
	LPDWORD      lpNumberOfBytesRead,
	LPOVERLAPPED lpOverlapped
);

BOOL
EncloseIOGetHandleInformation(
    HANDLE hObject,
    LPDWORD lpdwFlags
);

DWORD
EncloseIOGetFileType(
        HANDLE hFile
);

HANDLE
EncloseIOFindFirstFileW(
        LPCWSTR lpFileName,
        LPWIN32_FIND_DATAW lpFindFileData
);

BOOL
EncloseIOFindNextFileW(
        HANDLE hFindFile,
        LPWIN32_FIND_DATAW lpFindFileData
);

BOOL
EncloseIOFindClose(
        HANDLE hFindFile
);

BOOL
EncloseIODeviceIoControl(
        HANDLE hDevice,
        DWORD dwIoControlCode,
        LPVOID lpInBuffer,
        DWORD nInBufferSize,
        LPVOID lpOutBuffer,
        DWORD nOutBufferSize,
        LPDWORD lpBytesReturned,
        LPOVERLAPPED lpOverlapped
);

HANDLE
EncloseIOCreateIoCompletionPort(
        HANDLE FileHandle,
        HANDLE ExistingCompletionPort,
        ULONG_PTR CompletionKey,
        DWORD NumberOfConcurrentThreads
);

BOOL
EncloseIOReadDirectoryChangesW(
        HANDLE hDirectory,
        LPVOID lpBuffer,
        DWORD nBufferLength,
        BOOL bWatchSubtree,
        DWORD dwNotifyFilter,
        LPDWORD lpBytesReturned,
        LPOVERLAPPED lpOverlapped,
        LPOVERLAPPED_COMPLETION_ROUTINE lpCompletionRoutine
);

HMODULE
EncloseIOLoadLibraryExW(
	LPCWSTR lpLibFileName,
	HANDLE hFile,
	DWORD dwFlags
);

#else
int enclose_io_lstat(const char *path, struct stat *buf);
ssize_t enclose_io_readlink(const char *path, char *buf, size_t bufsize);
DIR * enclose_io_opendir(const char *filename);
int enclose_io_closedir(DIR *dirp);
struct SQUASH_DIRENT * enclose_io_readdir(DIR *dirp);
long enclose_io_telldir(DIR *dirp);
void enclose_io_seekdir(DIR *dirp, long loc);
void enclose_io_rewinddir(DIR *dirp);
int enclose_io_dirfd(DIR *dirp);
int enclose_io_scandir(const char *dirname, struct SQUASH_DIRENT ***namelist,
	int(*select)(const struct SQUASH_DIRENT *),
	int(*compar)(const struct SQUASH_DIRENT **, const struct SQUASH_DIRENT **));
ssize_t enclose_io_pread(int d, void *buf, size_t nbyte, off_t offset);
ssize_t enclose_io_readv(int d, const struct iovec *iov, int iovcnt);
void* enclose_io_dlopen(const char* path, int mode);
int enclose_io_access(const char *path, int mode);
int enclose_io_mkdir(const char *path, mode_t mode);

#endif // !_WIN32

#endif
