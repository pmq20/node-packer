/*
 * Copyright (c) 2017 Minqi Pan <pmq2001@gmail.com>
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

#ifdef __linux__ 

#include <unistd.h>
#include <errno.h>

int autoupdate_exepath(char* buffer, size_t* size) {
  ssize_t n;

  if (buffer == NULL || size == NULL || *size == 0)
    return -EINVAL;

  n = *size - 1;
  if (n > 0)
    n = readlink("/proc/self/exe", buffer, n);

  if (n == -1)
    return -errno;

  buffer[n] = '\0';
  *size = n;

  return 0;
}

#elif __APPLE__

#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <mach-o/dyld.h>
#include <limits.h>  // PATH_MAX

int autoupdate_exepath(char* buffer, size_t* size) {
  /* realpath(exepath) may be > PATH_MAX so double it to be on the safe side. */
  char abspath[PATH_MAX * 2 + 1];
  char exepath[PATH_MAX + 1];
  uint32_t exepath_size;
  size_t abspath_size;

  if (buffer == NULL || size == NULL || *size == 0)
    return -EINVAL;

  exepath_size = sizeof(exepath);
  if (_NSGetExecutablePath(exepath, &exepath_size))
    return -EIO;

  if (realpath(exepath, abspath) != abspath)
    return -errno;

  abspath_size = strlen(abspath);
  if (abspath_size == 0)
    return -EIO;

  *size -= 1;
  if (*size > abspath_size)
    *size = abspath_size;

  memcpy(buffer, abspath, *size);
  buffer[*size] = '\0';

  return 0;
}

#endif
