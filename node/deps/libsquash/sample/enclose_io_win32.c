/*
 * Copyright (c) 2017 Minqi Pan <pmq2001@gmail.com>
 *                    Shengyuan Liu <sounder.liu@gmail.com>
 *
 * This file is part of libsquash, distributed under the MIT License
 * For full terms see the included LICENSE file
 */

#include "enclose_io_common.h"

#ifdef _WIN32

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
	if (W_IS_ENCLOSE_IO_PATH(lpFileName)) {
		W_ENCLOSE_IO_PATH_CONVERT(lpFileName);
		int ret;
		struct stat buf;
		ret = squash_stat(enclose_io_fs, enclose_io_converted, &buf);
		if (-1 == ret) {
			goto error;
		}
		if (S_ISDIR(buf.st_mode)) {
			SQUASH_DIR *handle = squash_opendir(enclose_io_fs, enclose_io_converted);
			assert(NULL != handle);
			return (void *)handle;
		} else {
			ret = squash_open(enclose_io_fs, enclose_io_converted);
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
error:
	ENCLOSE_IO_SET_LAST_ERROR;
	return INVALID_HANDLE_VALUE;
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
			ret->FileAttributes = 0;
			if (DT_CHR == mydirent->d_type) {
				ret->FileAttributes &= FILE_ATTRIBUTE_DEVICE;
			} else if (DT_LNK == mydirent->d_type) {
				ret->FileAttributes &= FILE_ATTRIBUTE_REPARSE_POINT;
			} else if (DT_DIR == mydirent->d_type) {
				ret->FileAttributes &= FILE_ATTRIBUTE_DIRECTORY;
			} else {
				ret->FileAttributes &= FILE_ATTRIBUTE_NORMAL;
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

#endif
