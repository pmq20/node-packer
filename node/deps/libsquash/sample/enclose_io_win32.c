/*
 * Copyright (c) 2017 Minqi Pan <pmq2001@gmail.com>
 *                    Shengyuan Liu <sounder.liu@gmail.com>
 *
 * This file is part of libsquash, distributed under the MIT License
 * For full terms see the included LICENSE file
 */

#include "enclose_io_common.h"

#ifdef _WIN32

int enclose_io__open(const char *pathname, int flags)
{
        sqfs_name enclose_io_expanded;
        size_t enclose_io_cwd_len;
        size_t memcpy_len;
        sqfs_name enclose_io_converted_storage;
        char *enclose_io_converted;
        char *enclose_io_i;
        size_t enclose_io_converted_length;

	if (enclose_io_cwd[0] && '/' != *pathname) {
		ENCLOSE_IO_GEN_EXPANDED_NAME(pathname);
		return squash_open(enclose_io_fs, enclose_io_expanded);
	} else if (enclose_io_is_path(pathname)) {
		return squash_open(enclose_io_fs, pathname);
	} else {
                return _open(pathname, flags);
	}
}

int enclose_io__wopen(const wchar_t *pathname, int flags, int mode)
{
        sqfs_name enclose_io_expanded;
        size_t enclose_io_cwd_len;
        size_t memcpy_len;
        sqfs_name enclose_io_converted_storage;
        char *enclose_io_converted;
        char *enclose_io_i;
        size_t enclose_io_converted_length;

	if (enclose_io_cwd[0] && enclose_io_is_relative_w(pathname)) {
		W_ENCLOSE_IO_PATH_CONVERT(pathname);
		ENCLOSE_IO_GEN_EXPANDED_NAME(enclose_io_converted);
		return squash_open(enclose_io_fs, enclose_io_expanded);
	} else if (enclose_io_is_path_w(pathname)) {
		W_ENCLOSE_IO_PATH_CONVERT(pathname);
		return squash_open(enclose_io_fs, enclose_io_converted);
	} else {
		return _wopen(pathname, flags, mode);
	}
}

int enclose_io_open_osfhandle(intptr_t osfhandle, int flags)
{
	struct squash_file *entry = squash_find_entry((void *)osfhandle);
	if (entry) {
		return entry->fd;
	} else {
		return _open_osfhandle(osfhandle, flags);
	}
}

intptr_t enclose_io_get_osfhandle(int fd)
{
	if (SQUASH_VALID_VFD(fd)) {
                assert(!(S_ISDIR(squash_global_fdtable.fds[fd]->st.st_mode)));
		return (intptr_t)(squash_global_fdtable.fds[fd]->payload);
	}
	else {
		return _get_osfhandle(fd);
	}
}

int enclose_io_wchdir(const wchar_t *path)
{
        sqfs_name enclose_io_converted_storage;
        char *enclose_io_converted;
        char *enclose_io_i;
        size_t enclose_io_converted_length;

	if (enclose_io_is_path_w(path)) {
		W_ENCLOSE_IO_PATH_CONVERT(path);
		return enclose_io_chdir_helper(enclose_io_converted);
	} else {
		int ret = _wchdir(path);
		if (0 == ret) {
			enclose_io_cwd[0] = '\0';
		}
		return ret;
	}
}

wchar_t *enclose_io_wgetcwd(wchar_t *buf, size_t size)
{
        wchar_t tempbuf[256 + 1];
        size_t retlen;
	if (enclose_io_cwd[0]) {
		retlen = mbstowcs(tempbuf, enclose_io_cwd, 256);
		if (-1 == retlen) {
			return NULL;
		}
		if (NULL == buf) {
			buf = malloc((retlen + 1) * sizeof(wchar_t));
			if (NULL == buf) {
				errno = ENOMEM;
				return NULL;
			}
		} else {
			if (size - 1 < retlen) {
				retlen = size - 1;
			}
		}
		memcpy(buf, tempbuf, retlen * sizeof(wchar_t));
		buf[retlen] = L'\0';
		return buf;
	} else {
		return _wgetcwd(buf, size);
	}
}

