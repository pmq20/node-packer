// Copyright Joyent, Inc. and other Node contributors.
//
// Permission is hereby granted, free of charge, to any person obtaining a
// copy of this software and associated documentation files (the
// "Software"), to deal in the Software without restriction, including
// without limitation the rights to use, copy, modify, merge, publish,
// distribute, sublicense, and/or sell copies of the Software, and to permit
// persons to whom the Software is furnished to do so, subject to the
// following conditions:
//
// The above copyright notice and this permission notice shall be included
// in all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
// OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
// MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN
// NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM,
// DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR
// OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE
// USE OR OTHER DEALINGS IN THE SOFTWARE.

#include "node.h"
#include <stdio.h>

// --------- [Enclose.IO Hack start] ---------
extern "C" {
  #include "enclose_io.h"
  #include "autoupdate.h"
}
// --------- [Enclose.IO Hack end] ---------

#ifdef _WIN32
#include <windows.h>
#include <VersionHelpers.h>
#include <WinError.h>

int wmain(int argc, wchar_t *wargv[]) {
  // --------- [Enclose.IO Hack start] ---------
  int autoupdate_result;
  sqfs_err enclose_io_ret;
  int new_argc;
  wchar_t **new_argv;

  #if ENCLOSE_IO_AUTO_UPDATE
    autoupdate_result = autoupdate(
      argc,
      wargv,
      ENCLOSE_IO_AUTO_UPDATE_URL_Host,
      ENCLOSE_IO_AUTO_UPDATE_URL_Port,
      ENCLOSE_IO_AUTO_UPDATE_URL_Path,
      ENCLOSE_IO_AUTO_UPDATE_BASE,
      0
    );
  #endif

  enclose_io_ret = squash_start();
  assert(SQFS_OK == enclose_io_ret);
  enclose_io_fs = (sqfs *)calloc(sizeof(sqfs), 1);
  assert(NULL != enclose_io_fs);
  enclose_io_ret = sqfs_open_image(enclose_io_fs, enclose_io_memfs, 0);
  assert(SQFS_OK == enclose_io_ret);
  #ifdef ENCLOSE_IO_ROOT_ALIAS
    enclose_io_fs->root_alias = ENCLOSE_IO_ROOT_ALIAS;
  #endif
  #ifdef ENCLOSE_IO_ROOT_ALIAS2
    enclose_io_fs->root_alias2 = ENCLOSE_IO_ROOT_ALIAS2;
  #endif

  #ifdef ENCLOSE_IO_ENTRANCE
    new_argc = argc;
    new_argv = wargv;
    if (NULL == getenv("ENCLOSE_IO_USE_ORIGINAL_NODE")) {
      new_argv = (wchar_t **)malloc( (argc + 1) * sizeof(wchar_t *));
      assert(new_argv);
      new_argv[0] = wargv[0];
      new_argv[1] = ENCLOSE_IO_ENTRANCE;
      for (size_t i = 1; i < argc; ++i) {
             new_argv[2 + i - 1] = wargv[i];
      }
      new_argc = argc + 1;
    }
    argc = new_argc;
    wargv = new_argv;
  #endif
  // --------- [Enclose.IO Hack end] ---------

  if (!IsWindows7OrGreater()) {
    fprintf(stderr, "This application is only supported on Windows 7, "
                    "Windows Server 2008 R2, or higher.");
    exit(ERROR_EXE_MACHINE_TYPE_MISMATCH);
  }

  // Convert argv to to UTF8
  char** argv = new char*[argc + 1];
  for (int i = 0; i < argc; i++) {
    // Compute the size of the required buffer
    DWORD size = WideCharToMultiByte(CP_UTF8,
                                     0,
                                     wargv[i],
                                     -1,
                                     nullptr,
                                     0,
                                     nullptr,
                                     nullptr);
    if (size == 0) {
      // This should never happen.
      fprintf(stderr, "Could not convert arguments to utf8.");
      exit(1);
    }
    // Do the actual conversion
    argv[i] = new char[size];
    DWORD result = WideCharToMultiByte(CP_UTF8,
                                       0,
                                       wargv[i],
                                       -1,
                                       argv[i],
                                       size,
                                       nullptr,
                                       nullptr);
    if (result == 0) {
      // This should never happen.
      fprintf(stderr, "Could not convert arguments to utf8.");
      exit(1);
    }
  }
  argv[argc] = nullptr;
  // Now that conversion is done, we can finally start.
  return node::Start(argc, argv);
}
#else
// UNIX
#ifdef __linux__
#include <elf.h>
#ifdef __LP64__
#define Elf_auxv_t Elf64_auxv_t
#else
#define Elf_auxv_t Elf32_auxv_t
#endif  // __LP64__
extern char** environ;
#endif  // __linux__

