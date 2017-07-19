/*
 * Copyright (c) 2017 Minqi Pan <pmq2001@gmail.com>
 *
 * This file is part of libautoupdate, distributed under the MIT License
 * For full terms see the included LICENSE file
 */

#include "autoupdate.h"
#include "autoupdate_internal.h"
#include <assert.h>

#ifdef _WIN32

#include <Windows.h>
#include <wchar.h>
#include <Shlobj.h>

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <string.h>
#include <time.h>
#include <limits.h>

short autoupdate_should_proceed()
{
	TCHAR lpBuffer[32767 + 1];
	if (0 == GetEnvironmentVariable("CI", lpBuffer, 32767)) {
		return 1;
	} else {
		return 0;
	}
}

#else

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <pwd.h>
#include <string.h>
#include <time.h>
#include <limits.h>

short autoupdate_should_proceed()
{
	if (NULL == getenv("CI")) {
		return 1;
	} else {
		return 0;
	}
}

#endif // _WIN32

#ifdef _WIN32
short autoupdate_should_proceed_24_hours(int argc, wchar_t *wargv[], short will_write)
{
	const KNOWNFOLDERID rfid = FOLDERID_Profile;
	PWSTR ppszPath = NULL;
	HRESULT hret;
	wchar_t filepath[2 * 32768];
	const wchar_t *filename = L"\\.libautoupdate";
	size_t exec_path_len = 2 * 32768;
	char exec_path[2 * 32768];
#else
short autoupdate_should_proceed_24_hours(int argc, char *argv[], short will_write)
{
	char *filepath = NULL;
	const char *filename = "/.libautoupdate";
	size_t exec_path_len = 2 * PATH_MAX;
	char exec_path[2 * PATH_MAX];
	struct passwd *pw;
	const char *homedir;
#endif // _WIN32
	short has_written = 0;
	time_t time_now;
	long item_time;
	char *item_string = NULL;
	char *item_space;
	char *cursor;
	char *string = NULL;
	char *string0 = NULL;
	long fsize;
	FILE *f = NULL;
	int ret;
	size_t size_t_ret;
	
	if (autoupdate_exepath(exec_path, &exec_path_len) != 0) {
#ifdef _WIN32
		goto exit;
#else
		if (!argv[0]) {
			goto exit;
		}
		assert(strlen(argv[0]) < 2 * PATH_MAX);
		memcpy(exec_path, argv[0], strlen(argv[0]));
#endif
	}

	time_now = time(NULL);
	if ((time_t)-1 == time_now) {
		goto exit;
	}
#ifdef _WIN32
	hret = SHGetKnownFolderPath(
		&rfid,
		0,
		NULL,
		&ppszPath
	);
	if (S_OK != hret) {
		goto exit;
	}
	memcpy(filepath, ppszPath, wcslen(ppszPath) * sizeof(wchar_t));
	memcpy(filepath + wcslen(ppszPath), filename, wcslen(filename) * sizeof(wchar_t));
	filepath[wcslen(ppszPath) + wcslen(filename)] = 0;
	f = _wfopen(filepath, L"rb");
#else
	pw = getpwuid(getuid());
	if (NULL == pw) {
		goto exit;
	}
	homedir = pw->pw_dir;
	if (NULL == homedir) {
		goto exit;
	}
	filepath = malloc(strlen(homedir) + strlen(filename) + 1);
	if (NULL == filepath) {
		goto exit;
	}
	memcpy(filepath, homedir, strlen(homedir));
	memcpy(filepath + strlen(homedir), filename, strlen(filename));
	filepath[strlen(homedir) + strlen(filename)] = 0;
	f = fopen(filepath, "rb");
#endif // _WIN32
	if (NULL == f) {
		if (will_write) {
			string0 = NULL;
			goto write;
		} else {
			goto exit;
		}
	}
	ret = fseek(f, 0, SEEK_END);
	if (0 != ret) {
		goto exit;
	}
	fsize = ftell(f);
	if (fsize <= 0) {
		goto exit;
	}
	ret = fseek(f, 0, SEEK_SET);
	if (0 != ret) {
		goto exit;
	}
	
	string = malloc(fsize + 1);
	if (NULL == string) {
		goto exit;
	}
	string0 = string;
	size_t_ret = fread(string, fsize, 1, f);
	if (1 != size_t_ret) {
		goto exit;
	}
	string[fsize] = 0;
	ret = fclose(f);
	if (0 != ret) {
		goto exit;
	}
	f = NULL;
	string[fsize] = 0;
	while (string < string0 + fsize) {
		cursor = strchr(string, '\n');
		if (!cursor) {
			if (will_write) {
				string0 = NULL;
				goto write;
			} else {
				goto exit;
			}
		}
		*cursor = 0;
		item_space = strchr(string, ' ');
		if (!item_space) {
			goto exit;
		}
		*item_space = 0;
		item_time = atol(string);
		item_string = item_space + 1;
		if (exec_path_len == cursor - item_string && 0 == memcmp(item_string, exec_path, exec_path_len)) {
			if (will_write) {
				if (item_time >= 1000000000 && time_now >= 1000000000) {
					has_written = 1;
#ifdef _WIN32
					_ltoa(time_now, string, 10);
#else
					ret = sprintf(string, "%ld", time_now);
#endif // _WIN32
					string[10] = ' ';
					*cursor = '\n';
					break;
				}
			} else if (time_now - item_time < 24 * 3600) {
				return 0;
			}
		}
		*item_space = ' ';
		*cursor = '\n';
		string = cursor + 1;
	}
write:
	if (will_write) {
#ifdef _WIN32
		f = _wfopen(filepath, L"wb");
#else
		f = fopen(filepath, "wb");
#endif // _WIN32
		if (NULL == f) {
			goto exit;
		}
		if (string0) {
			ret = fwrite(string0, fsize, 1, f);
			if (1 != ret) {
				goto exit;
			}
		}
		if (!has_written) {
			char writting[20];
#ifdef _WIN32
			_ltoa(time_now, writting, 10);
			ret = fwrite(writting, strlen(writting), 1, f);
			if (1 != ret) {
				goto exit;
			}
			ret = fwrite(" ", 1, 1, f);
			if (1 != ret) {
				goto exit;
			}
#else
			ret = sprintf(writting, "%ld ", time_now);
			ret = fwrite(writting, strlen(writting), 1, f);
			if (1 != ret) {
				goto exit;
			}
#endif // _WIN32
			ret = fwrite(exec_path, exec_path_len, 1, f);
			if (1 != ret) {
				goto exit;
			}
			ret = fwrite("\n", 1, 1, f);
			if (1 != ret) {
				goto exit;
			}
		}
	}

exit:
	if (f) {
		fclose(f);
	}
	if (string0) {
		free(string0);
	}
#ifdef _WIN32
	if (ppszPath) {
		CoTaskMemFree(ppszPath);
	}
#else
	if (filepath) {
		free(filepath);
	}
#endif
	return 1;
}