int enclose_io_fstati64(int fildes, struct _stati64 *buf)
{
        struct stat st;
	if (SQUASH_VALID_VFD(fildes)) {
		if (NULL == buf) {
			return -1;
		}
		st = SQUASH_VFD_FILE(fildes)->st;
		buf->st_dev = st.st_dev;
		buf->st_ino = st.st_ino;
		buf->st_mode = st.st_mode;
		buf->st_nlink = st.st_nlink;
		buf->st_uid = st.st_uid;
		buf->st_gid = st.st_gid;
		buf->st_rdev = st.st_rdev;
		buf->st_size = st.st_size;
		buf->st_atime = st.st_atime;
		buf->st_mtime = st.st_mtime;
		buf->st_ctime = st.st_ctime;
		return 0;
	} else {
		return _fstati64(fildes, buf);
	}
}

__int64 enclose_io_lseeki64(int fildes, __int64 offset, int whence)
{
	if (SQUASH_VALID_VFD(fildes)) {
		return squash_lseek(fildes, offset, whence);
	} else {
		return _lseeki64(fildes, offset, whence);
	}
}

static HANDLE EncloseIOCreateFileWHelper(char * incoming)
{
	int ret;
	struct stat buf;
        SQUASH_DIR *dirp;

	ret = squash_stat(enclose_io_fs, incoming, &buf);
	if (-1 == ret) {
		ENCLOSE_IO_SET_LAST_ERROR;
		return INVALID_HANDLE_VALUE;
	}
	if (S_ISDIR(buf.st_mode)) {
		dirp = squash_opendir(enclose_io_fs, incoming);
		assert(NULL != dirp);
                return (HANDLE)(dirp);
	} else {
		ret = squash_open(enclose_io_fs, incoming);
		assert(ret >= 0);
                return (HANDLE)(squash_global_fdtable.fds[ret]->payload);
	}
}

HANDLE
EncloseIOCreateFileW(
	LPCWSTR lpFileName,
	DWORD dwDesiredAccess,
	DWORD dwShareMode,
	LPSECURITY_ATTRIBUTES lpSecurityAttributes,
	DWORD dwCreationDisposition,
	DWORD dwFlagsAndAttributes,
	HANDLE hTemplateFile
)
{
        sqfs_name enclose_io_expanded;
        size_t enclose_io_cwd_len;
        size_t memcpy_len;
        sqfs_name enclose_io_converted_storage;
        char *enclose_io_converted;
        char *enclose_io_i;
        size_t enclose_io_converted_length;

	if (enclose_io_cwd[0] && enclose_io_is_relative_w(lpFileName)) {
		W_ENCLOSE_IO_PATH_CONVERT(lpFileName);
		ENCLOSE_IO_GEN_EXPANDED_NAME(enclose_io_converted);
		return EncloseIOCreateFileWHelper(enclose_io_expanded);
	} else if (enclose_io_is_path_w(lpFileName)) {
		W_ENCLOSE_IO_PATH_CONVERT(lpFileName);
		return EncloseIOCreateFileWHelper(enclose_io_converted);
	} else {
		return CreateFileW(
			lpFileName,
			dwDesiredAccess,
			dwShareMode,
			lpSecurityAttributes,
			dwCreationDisposition,
			dwFlagsAndAttributes,
			hTemplateFile
		);
	}
}

BOOL
EncloseIOCloseHandle(
	HANDLE hObject
)
{
	struct squash_file *sqf = squash_find_entry((void *)hObject);
	if (sqf) {
		if (S_ISDIR(sqf->st.st_mode)) {
			squash_closedir((SQUASH_DIR *)hObject);
			return TRUE;
		} else {
			squash_close(*((int *)hObject));
			return TRUE;
		}
	} else {
		return CloseHandle(
			hObject
		);
	}
}