namespace node {
  extern bool linux_at_secure;
}  // namespace node

int main(int argc, char *argv[]) {
#if defined(__linux__)
  char** envp = environ;
  while (*envp++ != nullptr) {}
  Elf_auxv_t* auxv = reinterpret_cast<Elf_auxv_t*>(envp);
  for (; auxv->a_type != AT_NULL; auxv++) {
    if (auxv->a_type == AT_SECURE) {
      node::linux_at_secure = auxv->a_un.a_val;
      break;
    }
  }
#endif
  // --------- [Enclose.IO Hack start] ---------
  int autoupdate_result;
  sqfs_err enclose_io_ret;
  char *argv_memory;
  int new_argc;
  char **new_argv;
  
  #if ENCLOSE_IO_AUTO_UPDATE
    autoupdate_result = autoupdate(
      argc,
      argv,
      ENCLOSE_IO_AUTO_UPDATE_URL_Host,
      ENCLOSE_IO_AUTO_UPDATE_URL_Port,
      ENCLOSE_IO_AUTO_UPDATE_URL_Path,
      ENCLOSE_IO_AUTO_UPDATE_BASE,
      0
    );
  #endif
  
  enclose_io_ret = squash_start();
  assert(SQFS_OK == enclose_io_ret);
  enclose_io_fs = (sqfs *)malloc(sizeof(sqfs));
  assert(NULL != enclose_io_fs);
  memset(enclose_io_fs, 0, sizeof(sqfs));
  enclose_io_ret = sqfs_open_image(enclose_io_fs, enclose_io_memfs, 0);
  assert(SQFS_OK == enclose_io_ret);

  #ifdef ENCLOSE_IO_ENTRANCE
    argv_memory = NULL;
    new_argc = argc;
    new_argv = argv;
    if (NULL == getenv("ENCLOSE_IO_USE_ORIGINAL_NODE")) {
      new_argv = (char **)malloc( (argc + 1) * sizeof(char *));
      assert(new_argv);
      new_argv[0] = argv[0];
      new_argv[1] = ENCLOSE_IO_ENTRANCE;
      for (size_t i = 1; i < argc; ++i) {
             new_argv[2 + i - 1] = argv[i];
      }
      new_argc = argc + 1;
      /* argv memory should be adjacent. */
      size_t total_argv_size = 0;
      for (size_t i = 0; i < new_argc; ++i) {
             total_argv_size += strlen(new_argv[i]) + 1;
      }
      argv_memory = (char *)malloc( (total_argv_size) * sizeof(char));
      assert(argv_memory);
      for (size_t i = 0; i < new_argc; ++i) {
             memcpy(argv_memory, new_argv[i], strlen(new_argv[i]) + 1);
             new_argv[i] = argv_memory;
             argv_memory += strlen(new_argv[i]) + 1;
      }
      assert(argv_memory - new_argv[0] == total_argv_size);
    }
    argc = new_argc;
    argv = new_argv;
    #endif
  // --------- [Enclose.IO Hack end] ---------

  // Disable stdio buffering, it interacts poorly with printf()
  // calls elsewhere in the program (e.g., any logging from V8.)
  setvbuf(stdout, nullptr, _IONBF, 0);
  setvbuf(stderr, nullptr, _IONBF, 0);
  return node::Start(argc, argv);
}

#endif
