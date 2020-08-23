/*
 * Copyright (c) 2017 - 2020 Minqi Pan <pmq2001@gmail.com>
 *
 * This file is part of libautoupdate, distributed under the MIT License
 * For full terms see the included LICENSE file
 */

#include "autoupdate.h"
#include "autoupdate_internal.h"

#include <time.h>
#include <stdlib.h>

#ifdef _WIN32

#include <Windows.h>
#include <Shlwapi.h>
#include <process.h>
#include <assert.h>

wchar_t* autoupdate_tmpdir()
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

wchar_t* autoupdate_tmpf(wchar_t *tmpdir, const char *ext_name)
{
	const int squash_win32_buf_sz = 32767;
	wchar_t squash_win32_buf[32767 + 1];
	size_t curlen, size_ret;
	int try_cnt = 0;
	srand(time(NULL) * getpid());
	squash_win32_buf[squash_win32_buf_sz] = 0;
	while (try_cnt < 3) {
		squash_win32_buf[0] = 0;
		assert(0 == wcslen(squash_win32_buf));
		wcsncat(squash_win32_buf + wcslen(squash_win32_buf), tmpdir, squash_win32_buf_sz - wcslen(squash_win32_buf));
		wcsncat(squash_win32_buf + wcslen(squash_win32_buf), L"\\libautoupdate-", squash_win32_buf_sz - wcslen(squash_win32_buf));
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

#else

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>

char* autoupdate_tmpdir()
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

char* autoupdate_tmpf(char *tmpdir, const char *ext_name)
{
	const int squash_buf_sz = 32767;
	char squash_buf[squash_buf_sz + 1];
	int ret, try_cnt = 0;
	struct stat statbuf;

	srand(time(NULL) * getpid());
	while (try_cnt < 3) {
		if (ext_name) {
			ret = snprintf(squash_buf, squash_buf_sz, "%s/libautoupdate-%d.%s", tmpdir, rand(), ext_name);
		} else {
			ret = snprintf(squash_buf, squash_buf_sz, "%s/libautoupdate-%d", tmpdir, rand());
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