BOOL
EncloseIOReadFile(
	HANDLE       hFile,
	LPVOID       lpBuffer,
	DWORD        nNumberOfBytesToRead,
	LPDWORD      lpNumberOfBytesRead,
	LPOVERLAPPED lpOverlapped
)
{
	struct squash_file *sqf = squash_find_entry((void *)hFile);
        int ret;

	if (sqf) {
		if (NULL != lpOverlapped) {
                        squash_lseek(sqf->fd, lpOverlapped->Offset, SQUASH_SEEK_SET);
                        assert(0 == lpOverlapped->OffsetHigh); // TODO support OffsetHigh
                }
		ret = squash_read(sqf->fd, lpBuffer, nNumberOfBytesToRead);
		if (-1 == ret)
		{
			ENCLOSE_IO_SET_LAST_ERROR;
			return FALSE;
		}
		*lpNumberOfBytesRead = ret;
		return TRUE;
	} else {
		return ReadFile(
			hFile,
			lpBuffer,
			nNumberOfBytesToRead,
			lpNumberOfBytesRead,
			lpOverlapped
		);
	}
}

static DWORD EncloseIOGetFileAttributesHelper(struct stat *st)
{
	DWORD fa = 0; //FILE_ATTRIBUTE_READONLY;
	if (S_ISCHR(st->st_mode)) {
		fa |= FILE_ATTRIBUTE_DEVICE;
	} else if (S_ISLNK(st->st_mode)) {
		fa |= FILE_ATTRIBUTE_REPARSE_POINT;
	} else if (S_ISDIR(st->st_mode)) {
		fa |= FILE_ATTRIBUTE_DIRECTORY;
	} else {
		fa |= FILE_ATTRIBUTE_NORMAL;
	}
	return fa;
}

DWORD
EncloseIOGetFileAttributesW(
    LPCWSTR lpFileName
)
{
        sqfs_name enclose_io_expanded;
        size_t enclose_io_cwd_len;
        size_t memcpy_len;
        sqfs_name enclose_io_converted_storage;
        char *enclose_io_converted;
        char *enclose_io_i;
        size_t enclose_io_converted_length;
        int ret;
        struct stat buf;

	if (enclose_io_cwd[0] && enclose_io_is_relative_w(lpFileName)) {
		W_ENCLOSE_IO_PATH_CONVERT(lpFileName);
		ENCLOSE_IO_GEN_EXPANDED_NAME(enclose_io_converted);
		ret = squash_stat(enclose_io_fs, enclose_io_expanded, &buf);
		if (-1 == ret) {
			ENCLOSE_IO_SET_LAST_ERROR;
			return INVALID_FILE_ATTRIBUTES;
		}
		return EncloseIOGetFileAttributesHelper(&buf);
	} else if (enclose_io_is_path_w(lpFileName)) {
		W_ENCLOSE_IO_PATH_CONVERT(lpFileName);
		ret = squash_stat(enclose_io_fs, enclose_io_converted, &buf);
		if (-1 == ret) {
			ENCLOSE_IO_SET_LAST_ERROR;
			return INVALID_FILE_ATTRIBUTES;
		}		
		return EncloseIOGetFileAttributesHelper(&buf);
	} else {
		return GetFileAttributesW(
			lpFileName
		);
	}
}

static void EncloseIOUnixtimeToFiletime(time_t time, FILETIME *ft)
{
    ULARGE_INTEGER tmp;
    tmp.QuadPart = ((long long)time + (long long)((1970-1601)*365.2425) * 24 * 60 * 60) * 10 * 1000 * 1000;
    ft->dwLowDateTime = tmp.LowPart;
    ft->dwHighDateTime = tmp.HighPart;
}

static void EncloseIOFillWin32FileAttributeDataHelper(WIN32_FILE_ATTRIBUTE_DATA *fa, struct stat *st)
{
        fa->dwFileAttributes = EncloseIOGetFileAttributesHelper(st);
        EncloseIOUnixtimeToFiletime(st->st_atime, &fa->ftLastAccessTime);
        EncloseIOUnixtimeToFiletime(st->st_mtime, &fa->ftLastWriteTime);
        EncloseIOUnixtimeToFiletime(st->st_ctime, &fa->ftCreationTime);
        // TODO somehow support large files?
        fa->nFileSizeHigh = 0;
        fa->nFileSizeLow = st->st_size;
}

