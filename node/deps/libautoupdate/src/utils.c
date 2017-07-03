/*
 * Copyright (c) 2017 Minqi Pan <pmq2001@gmail.com>
 *
 * This file is part of libautoupdate, distributed under the MIT License
 * For full terms see the included LICENSE file
 */

#include "autoupdate.h"
#include "autoupdate_internal.h"

#ifdef _WIN32

#include <Windows.h>

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

#include <stdlib.h>     /* getenv */

short autoupdate_should_proceed()
{
	if (NULL == getenv("CI")) {
		return 1;
	} else {
		return 0;
	}
}

#endif // _WIN32
