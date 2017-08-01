/*
 * Copyright (c) 2017 Minqi Pan <pmq2001@gmail.com>
 *                    Shengyuan Liu <sounder.liu@gmail.com>
 *
 * This file is part of libsquash, distributed under the MIT License
 * For full terms see the included LICENSE file
 */

#include "squash.h"
#include <time.h>
#include <stdlib.h>

#ifdef _WIN32
#include <Windows.h>
#include <Shlwapi.h>
SQUASH_OS_PATH squash_tmpdir()
{
	const int squash_win32_buf_sz = 32767;
	wchar_t squash_win32_buf[32767 + 1];
	DWORD length;

	length = GetEnvironmentVariableW(L"TEMP", squash_win32_buf, squash_win32_buf_sz);
	if (length) {
		goto out;
	}
	length = GetEnvironmentVariableW(L"TMP", squash_win32_buf, squash_win32_buf_sz);
	if (length) {
		goto out;
	}
	length = GetEnvironmentVariableW(L"SystemRoot", squash_win32_buf, squash_win32_buf_sz);
	if (!length) {
		length = GetEnvironmentVariableW(L"windir", squash_win32_buf, squash_win32_buf_sz);
	}
	if (length) {
		if (length + 5 >= squash_win32_buf_sz) {
			return NULL;
		}
		squash_win32_buf[length] = L'\\';
		squash_win32_buf[length + 1] = L't';
		squash_win32_buf[length + 2] = L'e';
		squash_win32_buf[length + 3] = L'm';
		squash_win32_buf[length + 4] = L'p';
		squash_win32_buf[length + 5] = 0;
		length += 5;
		goto out;
	}
	return NULL;
out:
	if (length >= 2 && L'\\' == squash_win32_buf[length - 1] && L':' != squash_win32_buf[length - 2]) {
		squash_win32_buf[length - 1] = 0;
		length -= 1;
	}
	return wcsdup(squash_win32_buf);
}
SQUASH_OS_PATH squash_tmpf(SQUASH_OS_PATH tmpdir, const char *ext_name)
{
	const int squash_win32_buf_sz = 32767;
	wchar_t squash_win32_buf[32767 + 1];
	size_t curlen, size_ret;
	int ret, try_cnt = 0;
	srand(time(NULL) * getpid());
	squash_win32_buf[squash_win32_buf_sz] = 0;
	while (try_cnt < 3) {
		squash_win32_buf[0] = 0;
		assert(0 == wcslen(squash_win32_buf));
		wcsncat(squash_win32_buf + wcslen(squash_win32_buf), tmpdir, squash_win32_buf_sz - wcslen(squash_win32_buf));
		wcsncat(squash_win32_buf + wcslen(squash_win32_buf), L"\\libsquash-", squash_win32_buf_sz - wcslen(squash_win32_buf));
		// up to 33 characters for _itoa
		if (squash_win32_buf_sz - wcslen(squash_win32_buf) <= 33) {
			return NULL;
		}
		_itow(rand(), squash_win32_buf + wcslen(squash_win32_buf), 10);
		if (ext_name) {
			wcsncat(squash_win32_buf + wcslen(squash_win32_buf), L".", squash_win32_buf_sz - wcslen(squash_win32_buf));
		}
		if (ext_name) {
			curlen = wcslen(squash_win32_buf);
			size_ret = mbstowcs((wchar_t*)(squash_win32_buf) + curlen, ext_name, squash_win32_buf_sz - curlen);
			if ((size_t)-1 == size_ret) {
				return NULL;
			}
			*((wchar_t*)(squash_win32_buf) + curlen + size_ret) = 0;
		}
		if (!PathFileExistsW(squash_win32_buf)) {
			return wcsdup(squash_win32_buf);
		}
		++try_cnt;
	}
	return NULL;
}
#else // _WIN32

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