BOOL
EncloseIOGetFileAttributesExW(
    LPCWSTR lpFileName,
    GET_FILEEX_INFO_LEVELS fInfoLevelId,
    LPVOID lpFileInformation
)
{
        sqfs_name enclose_io_expanded;
        size_t enclose_io_cwd_len;
        size_t memcpy_len;
        sqfs_name enclose_io_converted_storage;
        char *enclose_io_converted;
        char *enclose_io_i;
        size_t enclose_io_converted_length;
        int ret;
        struct stat buf;
        WIN32_FILE_ATTRIBUTE_DATA *fa;

	if (enclose_io_cwd[0] && enclose_io_is_relative_w(lpFileName)) {
		W_ENCLOSE_IO_PATH_CONVERT(lpFileName);
		ENCLOSE_IO_GEN_EXPANDED_NAME(enclose_io_converted);
		assert(GetFileExInfoStandard == fInfoLevelId);
		ret = squash_stat(enclose_io_fs, enclose_io_expanded, &buf);
		if (-1 == ret) {
			ENCLOSE_IO_SET_LAST_ERROR;
			return 0;
		}
		fa = (WIN32_FILE_ATTRIBUTE_DATA *)lpFileInformation;
                EncloseIOFillWin32FileAttributeDataHelper(fa, &buf);
		return 1;
	} else if (enclose_io_is_path_w(lpFileName)) {
		W_ENCLOSE_IO_PATH_CONVERT(lpFileName);
		assert(GetFileExInfoStandard == fInfoLevelId);
		ret = squash_stat(enclose_io_fs, enclose_io_converted, &buf);
		if (-1 == ret) {
			ENCLOSE_IO_SET_LAST_ERROR;
			return 0;
		}
		fa = (WIN32_FILE_ATTRIBUTE_DATA *)lpFileInformation;
                EncloseIOFillWin32FileAttributeDataHelper(fa, &buf);
		return 1;
	} else {
		return GetFileAttributesExW(
			lpFileName,
			fInfoLevelId,
			lpFileInformation
		);
	}
}

BOOL
EncloseIOGetHandleInformation(
    HANDLE hObject,
    LPDWORD lpdwFlags
)
{
	struct squash_file *sqf = squash_find_entry((void *)hObject);
        if (sqf) {
                *lpdwFlags = 0;
                return 1;
        } else {
                return GetHandleInformation(
                        hObject,
                        lpdwFlags
                );
        }
}

DWORD
EncloseIOGetFileType(
        HANDLE hFile
)
{
	struct squash_file *sqf = squash_find_entry((void *)hFile);
        struct stat st;
        if (sqf) {
		st = sqf->st;
                if (S_ISCHR(st.st_mode)) {
                        return FILE_TYPE_CHAR;
                } else if (S_ISREG(st.st_mode)) {
                        return FILE_TYPE_DISK;
                } else if (S_ISFIFO(st.st_mode)) {
                        return FILE_TYPE_PIPE;
                } else {
                        return FILE_TYPE_UNKNOWN;
                }
        } else {
                return GetFileType(
                        hFile
                );
        }
}

DWORD EncloseIODType2FileAttributes(int d_type)
{
        DWORD x = 0; //FILE_ATTRIBUTE_READONLY;
        if (DT_CHR == d_type) {
                x |= FILE_ATTRIBUTE_DEVICE;
        } else if (DT_LNK == d_type) {
                x |= FILE_ATTRIBUTE_REPARSE_POINT;
        } else if (DT_DIR == d_type) {
                x |= FILE_ATTRIBUTE_DIRECTORY;
        } else {
                x |= FILE_ATTRIBUTE_NORMAL;
        }
        return x;
}

