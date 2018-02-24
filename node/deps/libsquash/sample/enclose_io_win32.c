/*
 * Copyright (c) 2017 Minqi Pan <pmq2001@gmail.com>
 *                    Shengyuan Liu <sounder.liu@gmail.com>
 *
 * This file is part of libsquash, distributed under the MIT License
 * For full terms see the included LICENSE file
 */

#include "enclose_io_common.h"

#ifdef _WIN32

extern char *mkdir_workdir;
extern char *enclose_io_mkdir_scope;

int enclose_io__open(const char *pathname, int flags)
{
	if (enclose_io_cwd[0] && '/' != *pathname) {
		sqfs_path enclose_io_expanded;
		size_t enclose_io_cwd_len;
		size_t memcpy_len;
		ENCLOSE_IO_GEN_EXPANDED_NAME(pathname);
		ENCLOSE_IO_CONSIDER_MKDIR_WORKDIR_RETURN(
			enclose_io_expanded,
			enclose_io_dos_return(squash_open(enclose_io_fs, enclose_io_expanded)),
			_open(mkdir_workdir_expanded, flags)
		);
	} else if (enclose_io_is_path(pathname)) {
		ENCLOSE_IO_CONSIDER_MKDIR_WORKDIR_RETURN(
			pathname,
			enclose_io_dos_return(squash_open(enclose_io_fs, pathname)),
			_open(mkdir_workdir_expanded, flags)
		);
	} else {
                return _open(pathname, flags);
	}
}

