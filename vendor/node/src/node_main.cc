extern "C" {
#include "enclose_io.h"
}
sqfs *enclose_io_fs;

#include "node.h"

#ifdef _WIN32
#include <VersionHelpers.h>
#include <WinError.h>

int wmain(int argc, wchar_t *wargv[]) {
  sqfs_err enclose_io_ret;
  enclose_io_fs = (sqfs *)malloc(sizeof(sqfs));
  assert(NULL != enclose_io_fs);
  memset(enclose_io_fs, 0, sizeof(sqfs));
  enclose_io_ret = sqfs_open_image(enclose_io_fs, enclose_io_memfs, 0);
  assert(SQFS_OK == enclose_io_ret);
  
  wchar_t *argv_memory = NULL;
  int new_argc = argc;
  wchar_t **new_argv = wargv;
  if (NULL == getenv("ENCLOSE_IO_USE_ORIGINAL_NODE")) {
    ENCLOSE_IO_ENTRANCE;
  }

  if (!IsWindows7OrGreater()) {
    fprintf(stderr, "This application is only supported on Windows 7, "
                    "Windows Server 2008 R2, or higher.");
    exit(ERROR_EXE_MACHINE_TYPE_MISMATCH);
  }

  // Convert argv to to UTF8
  char** argv = new char*[new_argc + 1];
  for (int i = 0; i < new_argc; i++) {
    // Compute the size of the required buffer
    DWORD size = WideCharToMultiByte(CP_UTF8,
                                     0,
                                     new_argv[i],
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
                                       new_argv[i],
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
  argv[new_argc] = nullptr;
  // Now that conversion is done, we can finally start.
  return node::Start(new_argc, argv);
}
#else
// UNIX
int main(int argc, char *argv[]) {
  sqfs_err enclose_io_ret;
  enclose_io_fs = (sqfs *)malloc(sizeof(sqfs));
  assert(NULL != enclose_io_fs);
  memset(enclose_io_fs, 0, sizeof(sqfs));
  enclose_io_ret = sqfs_open_image(enclose_io_fs, enclose_io_memfs, 0);
  assert(SQFS_OK == enclose_io_ret);

  char *argv_memory = NULL;
  int new_argc = argc;
  char **new_argv = argv;
  if (NULL == getenv("ENCLOSE_IO_USE_ORIGINAL_NODE")) {
    ENCLOSE_IO_ENTRANCE;
  }

  // Disable stdio buffering, it interacts poorly with printf()
  // calls elsewhere in the program (e.g., any logging from V8.)
  setvbuf(stdout, nullptr, _IONBF, 0);
  setvbuf(stderr, nullptr, _IONBF, 0);
  return node::Start(new_argc, new_argv);
}
#endif