HANDLE
EncloseIOFindFirstFileHelper(
        char *incoming,
        LPWIN32_FIND_DATAW lpFindFileData
)
{
        char *dup_incoming = strdup(incoming);
        char *parent = incoming + strlen(incoming);
        SQUASH_DIR *dirp;
        struct SQUASH_DIRENT *mydirent;
        char *current_path_tail;
        char *current_path;

        while (parent >= incoming) {
                if ('/' == *parent) {
                        break;
                }
                parent -= 1;
        }
        *parent = '\0';
        dirp = squash_opendir(enclose_io_fs, incoming);
        dirp->payload = dup_incoming;
        if (NULL == dirp) {
                ENCLOSE_IO_SET_LAST_ERROR;
                return INVALID_HANDLE_VALUE;
        }
        current_path = (char *)malloc(strlen(dup_incoming) + SQUASHFS_NAME_LEN + 1);
        if (NULL == current_path) {
                SetLastError(ERROR_NOT_ENOUGH_MEMORY);
                return INVALID_HANDLE_VALUE;
        }
        memcpy(current_path, incoming, strlen(incoming) + 1);
        strcat(current_path, "/");
        current_path_tail = current_path + strlen(current_path);
        do {
        	mydirent = squash_readdir(dirp);
                if (NULL == mydirent) {
                        break;
                }
                memcpy(current_path_tail, mydirent->d_name, strlen(mydirent->d_name) + 1);
        } while (!PathMatchSpecA(current_path, dup_incoming));
        free(current_path);
        if (NULL == mydirent) {
                squash_closedir(dirp);
                ENCLOSE_IO_SET_LAST_ERROR;
                return INVALID_HANDLE_VALUE;
        }
        mbstowcs(lpFindFileData->cFileName, mydirent->d_name, sizeof(lpFindFileData->cFileName));
        lpFindFileData->cAlternateFileName[0] = 0;
        lpFindFileData->dwFileAttributes = EncloseIODType2FileAttributes(mydirent->d_type);
        return (HANDLE)(dirp);
}

HANDLE
EncloseIOFindFirstFileW(
        LPCWSTR lpFileName,
        LPWIN32_FIND_DATAW lpFindFileData
)
{
        sqfs_name enclose_io_expanded;
        size_t enclose_io_cwd_len;
        size_t memcpy_len;
        sqfs_name enclose_io_converted_storage;
        char *enclose_io_converted;
        char *enclose_io_i;
        size_t enclose_io_converted_length;

	if (enclose_io_cwd[0] && enclose_io_is_relative_w(lpFileName)) {
		W_ENCLOSE_IO_PATH_CONVERT(lpFileName);
		ENCLOSE_IO_GEN_EXPANDED_NAME(enclose_io_converted);
		return EncloseIOFindFirstFileHelper(enclose_io_expanded, lpFindFileData);
	} else if (enclose_io_is_path_w(lpFileName)) {
		W_ENCLOSE_IO_PATH_CONVERT(lpFileName);
		return EncloseIOFindFirstFileHelper(enclose_io_converted, lpFindFileData);
	} else {
                return FindFirstFileW(
                        lpFileName,
                        lpFindFileData
                );
	}
}

BOOL
EncloseIOFindNextFileW(
        HANDLE hFindFile,
        LPWIN32_FIND_DATAW lpFindFileData
)
{
	struct squash_file *sqf = squash_find_entry((void *)hFindFile);
        SQUASH_DIR *dirp;
        struct SQUASH_DIRENT *mydirent;
        char *current_path;
        char *current_path_tail;

	if (sqf) {
                dirp = (SQUASH_DIR*)hFindFile;
                current_path = (char *)malloc(strlen((char *)(dirp->payload)) + SQUASHFS_NAME_LEN + 1);
                if (NULL == current_path) {
                        SetLastError(ERROR_NOT_ENOUGH_MEMORY);
                        return 0;
                }
                memcpy(current_path, dirp->filename, strlen(dirp->filename) + 1);
                strcat(current_path, "/");
                current_path_tail = current_path + strlen(current_path);
                do {
                        mydirent = squash_readdir(dirp);
                        if (NULL == mydirent) {
                                break;
                        }
                        memcpy(current_path_tail, mydirent->d_name, strlen(mydirent->d_name) + 1);
                } while (!PathMatchSpecA(current_path, (char *)(dirp->payload)));
                free(current_path);
                if (NULL == mydirent) {
                        SetLastError(ERROR_NO_MORE_FILES);
                        return 0;
                }
                mbstowcs(lpFindFileData->cFileName, mydirent->d_name, sizeof(lpFindFileData->cFileName));
                lpFindFileData->cAlternateFileName[0] = 0;
                lpFindFileData->dwFileAttributes = EncloseIODType2FileAttributes(mydirent->d_type);
                return 1;
        } else {
                return FindNextFileW(
                        hFindFile,
                        lpFindFileData
                );
        }
}

