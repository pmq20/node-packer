/*
 * Copyright (c) 2017 Minqi Pan <pmq2001@gmail.com>
 *                    Shengyuan Liu <sounder.liu@gmail.com>
 *
 * This file is part of libsquash, distributed under the MIT License
 * For full terms see the included LICENSE file
 */

#ifndef ENCLOSE_IO_WIN32_H_9A3D18C6
#define ENCLOSE_IO_WIN32_H_9A3D18C6
#ifndef __cplusplus
#ifdef _WIN32

#ifdef _fstati64
#undef _fstati64
#endif

#define chdir(...)	enclose_io_chdir(__VA_ARGS__)

#ifndef RUBY_EXPORT
	// Ruby has defined the following symbols to
	// something like rb_w32_uopen
	#define getcwd(...)	enclose_io_getcwd(__VA_ARGS__)
	#define stat(...)	enclose_io_stat(__VA_ARGS__)
	#define fstat(...)	enclose_io_fstat(__VA_ARGS__)
	#define open(...)	enclose_io_open(ENCLOSE_IO_PP_NARG(__VA_ARGS__), __VA_ARGS__)
	#define close(...)	enclose_io_close(__VA_ARGS__)
	#define read(...)	enclose_io_read(__VA_ARGS__)
	#define lseek(...)	enclose_io_lseek(__VA_ARGS__)

	// Ruby does not use those things
	#define pNtQueryDirectoryFile(...) EncloseIOpNtQueryDirectoryFile(__VA_ARGS__)
	#define pNtQueryInformationFile(...) EncloseIOpNtQueryInformationFile(__VA_ARGS__)
	#define pNtQueryVolumeInformationFile(...) EncloseIOpNtQueryVolumeInformationFile(__VA_ARGS__)
#endif // !RUBY_EXPORT

#define _chdir(...) enclose_io_chdir(__VA_ARGS__)
#define _wchdir(...) enclose_io_wchdir(__VA_ARGS__)
#define _getwd(...) enclose_io_getwd(__VA_ARGS__)
#define _wgetwd(...) enclose_io_wgetwd(__VA_ARGS__)
#define _fstati64(...)	enclose_io_fstati64(__VA_ARGS__)
#define _open(...)	enclose_io__open(__VA_ARGS__)
#define _wopen(...)	enclose_io__wopen(__VA_ARGS__)
#define _wmkdir(...) enclose_io__wmkdir(__VA_ARGS__)
#define _close(...)	enclose_io_close(__VA_ARGS__)
#define _open_osfhandle(...) enclose_io_open_osfhandle(__VA_ARGS__)
#define _get_osfhandle(...) enclose_io_get_osfhandle(__VA_ARGS__)
#define _read(...)	enclose_io_read(__VA_ARGS__)
#define _lseeki64(...)	enclose_io_lseeki64(__VA_ARGS__)

#define CreateFileW(...) EncloseIOCreateFileW(__VA_ARGS__)
#define CloseHandle(...) EncloseIOCloseHandle(__VA_ARGS__)
#define ReadFile(...) EncloseIOReadFile(__VA_ARGS__)

#define GetFileAttributesW(...) EncloseIOGetFileAttributesW(__VA_ARGS__)
#define GetFileAttributesExW(...) EncloseIOGetFileAttributesExW(__VA_ARGS__)
#define GetHandleInformation(...) EncloseIOGetHandleInformation(__VA_ARGS__)
#define GetFileType(...) EncloseIOGetFileType(__VA_ARGS__)
#define FindFirstFileW(...) EncloseIOFindFirstFileW(__VA_ARGS__)
#define FindNextFileW(...) EncloseIOFindNextFileW(__VA_ARGS__)
#define FindClose(...) EncloseIOFindClose(__VA_ARGS__)
#define DeviceIoControl(...) EncloseIODeviceIoControl(__VA_ARGS__)
#define CreateIoCompletionPort(...) EncloseIOCreateIoCompletionPort(__VA_ARGS__)
#define ReadDirectoryChangesW(...) EncloseIOReadDirectoryChangesW(__VA_ARGS__)
#define LoadLibraryExW(...) EncloseIOLoadLibraryExW(__VA_ARGS__)
#define CreateProcessW(...) EncloseIOCreateProcessW(__VA_ARGS__)
#define SetCurrentDirectoryW(...) EncloseIOSetCurrentDirectoryW(__VA_ARGS__)
#define GetCurrentDirectoryW(...) EncloseIOGetCurrentDirectoryW(__VA_ARGS__)

#endif //_WIN32
#endif //!__cplusplus
#endif
