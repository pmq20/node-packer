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
		int ret = enclose_io_open(2, enclose_io_converted, 0);
		if (-1 == ret) {
			ENCLOSE_IO_SET_LAST_ERROR;
			return INVALID_HANDLE_VALUE;
		} else {
			assert(ret > 0);
			int *handle = (int *)malloc(sizeof(int));
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
}

#endif
