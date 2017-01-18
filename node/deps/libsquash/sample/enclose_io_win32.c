/*
 * Copyright (c) 2017 Minqi Pan <pmq2001@gmail.com>
 *                    Shengyuan Liu <sounder.liu@gmail.com>
 *
 * This file is part of libsquash, distributed under the MIT License
 * For full terms see the included LICENSE file
 */

#include "enclose_io_common.h"

#ifdef _WIN32

int enclose_io_wopen(int nargs, const wchar_t *pathname, int flags, ...)
{
	if (enclose_io_cwd[0] && W_IS_ENCLOSE_IO_RELATIVE(pathname)) {
		W_ENCLOSE_IO_PATH_CONVERT(pathname);
		ENCLOSE_IO_GEN_EXPANDED_NAME(enclose_io_converted);
		return squash_open(enclose_io_fs, enclose_io_expanded);
	} else if (W_IS_ENCLOSE_IO_PATH(pathname)) {
		W_ENCLOSE_IO_PATH_CONVERT(pathname);
		return squash_open(enclose_io_fs, enclose_io_converted);
	} else {
		if (2 == nargs) {
			return _wopen(pathname, flags);
		} else {
			assert(3 == nargs);
			va_list args;
			va_start(args, flags);
			int mode = va_arg(args, int);
			va_end(args);
			return _wopen(pathname, flags, mode);
		}
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
		return (intptr_t)(squash_global_fdtable.fds[fd]->payload);
	}
	else {
		return _get_osfhandle(fd);
	}
}

int enclose_io_wchdir(const wchar_t *path)
{
	if (W_IS_ENCLOSE_IO_PATH(path)) {
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
	if (enclose_io_cwd[0]) {
		wchar_t tempbuf[256 + 1];
		size_t retlen = mbstowcs(tempbuf, enclose_io_cwd, 256);
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
	if (SQUASH_VALID_VFD(fildes)) {
		if (NULL == buf) {
			return -1;
		}
		struct stat st = SQUASH_VFD_FILE(fildes)->st;
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
	ret = squash_stat(enclose_io_fs, incoming, &buf);
	if (-1 == ret) {
		ENCLOSE_IO_SET_LAST_ERROR;
		return INVALID_HANDLE_VALUE;
	}
	if (S_ISDIR(buf.st_mode)) {
		SQUASH_DIR *handle = squash_opendir(enclose_io_fs, incoming);
		assert(NULL != handle);
		return (void *)handle;
	}
	else {
		ret = squash_open(enclose_io_fs, incoming);
		assert(ret >= 0);
		// TODO free it
		int *handle = (int *)malloc(sizeof(int));
		if (NULL == handle) {
			SetLastError(ERROR_NOT_ENOUGH_MEMORY);
			return INVALID_HANDLE_VALUE;
		}
		*handle = ret;
		squash_global_fdtable.fds[*handle]->payload = (void *)handle;
		return (void *)handle;
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
	if (enclose_io_cwd[0] && W_IS_ENCLOSE_IO_RELATIVE(lpFileName)) {
		W_ENCLOSE_IO_PATH_CONVERT(lpFileName);
		ENCLOSE_IO_GEN_EXPANDED_NAME(enclose_io_converted);
		return EncloseIOCreateFileWHelper(enclose_io_expanded);
	} else if (W_IS_ENCLOSE_IO_PATH(lpFileName)) {
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
	if (squash_find_entry((void *)FileHandle)) {
		SQUASH_DIR *handle = (SQUASH_DIR *)FileHandle;
		struct dirent *mydirent = squash_readdir(handle);
		if (NULL == mydirent) {
			return STATUS_NO_MORE_FILES;
		} else {
			FILE_DIRECTORY_INFORMATION *ret = (FILE_DIRECTORY_INFORMATION *)FileInformation;
			ret->NextEntryOffset = 0;
			size_t retlen = mbstowcs(ret->FileName, mydirent->d_name, 256);
			if (retlen > 0) {
				ret->FileNameLength = sizeof(ret->FileName[0]) * retlen;
			} else {
				return -1;
			}
			ret->FileAttributes = FILE_ATTRIBUTE_READONLY;
			if (DT_CHR == mydirent->d_type) {
				ret->FileAttributes |= FILE_ATTRIBUTE_DEVICE;
			} else if (DT_LNK == mydirent->d_type) {
				ret->FileAttributes |= FILE_ATTRIBUTE_REPARSE_POINT;
			} else if (DT_DIR == mydirent->d_type) {
				ret->FileAttributes |= FILE_ATTRIBUTE_DIRECTORY;
			} else {
				ret->FileAttributes |= FILE_ATTRIBUTE_NORMAL;
			}
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
			free((int *)hObject);
			return TRUE;
		}
	} else {
		return CloseHandle(
			hObject
		);
	}
}

NTSTATUS
EncloseIOpNtQueryInformationFile(
	HANDLE FileHandle,
	PIO_STATUS_BLOCK IoStatusBlock,
	PVOID FileInformation,
	ULONG Length,
	FILE_INFORMATION_CLASS FileInformationClass)
{
	struct squash_file *sqf = squash_find_entry((void *)FileHandle);
	if (sqf) {
		struct stat st = sqf->st;
		IoStatusBlock->Status = STATUS_NOT_IMPLEMENTED;
		FILE_ALL_INFORMATION *file_info = (FILE_ALL_INFORMATION *)FileInformation;
		file_info->BasicInformation.FileAttributes = FILE_ATTRIBUTE_READONLY;
		if (S_ISCHR(st.st_mode)) {
			file_info->BasicInformation.FileAttributes |= FILE_ATTRIBUTE_DEVICE;
		} else if (S_ISLNK(st.st_mode)) {
			file_info->BasicInformation.FileAttributes |= FILE_ATTRIBUTE_REPARSE_POINT;
		} else if (S_ISDIR(st.st_mode)) {
			file_info->BasicInformation.FileAttributes |= FILE_ATTRIBUTE_DIRECTORY;
		} else {
			file_info->BasicInformation.FileAttributes |= FILE_ATTRIBUTE_NORMAL;
		}
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
	if (sqf) {
		// TODO the case of lpOverlapped
		assert(NULL == lpOverlapped);
		int ret = squash_read(sqf->fd, lpBuffer, nNumberOfBytesToRead);
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

#endif