SQUASH_OS_PATH squash_tmpdir()
{
	char *try_try;
	size_t length;
	try_try = getenv("TMPDIR");
	if (try_try) {
		goto out;
	}
	try_try = getenv("TMP");
	if (try_try) {
		goto out;
	}
	try_try = getenv("TEMP");
	if (try_try) {
		goto out;
	}
	try_try = "/tmp";
out:
	try_try = strdup(try_try);
	length = strlen(try_try);
	if (length >= 2 && '/' == try_try[length - 1]) {
		try_try[length - 1] = 0;
	}
	return try_try;
}
SQUASH_OS_PATH squash_tmpf(SQUASH_OS_PATH tmpdir, const char *ext_name)
{
	const int squash_buf_sz = 32767;
	char squash_buf[squash_buf_sz + 1];
	int ret, try_cnt = 0;
	struct stat statbuf;

	srand(time(NULL) * getpid());
	while (try_cnt < 3) {
		if (ext_name) {
			ret = snprintf(squash_buf, squash_buf_sz, "%s/libsquash-%d.%s", tmpdir, rand(), ext_name);
		} else {
			ret = snprintf(squash_buf, squash_buf_sz, "%s/libsquash-%d", tmpdir, rand());
		}
		if (-1 == ret) {
			return NULL;
		}
		if (-1 == stat(squash_buf, &statbuf)) {
			return strdup(squash_buf);
		}
		++try_cnt;
	}
	return NULL;
}
#endif // _WIN32

static SQUASH_OS_PATH squash_uncached_extract(sqfs *fs, const char *path, const char *ext_name)
{
	static SQUASH_OS_PATH tmpdir = NULL;
	FILE *fp;
	int fd;
	SQUASH_OS_PATH tmpf;
	size_t size;
	ssize_t ssize;
	char buffer[16 * 1024];
	sqfs_off_t offset;
	struct squash_file *file;

	fd = squash_open(fs, path);
	if (-1 == fd) {
		return NULL;
	}
	if (NULL == tmpdir) {
		tmpdir = squash_tmpdir();
	}
	if (NULL == tmpdir) {
		return NULL;
	}
	tmpf = squash_tmpf(tmpdir, ext_name);
	if (NULL == tmpf) {
		return NULL;
	}
#ifdef _WIN32
	fp = _wfopen(tmpf, L"wb");
#else
	fp = fopen(tmpf, "wb");
#endif
	if (NULL == fp) {
		free(tmpf);
		return NULL;
	}
	file = SQUASH_VFD_FILE(fd);
	offset = file->node.xtra.reg.file_size;
	while (offset > 0) {
		ssize = squash_read(fd, buffer, 16 * 1024);
		if (ssize <= 0) {
			fclose(fp);
			free(tmpf);
			return NULL;
		}
		offset -= ssize;
		size = fwrite(buffer, ssize, 1, fp);
		if (size != 1) {
			fclose(fp);
			free(tmpf);
			return NULL;
		}
	}
	assert(0 == offset);
	fclose(fp);
	return tmpf;
}

struct SquashExtractEntry {
	sqfs *fs;
	const char *path;
	SQUASH_OS_PATH ret;
	struct SquashExtractEntry *next;
};

static struct SquashExtractEntry* squash_extract_cache = NULL;

static const struct SquashExtractEntry* squash_extract_cache_find(sqfs *fs, const char *path)
{
	struct SquashExtractEntry* ptr = squash_extract_cache;
	while (NULL != ptr) {
		if (fs == ptr->fs && 0 == strcmp(path, ptr->path)) {
			return ptr;
		}
		ptr = ptr->next;
	}
	return ptr;
}
static void squash_extract_cache_insert(sqfs *fs, const char *path, SQUASH_OS_PATH ret)
{
	struct SquashExtractEntry* ptr = malloc(sizeof(struct SquashExtractEntry));
	if (NULL == ptr) {
		return;
	}
	ptr->fs = fs;
	ptr->path = path;
	ptr->ret = ret;
	ptr->next = squash_extract_cache;
	squash_extract_cache = ptr;
}

SQUASH_OS_PATH squash_extract(sqfs *fs, const char *path, const char *ext_name)
{
	SQUASH_OS_PATH ret;
	static struct SquashExtractEntry* found;

	found = squash_extract_cache_find(fs, path);
	if (NULL != found) {
		return found->ret;
	}
	ret = squash_uncached_extract(fs, path, ext_name);
	if (NULL != ret) {
		squash_extract_cache_insert(fs, path, ret);
	}
	return ret;
}

void squash_extract_clear_cache()
{
	struct SquashExtractEntry* ptr = squash_extract_cache;
	while (NULL != ptr) {
#ifdef _WIN32
		DeleteFileW(ptr->ret);
#else
		unlink(ptr->ret);
#endif
		ptr = ptr->next;
	}
}
