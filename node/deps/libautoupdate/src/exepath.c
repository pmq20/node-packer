/*
 * Copyright (c) 2017 - 2020 Minqi Pan et al.
 *
 * This file is part of libautoupdate, distributed under the MIT License
 * For full terms see the included LICENSE file
 */

/*
 * autoupdate_exepath is derived from uv_exepath of libuv.
 * libuv is licensed for use as follows:
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to
 * deal in the Software without restriction, including without limitation the
 * rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
 * sell copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
 * IN THE SOFTWARE.
 */

#include "autoupdate.h"
#include "autoupdate_internal.h"

#ifdef _WIN32

#include <assert.h>
#include <direct.h>
#include <limits.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <wchar.h>
#include <windows.h>

int autoupdate_exepath(char* buffer, size_t* size_ptr) {
  int utf8_len, utf16_buffer_len, utf16_len;
  WCHAR* utf16_buffer;
  int err;

  if (buffer == NULL || size_ptr == NULL || *size_ptr == 0) {
    return -1;
  }

  if (*size_ptr > 32768) {
    /* Windows paths can never be longer than this. */
    utf16_buffer_len = 32768;
  } else {
    utf16_buffer_len = (int) *size_ptr;
  }

  utf16_buffer = (WCHAR*) malloc(sizeof(WCHAR) * utf16_buffer_len);
  if (!utf16_buffer) {
    return -1;
  }

  /* Get the path as UTF-16. */
  utf16_len = GetModuleFileNameW(NULL, utf16_buffer, utf16_buffer_len);
  if (utf16_len <= 0) {
    err = GetLastError();
    goto error;
  }

  /* utf16_len contains the length, *not* including the terminating null. */
  utf16_buffer[utf16_len] = L'\0';

  /* Convert to UTF-8 */
  utf8_len = WideCharToMultiByte(CP_UTF8,
                                 0,
                                 utf16_buffer,
                                 -1,
                                 buffer,
                                 (int) *size_ptr,
                                 NULL,
                                 NULL);
  if (utf8_len == 0) {
    err = GetLastError();
    goto error;
  }

  free(utf16_buffer);

  /* utf8_len *does* include the terminating null at this point, but the */
  /* returned size shouldn't. */
  *size_ptr = utf8_len - 1;
  return 0;

 error:
  free(utf16_buffer);
  return -1;
}

#endif

#ifdef __linux__
#include <unistd.h>

int autoupdate_exepath(char* buffer, size_t* size) {
  ssize_t n;

  if (buffer == NULL || size == NULL || *size == 0)
    return -1;

  n = *size - 1;
  if (n > 0)
    n = readlink("/proc/self/exe", buffer, n);

  if (n == -1)
    return -1;

  buffer[n] = '\0';
  *size = n;

  return 0;
}
#endif

#ifdef __APPLE__
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <mach-o/dyld.h>
#include <limits.h>  // PATH_MAX

int autoupdate_exepath(char* buffer, size_t* size) {
  /* realpath(exepath) may be > PATH_MAX so double it to be on the safe side. */
  char abspath[PATH_MAX * 2 + 1];
  char exepath[PATH_MAX + 1];
  uint32_t exepath_size;
  size_t abspath_size;

  if (buffer == NULL || size == NULL || *size == 0)
    return -1;

  exepath_size = sizeof(exepath);
  if (_NSGetExecutablePath(exepath, &exepath_size))
    return -1;

  if (realpath(exepath, abspath) != abspath)
    return -1;

  abspath_size = strlen(abspath);
  if (abspath_size == 0)
    return -1;

  *size -= 1;
  if (*size > abspath_size)
    *size = abspath_size;

  memcpy(buffer, abspath, *size);
  buffer[*size] = '\0';

  return 0;
}
#endif
