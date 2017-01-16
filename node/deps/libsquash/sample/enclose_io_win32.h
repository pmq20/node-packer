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

#define CreateFileW(...) EncloseIOCreateFileW(__VA_ARGS__)
#define pNtQueryDirectoryFile(...) EncloseIOpNtQueryDirectoryFile(__VA_ARGS__)
#define CloseHandle(...) EncloseIOCloseHandle(__VA_ARGS__)

#endif //_WIN32
#endif //!__cplusplus
#endif