int enclose_io__wopen(const wchar_t *pathname, int flags, int mode)
{
	if (enclose_io_cwd[0] && enclose_io_is_relative_w(pathname)) {
		sqfs_path enclose_io_expanded;
		size_t enclose_io_cwd_len;
		size_t memcpy_len;
		sqfs_path enclose_io_converted_storage;
		char *enclose_io_converted;
		char *enclose_io_i;
		size_t enclose_io_converted_length;

		W_ENCLOSE_IO_PATH_CONVERT(pathname);
		ENCLOSE_IO_GEN_EXPANDED_NAME(enclose_io_converted);
		ENCLOSE_IO_CONSIDER_MKDIR_WORKDIR_RETURN(
			enclose_io_expanded,
			enclose_io_dos_return(squash_open(enclose_io_fs, enclose_io_expanded)),
			_open(mkdir_workdir_expanded, flags, mode)
		);
	} else if (enclose_io_is_path_w(pathname)) {
		sqfs_path enclose_io_converted_storage;
		char *enclose_io_converted;
		char *enclose_io_i;
		size_t enclose_io_converted_length;

		W_ENCLOSE_IO_PATH_CONVERT(pathname);
		ENCLOSE_IO_CONSIDER_MKDIR_WORKDIR_RETURN(
			enclose_io_converted,
			enclose_io_dos_return(squash_open(enclose_io_fs, enclose_io_converted)),
			_open(mkdir_workdir_expanded, flags, mode)
		);
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
	if (enclose_io_is_path_w(path)) {
		sqfs_path enclose_io_converted_storage;
		char *enclose_io_converted;
		char *enclose_io_i;
		size_t enclose_io_converted_length;
		struct stat st;
		int ret;

		W_ENCLOSE_IO_PATH_CONVERT(path);

		if (mkdir_workdir) {
			sqfs_path mkdir_workdir_expanded;
			char *mkdir_workdir_expanded_head;
			size_t mkdir_workdir_len;
			size_t memcpy_len;
			struct stat mkdir_workdir_buf;
			mkdir_workdir_len = strlen(mkdir_workdir);
			memcpy(mkdir_workdir_expanded, mkdir_workdir, mkdir_workdir_len);
			memcpy_len = strlen(enclose_io_converted);
			if (SQUASHFS_PATH_LEN - mkdir_workdir_len < memcpy_len) {
				memcpy_len = SQUASHFS_PATH_LEN - mkdir_workdir_len;
			}
			memcpy(&mkdir_workdir_expanded[mkdir_workdir_len], (enclose_io_converted), memcpy_len);
			mkdir_workdir_expanded[mkdir_workdir_len + memcpy_len] = '\0';
			mkdir_workdir_expanded_head = strstr(mkdir_workdir_expanded, enclose_io_mkdir_scope);
			if (mkdir_workdir_expanded_head && '/' == mkdir_workdir_expanded_head[strlen(enclose_io_mkdir_scope)]) {
				memmove(
					mkdir_workdir_expanded_head,
					mkdir_workdir_expanded_head + strlen(enclose_io_mkdir_scope),
					strlen(mkdir_workdir_expanded_head + strlen(enclose_io_mkdir_scope)) + 1
				);
				if (0 == stat(mkdir_workdir_expanded, &mkdir_workdir_buf)) {
					int ret = _chdir(mkdir_workdir_expanded);
					if (0 == ret) {
						enclose_io_chdir_helper(enclose_io_converted);
					}
					return ret;
				}
			}
		}

		ret = squash_stat(enclose_io_fs, enclose_io_converted, &st);
		if (-1 == ret) {
			ENCLOSE_IO_SET_LAST_ERROR;
			return -1;
		}
		if (S_ISDIR(st.st_mode)) {
			enclose_io_chdir_helper(enclose_io_converted);
			return 0;
		} else {
			errno = ENOENT;
			ENCLOSE_IO_SET_LAST_ERROR;
			return -1;
		}
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
        wchar_t tempbuf[SQUASHFS_PATH_LEN + 1];
        size_t retlen;
	if (enclose_io_cwd[0]) {
		retlen = mbstowcs(tempbuf, enclose_io_cwd, SQUASHFS_PATH_LEN);
		if ((size_t)-1 == retlen) {
                        errno = ERANGE;
                        ENCLOSE_IO_SET_LAST_ERROR;
			return NULL;
		}
                tempbuf[retlen] = L'\0';
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
                        errno = EINVAL;
                        ENCLOSE_IO_SET_LAST_ERROR;
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
		return enclose_io_dos_return(squash_lseek(fildes, offset, whence));
	} else {
		return _lseeki64(fildes, offset, whence);
	}
}

static HANDLE EncloseIOCreateFileWHelper(
	char * incoming,
	DWORD dwFlagsAndAttributes)
{
	int ret;
	struct stat buf;
	SQUASH_DIR *dirp;
	int follow_link;

	if (dwFlagsAndAttributes & FILE_FLAG_OPEN_REPARSE_POINT) {
		ret = squash_lstat(enclose_io_fs, incoming, &buf);
		follow_link = 0;
	} else {
		ret = squash_stat(enclose_io_fs, incoming, &buf);
		follow_link = 1;
	}

	if (-1 == ret) {
		ENCLOSE_IO_SET_LAST_ERROR;
		return INVALID_HANDLE_VALUE;
	}
	if (S_ISDIR(buf.st_mode)) {
		dirp = squash_opendir_inner(enclose_io_fs, incoming, follow_link);
		if (NULL != dirp) {
			return (HANDLE)(dirp);
		} else {
			ENCLOSE_IO_SET_LAST_ERROR;
			return INVALID_HANDLE_VALUE;
		}
	} else {
		ret = squash_open_inner(enclose_io_fs, incoming, follow_link);
		if (ret >= 0) {
			return (HANDLE)(squash_global_fdtable.fds[ret]->payload);
		} else {
			ENCLOSE_IO_SET_LAST_ERROR;
			return INVALID_HANDLE_VALUE;
		}
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
	if (enclose_io_cwd[0] && enclose_io_is_relative_w(lpFileName)) {
		sqfs_path enclose_io_expanded;
		size_t enclose_io_cwd_len;
		size_t memcpy_len;
		sqfs_path enclose_io_converted_storage;
		char *enclose_io_converted;
		char *enclose_io_i;
		size_t enclose_io_converted_length;

		W_ENCLOSE_IO_PATH_CONVERT(lpFileName);
		ENCLOSE_IO_GEN_EXPANDED_NAME(enclose_io_converted);
		if (OPEN_EXISTING == dwCreationDisposition) {
			ENCLOSE_IO_CONSIDER_MKDIR_WORKDIR_RETURN(
				enclose_io_expanded,
				EncloseIOCreateFileWHelper(
					enclose_io_expanded,
					dwFlagsAndAttributes
				),
				CreateFile(
					mkdir_workdir_expanded,
					dwDesiredAccess,
					dwShareMode,
					lpSecurityAttributes,
					dwCreationDisposition,
					dwFlagsAndAttributes,
					hTemplateFile
				)
			);
		} else {
			char *dir_thereof_ptr = strrchr(enclose_io_expanded, '/');
			if (dir_thereof_ptr) {
				*dir_thereof_ptr = 0;
				enclose_io_mkdir(enclose_io_expanded, 0777);
				*dir_thereof_ptr = '/';
			}
			if (mkdir_workdir) {
				sqfs_path mkdir_workdir_expanded;
				char *mkdir_workdir_expanded_head;
				size_t mkdir_workdir_len;
				size_t memcpy_len;
				struct stat mkdir_workdir_buf;
				mkdir_workdir_len = strlen(mkdir_workdir);
				memcpy(mkdir_workdir_expanded, mkdir_workdir, mkdir_workdir_len);
				memcpy_len = strlen(enclose_io_expanded);
				if (SQUASHFS_PATH_LEN - mkdir_workdir_len < memcpy_len) {
					memcpy_len = SQUASHFS_PATH_LEN - mkdir_workdir_len;
				}
				memcpy(&mkdir_workdir_expanded[mkdir_workdir_len], enclose_io_expanded, memcpy_len);
				mkdir_workdir_expanded[mkdir_workdir_len + memcpy_len] = '\0';
				mkdir_workdir_expanded_head = strstr(mkdir_workdir_expanded, enclose_io_mkdir_scope);
				if (mkdir_workdir_expanded_head && '/' == mkdir_workdir_expanded_head[strlen(enclose_io_mkdir_scope)]) {
					memmove(
						mkdir_workdir_expanded_head,
						mkdir_workdir_expanded_head + strlen(enclose_io_mkdir_scope),
						strlen(mkdir_workdir_expanded_head + strlen(enclose_io_mkdir_scope)) + 1
					);
					return CreateFile(
						mkdir_workdir_expanded,
						dwDesiredAccess,
						dwShareMode,
						lpSecurityAttributes,
						dwCreationDisposition,
						dwFlagsAndAttributes,
						hTemplateFile
					);
				} else {
					errno = ENOENT;
					return enclose_io_dos_return(-1);
				}
			} else {
				errno = ENOENT;
				return enclose_io_dos_return(-1);
			}
		}
	} else if (enclose_io_is_path_w(lpFileName)) {
		sqfs_path enclose_io_converted_storage;
		char *enclose_io_converted;
		char *enclose_io_i;
		size_t enclose_io_converted_length;

		W_ENCLOSE_IO_PATH_CONVERT(lpFileName);
		
		if (OPEN_EXISTING == dwCreationDisposition) {
			ENCLOSE_IO_CONSIDER_MKDIR_WORKDIR_RETURN(
				enclose_io_converted,
				EncloseIOCreateFileWHelper(
					enclose_io_converted,
					dwFlagsAndAttributes
				),
				CreateFile(
					mkdir_workdir_expanded,
					dwDesiredAccess,
					dwShareMode,
					lpSecurityAttributes,
					dwCreationDisposition,
					dwFlagsAndAttributes,
					hTemplateFile
				)
			);
		} else {
			char *dir_thereof_ptr = strrchr(enclose_io_converted, '/');
			if (dir_thereof_ptr) {
				*dir_thereof_ptr = 0;
				enclose_io_mkdir(enclose_io_converted, 0777);
				*dir_thereof_ptr = '/';
			}
			if (mkdir_workdir) {
				sqfs_path mkdir_workdir_expanded;
				char *mkdir_workdir_expanded_head;
				size_t mkdir_workdir_len;
				size_t memcpy_len;
				struct stat mkdir_workdir_buf;
				mkdir_workdir_len = strlen(mkdir_workdir);
				memcpy(mkdir_workdir_expanded, mkdir_workdir, mkdir_workdir_len);
				memcpy_len = strlen(enclose_io_converted);
				if (SQUASHFS_PATH_LEN - mkdir_workdir_len < memcpy_len) {
					memcpy_len = SQUASHFS_PATH_LEN - mkdir_workdir_len;
				}
				memcpy(&mkdir_workdir_expanded[mkdir_workdir_len], enclose_io_converted, memcpy_len);
				mkdir_workdir_expanded[mkdir_workdir_len + memcpy_len] = '\0';
				mkdir_workdir_expanded_head = strstr(mkdir_workdir_expanded, enclose_io_mkdir_scope);
				if (mkdir_workdir_expanded_head && '/' == mkdir_workdir_expanded_head[strlen(enclose_io_mkdir_scope)]) {
					memmove(
						mkdir_workdir_expanded_head,
						mkdir_workdir_expanded_head + strlen(enclose_io_mkdir_scope),
						strlen(mkdir_workdir_expanded_head + strlen(enclose_io_mkdir_scope)) + 1
					);
					return CreateFile(
						mkdir_workdir_expanded,
						dwDesiredAccess,
						dwShareMode,
						lpSecurityAttributes,
						dwCreationDisposition,
						dwFlagsAndAttributes,
						hTemplateFile
					);
				} else {
					errno = ENOENT;
					return enclose_io_dos_return(-1);
				}
			} else {
				errno = ENOENT;
				return enclose_io_dos_return(-1);
			}
		}
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
                int ret;
		if (S_ISDIR(sqf->st.st_mode)) {
			ret = squash_closedir((SQUASH_DIR *)hObject);
                        if (-1 == ret) {
                                ENCLOSE_IO_SET_LAST_ERROR;
                                return FALSE;
                        }
			return TRUE;
		} else {
			ret = squash_close(*((int *)hObject));
                        if (-1 == ret) {
                                ENCLOSE_IO_SET_LAST_ERROR;
                                return FALSE;
                        }
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
		if (-1 == ret) {
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
EncloseIOGetFileAttributesWHelper(
	const char *path
)
{
	int ret;
	struct stat buf;
	ret = squash_stat(enclose_io_fs, path, &buf);
	if (-1 == ret) {
		ENCLOSE_IO_SET_LAST_ERROR;
		return INVALID_FILE_ATTRIBUTES;
	}
	return EncloseIOGetFileAttributesHelper(&buf);
}

DWORD
EncloseIOGetFileAttributesW(
    LPCWSTR lpFileName
)
{
	if (enclose_io_cwd[0] && enclose_io_is_relative_w(lpFileName)) {
		sqfs_path enclose_io_expanded;
		size_t enclose_io_cwd_len;
		size_t memcpy_len;
		sqfs_path enclose_io_converted_storage;
		char *enclose_io_converted;
		char *enclose_io_i;
		size_t enclose_io_converted_length;

		W_ENCLOSE_IO_PATH_CONVERT(lpFileName);
		ENCLOSE_IO_GEN_EXPANDED_NAME(enclose_io_converted);
		
		ENCLOSE_IO_CONSIDER_MKDIR_WORKDIR_RETURN(
			enclose_io_expanded,
			EncloseIOGetFileAttributesWHelper(enclose_io_expanded),
			GetFileAttributes(
				mkdir_workdir_expanded
			)
		);
	} else if (enclose_io_is_path_w(lpFileName)) {
		sqfs_path enclose_io_converted_storage;
		char *enclose_io_converted;
		char *enclose_io_i;
		size_t enclose_io_converted_length;

		W_ENCLOSE_IO_PATH_CONVERT(lpFileName);
		
		ENCLOSE_IO_CONSIDER_MKDIR_WORKDIR_RETURN(
			enclose_io_converted,
			EncloseIOGetFileAttributesWHelper(enclose_io_converted),
			GetFileAttributes(
				mkdir_workdir_expanded
			)
		);
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
EncloseIOGetFileAttributesExWHelper(
	const char *path,
	LPVOID lpFileInformation
)
{
	int ret;
	struct stat buf;
	WIN32_FILE_ATTRIBUTE_DATA *fa;

	ret = squash_stat(enclose_io_fs, path, &buf);
	if (-1 == ret) {
		ENCLOSE_IO_SET_LAST_ERROR;
		return 0;
	}
	fa = (WIN32_FILE_ATTRIBUTE_DATA *)lpFileInformation;
	EncloseIOFillWin32FileAttributeDataHelper(fa, &buf);
	return 1;
}

BOOL
EncloseIOGetFileAttributesExW(
    LPCWSTR lpFileName,
    GET_FILEEX_INFO_LEVELS fInfoLevelId,
    LPVOID lpFileInformation
)
{
	if (enclose_io_cwd[0] && enclose_io_is_relative_w(lpFileName)) {
		sqfs_path enclose_io_expanded;
		size_t enclose_io_cwd_len;
		size_t memcpy_len;
		sqfs_path enclose_io_converted_storage;
		char *enclose_io_converted;
		char *enclose_io_i;
		size_t enclose_io_converted_length;

		W_ENCLOSE_IO_PATH_CONVERT(lpFileName);
		ENCLOSE_IO_GEN_EXPANDED_NAME(enclose_io_converted);
		assert(GetFileExInfoStandard == fInfoLevelId);
		
		ENCLOSE_IO_CONSIDER_MKDIR_WORKDIR_RETURN(
			enclose_io_expanded,
			EncloseIOGetFileAttributesExWHelper(enclose_io_expanded, lpFileInformation),
			GetFileAttributesEx(
				mkdir_workdir_expanded,
				fInfoLevelId,
				lpFileInformation
			),
		);
	} else if (enclose_io_is_path_w(lpFileName)) {
		sqfs_path enclose_io_converted_storage;
		char *enclose_io_converted;
		char *enclose_io_i;
		size_t enclose_io_converted_length;

		W_ENCLOSE_IO_PATH_CONVERT(lpFileName);
		assert(GetFileExInfoStandard == fInfoLevelId);

		ENCLOSE_IO_CONSIDER_MKDIR_WORKDIR_RETURN(
			enclose_io_converted,
			EncloseIOGetFileAttributesExWHelper(enclose_io_converted, lpFileInformation),
			GetFileAttributesEx(
				mkdir_workdir_expanded,
				fInfoLevelId,
				lpFileInformation
			)
		);
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
        if (sqf) {
                struct stat st;
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
        size_t mbstowcs_size;

        while (parent >= incoming) {
                if ('/' == *parent) {
                        break;
                }
                parent -= 1;
        }
        *parent = '\0';
        dirp = squash_opendir(enclose_io_fs, incoming);
        if (NULL == dirp) {
                ENCLOSE_IO_SET_LAST_ERROR;
                return INVALID_HANDLE_VALUE;
        }
        dirp->payload = dup_incoming;
        current_path = (char *)malloc(strlen(dup_incoming) + SQUASHFS_PATH_LEN + 1);
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
                int ret = squash_closedir(dirp);
                assert(0 == ret);
                ENCLOSE_IO_SET_LAST_ERROR;
                return INVALID_HANDLE_VALUE;
        }
        mbstowcs_size = mbstowcs(lpFindFileData->cFileName, mydirent->d_name, sizeof(lpFindFileData->cFileName) / sizeof(lpFindFileData->cFileName[0]) - 1);
        if ((size_t)-1 == mbstowcs_size) {
                errno = EIO;
                ENCLOSE_IO_SET_LAST_ERROR;
                return 0;
        }
        lpFindFileData->cFileName[mbstowcs_size] = 0;
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
	if (enclose_io_cwd[0] && enclose_io_is_relative_w(lpFileName)) {
		sqfs_path enclose_io_expanded;
		size_t enclose_io_cwd_len;
		size_t memcpy_len;
		sqfs_path enclose_io_converted_storage;
		char *enclose_io_converted;
		char *enclose_io_i;
		size_t enclose_io_converted_length;

		W_ENCLOSE_IO_PATH_CONVERT(lpFileName);
		ENCLOSE_IO_GEN_EXPANDED_NAME(enclose_io_converted);
		
		ENCLOSE_IO_CONSIDER_MKDIR_WORKDIR_RETURN(
			enclose_io_expanded,
			EncloseIOFindFirstFileHelper(enclose_io_expanded, lpFindFileData),
			FindFirstFile(
				mkdir_workdir_expanded,
				lpFindFileData
			)
		);
	} else if (enclose_io_is_path_w(lpFileName)) {
		sqfs_path enclose_io_converted_storage;
		char *enclose_io_converted;
		char *enclose_io_i;
		size_t enclose_io_converted_length;

		W_ENCLOSE_IO_PATH_CONVERT(lpFileName);
		
		ENCLOSE_IO_CONSIDER_MKDIR_WORKDIR_RETURN(
			enclose_io_converted,
			EncloseIOFindFirstFileHelper(enclose_io_converted, lpFindFileData),
			FindFirstFile(
				mkdir_workdir_expanded,
				lpFindFileData
			)
		);
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

	if (sqf) {
                SQUASH_DIR *dirp;
                struct SQUASH_DIRENT *mydirent;
                char *current_path;
                char *current_path_tail;
                size_t mbstowcs_size;

                dirp = (SQUASH_DIR*)hFindFile;
                current_path = (char *)malloc(strlen((char *)(dirp->payload)) + SQUASHFS_PATH_LEN + 1);
                if (NULL == current_path) {
                        errno = ENOMEM;
                        ENCLOSE_IO_SET_LAST_ERROR;
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
                        errno = ENOENT;
                        SetLastError(ERROR_NO_MORE_FILES);
                        _doserrno = ERROR_NO_MORE_FILES;
                        return 0;
                }
                mbstowcs_size = mbstowcs(lpFindFileData->cFileName, mydirent->d_name, sizeof(lpFindFileData->cFileName) / sizeof(lpFindFileData->cFileName[0]) - 1);
                if ((size_t)-1 == mbstowcs_size) {
                        errno = EIO;
                        ENCLOSE_IO_SET_LAST_ERROR;
                        return 0;
                }
                lpFindFileData->cFileName[mbstowcs_size] = 0;
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
)
{
	struct squash_file *sqf = squash_find_entry((void *)hDevice);
	int ret;

	if (sqf) {
		char the_path[SQUASHFS_PATH_LEN + 1];
		wchar_t the_wpath[SQUASHFS_PATH_LEN + 1];
		struct stat st;
		size_t retlen;
		REPARSE_DATA_BUFFER* reparse_data = (REPARSE_DATA_BUFFER*)lpOutBuffer;

		// TODO handle the overlapped
		assert(NULL == lpOverlapped);

		// TODO support more than FSCTL_GET_REPARSE_POINT
		assert(0 == nInBufferSize);

		// FSCTL_GET_REPARSE_POINT
		// https://msdn.microsoft.com/en-us/library/windows/desktop/aa364571(v=vs.85).aspx
		assert(dwIoControlCode == FSCTL_GET_REPARSE_POINT);
		assert(NULL == lpInBuffer);
		st = sqf->st;
		if (!S_ISLNK(st.st_mode)) {
			errno = EINVAL;
			SetLastError(ERROR_NOT_A_REPARSE_POINT);
			_doserrno = ERROR_NOT_A_REPARSE_POINT;
			return FALSE;
		}
		ret = squash_readlink_inode(
			enclose_io_fs,
			&sqf->node,
			the_path,
			SQUASHFS_PATH_LEN
		);
		if (-1 == ret) {
			ENCLOSE_IO_SET_LAST_ERROR;
			return FALSE;
		}
                
		retlen = mbstowcs(the_wpath, the_path, SQUASHFS_PATH_LEN);
		if ((size_t)-1 == retlen) {
			errno = ENAMETOOLONG;
			ENCLOSE_IO_SET_LAST_ERROR;
			return FALSE;
		}
		the_wpath[retlen] = L'\0';
		*lpBytesReturned = sizeof(REPARSE_DATA_BUFFER) + retlen * sizeof(wchar_t);
		if (*lpBytesReturned > nOutBufferSize) {
			*lpBytesReturned = 0;
			errno = ENAMETOOLONG;
			ENCLOSE_IO_SET_LAST_ERROR;
			return FALSE;
		}
		reparse_data->ReparseTag = IO_REPARSE_TAG_SYMLINK;
		memcpy(reparse_data->SymbolicLinkReparseBuffer.PathBuffer, the_wpath, (retlen + 1) * sizeof(wchar_t));
		reparse_data->SymbolicLinkReparseBuffer.SubstituteNameLength = retlen * sizeof(wchar_t);
		reparse_data->SymbolicLinkReparseBuffer.SubstituteNameOffset = 0;
		return TRUE;
	} else {
		return DeviceIoControl(
			hDevice,
			dwIoControlCode,
			lpInBuffer,
			nInBufferSize,
			lpOutBuffer,
			nOutBufferSize,
			lpBytesReturned,
			lpOverlapped
		);
	}
}

HANDLE
EncloseIOCreateIoCompletionPort(
        HANDLE FileHandle,
        HANDLE ExistingCompletionPort,
        ULONG_PTR CompletionKey,
        DWORD NumberOfConcurrentThreads
)
{
	struct squash_file *sqf = squash_find_entry((void *)FileHandle);

	if (sqf) {
		// do nothing
		assert(NULL != ExistingCompletionPort);
		return ExistingCompletionPort;
	} else {
		return CreateIoCompletionPort(
			FileHandle,
			ExistingCompletionPort,
			CompletionKey,
			NumberOfConcurrentThreads
		);
	}
}

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
)
{
	struct squash_file *sqf = squash_find_entry((void *)hDirectory);

	if (sqf) {
		// do nothing
		assert(lpOverlapped);
		return TRUE;
	} else {
		return ReadDirectoryChangesW(
			hDirectory,
			lpBuffer,
			nBufferLength,
			bWatchSubtree,
			dwNotifyFilter,
			lpBytesReturned,
			lpOverlapped,
			lpCompletionRoutine
		);
	}
}

HMODULE
EncloseIOLoadLibraryExW(
	LPCWSTR lpLibFileName,
	HANDLE hFile,
	DWORD dwFlags
)
{
	if (enclose_io_cwd[0] && enclose_io_is_relative_w(lpLibFileName)) {
		sqfs_path enclose_io_expanded;
		size_t enclose_io_cwd_len;
		size_t memcpy_len;
		sqfs_path enclose_io_converted_storage;
		char *enclose_io_converted;
		char *enclose_io_i;
		size_t enclose_io_converted_length;

		W_ENCLOSE_IO_PATH_CONVERT(lpLibFileName);
		ENCLOSE_IO_GEN_EXPANDED_NAME(enclose_io_converted);
		
		ENCLOSE_IO_CONSIDER_MKDIR_WORKDIR_RETURN(
			enclose_io_expanded,
			LoadLibraryExW(
				squash_extract(enclose_io_fs, enclose_io_expanded, "dll"),
				hFile,
				dwFlags
			),
			LoadLibraryEx(
				mkdir_workdir_expanded,
				hFile,
				dwFlags
			)
		);
	}
	else if (enclose_io_is_path_w(lpLibFileName)) {
		sqfs_path enclose_io_converted_storage;
		char *enclose_io_converted;
		char *enclose_io_i;
		size_t enclose_io_converted_length;

		W_ENCLOSE_IO_PATH_CONVERT(lpLibFileName);
		
		ENCLOSE_IO_CONSIDER_MKDIR_WORKDIR_RETURN(
			enclose_io_converted,
			LoadLibraryExW(
				squash_extract(enclose_io_fs, enclose_io_converted, "dll"),
				hFile,
				dwFlags
			),
			LoadLibraryEx(
				mkdir_workdir_expanded,
				hFile,
				dwFlags
			)
		);
	}
	else {
		return LoadLibraryExW(
			lpLibFileName,
			hFile,
			dwFlags
		);
	}
}

BOOL
EncloseIOCreateProcessW(
	LPCWSTR lpApplicationName,
	LPWSTR lpCommandLine,
	LPSECURITY_ATTRIBUTES lpProcessAttributes,
	LPSECURITY_ATTRIBUTES lpThreadAttributes,
	BOOL bInheritHandles,
	DWORD dwCreationFlags,
	LPVOID lpEnvironment,
	LPCWSTR lpCurrentDirectory,
	LPSTARTUPINFOW lpStartupInfo,
	LPPROCESS_INFORMATION lpProcessInformation
)
{
	if (NULL != lpCurrentDirectory && enclose_io_is_path_w(lpCurrentDirectory)) {
		lpCurrentDirectory = NULL;
	}
	return CreateProcessW(
		lpApplicationName,
		lpCommandLine,
		lpProcessAttributes,
		lpThreadAttributes,
		bInheritHandles,
		dwCreationFlags,
		lpEnvironment,
		lpCurrentDirectory,
		lpStartupInfo,
		lpProcessInformation
	);
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
                        /* FileInformation points to a buffer to hold directory entries.
                         * It's important that this buffer can hold at least one entry, regardless
                         * of the length of the file names present in the enumerated directory.
                         * A file name is at most 256 WCHARs long.
                         * According to MSDN, the buffer must be aligned at an 8-byte boundary.
                         */
                        assert(Length > 256 * sizeof(wchar_t));
			FILE_DIRECTORY_INFORMATION *ret = (FILE_DIRECTORY_INFORMATION *)FileInformation;
			ret->NextEntryOffset = 0;
			retlen = mbstowcs(ret->FileName, mydirent->d_name, 255);
			if ((size_t)-1 == retlen) {
				return -1;
                        }
                        ret->FileName[retlen] = 0;
                        ret->FileNameLength = sizeof(ret->FileName[0]) * retlen;
			ret->FileAttributes = EncloseIODType2FileAttributes(mydirent->d_type);
                        IoStatusBlock->Information = (char*)(&ret->FileName[retlen]) - (char*)FileInformation + 1;
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