BOOL
EncloseIOFindClose(
        HANDLE hFindFile
)
{
	struct squash_file *sqf = squash_find_entry((void *)hFindFile);
        int ret;

	if (sqf) {
                ret = squash_closedir((SQUASH_DIR *)hFindFile);
                if (0 == ret) {
                        return 1;
                } else {
                        ENCLOSE_IO_SET_LAST_ERROR;
                        return 0;
                }
        } else {
                return FindClose(
                        hFindFile
                );
        }
}

#ifndef RUBY_EXPORT
NTSTATUS
EncloseIOpNtQueryInformationFile(
	HANDLE FileHandle,
	PIO_STATUS_BLOCK IoStatusBlock,
	PVOID FileInformation,
	ULONG Length,
	FILE_INFORMATION_CLASS FileInformationClass)
{
	struct squash_file *sqf = squash_find_entry((void *)FileHandle);
        struct stat st;
	if (sqf) {
		st = sqf->st;
		IoStatusBlock->Status = STATUS_NOT_IMPLEMENTED;
		FILE_ALL_INFORMATION *file_info = (FILE_ALL_INFORMATION *)FileInformation;
		file_info->BasicInformation.FileAttributes = EncloseIOGetFileAttributesHelper(&st);
		file_info->StandardInformation.EndOfFile.QuadPart = st.st_size;

		file_info->BasicInformation.LastAccessTime.QuadPart = st.st_atime * 10000000ULL + 116444736000000000ULL;
		file_info->BasicInformation.ChangeTime.QuadPart = st.st_mtime * 10000000ULL + 116444736000000000ULL;
		file_info->BasicInformation.LastWriteTime.QuadPart = st.st_mtime * 10000000ULL + 116444736000000000ULL;
		file_info->BasicInformation.CreationTime.QuadPart = st.st_ctime * 10000000ULL + 116444736000000000ULL;
		file_info->InternalInformation.IndexNumber.QuadPart = st.st_ino;
		file_info->StandardInformation.AllocationSize.QuadPart = st.st_size;
		file_info->StandardInformation.NumberOfLinks = st.st_nlink;

		return STATUS_SUCCESS;
	} else {
		return pNtQueryInformationFile(
			FileHandle,
			IoStatusBlock,
			FileInformation,
			Length,
			FileInformationClass
		);
	}
}

NTSTATUS
EncloseIOpNtQueryVolumeInformationFile(
	HANDLE FileHandle,
	PIO_STATUS_BLOCK IoStatusBlock,
	PVOID FsInformation,
	ULONG Length,
	FS_INFORMATION_CLASS FsInformationClass)
{
	if (squash_find_entry((void *)FileHandle)) {
		IoStatusBlock->Status = STATUS_NOT_IMPLEMENTED;
		return STATUS_SUCCESS;
	} else {
		return pNtQueryVolumeInformationFile(
			FileHandle,
			IoStatusBlock,
			FsInformation,
			Length,
			FsInformationClass);
	}
}

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
)
{
        SQUASH_DIR *handle;
        struct SQUASH_DIRENT *mydirent;
        size_t retlen;

	if (squash_find_entry((void *)FileHandle)) {
		handle = (SQUASH_DIR *)FileHandle;
		mydirent = squash_readdir(handle);
		if (NULL == mydirent) {
			return STATUS_NO_MORE_FILES;
		} else {
			FILE_DIRECTORY_INFORMATION *ret = (FILE_DIRECTORY_INFORMATION *)FileInformation;
			ret->NextEntryOffset = 0;
			retlen = mbstowcs(ret->FileName, mydirent->d_name, 256);
			if (retlen > 0) {
				ret->FileNameLength = sizeof(ret->FileName[0]) * retlen;
			} else {
				return -1;
			}
			ret->FileAttributes = EncloseIODType2FileAttributes(mydirent->d_type);
			return STATUS_SUCCESS;
		}
	} else {
		return pNtQueryDirectoryFile(
			FileHandle,
			Event,
			ApcRoutine,
			ApcContext,
			IoStatusBlock,
			FileInformation,
			Length,
			FileInformationClass,
			ReturnSingleEntry,
			FileName,
			RestartScan
		);
	}
}

#endif // !RUBY_EXPORT
#endif
