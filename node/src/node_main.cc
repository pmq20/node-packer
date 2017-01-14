#include "node.h"

// ======= [Enclose.io Hack start] =========
#ifdef _WIN32
#define WINDOWS_H_A80B5674
typedef unsigned short sqfs_mode_t;
typedef uint32_t sqfs_id_t;
typedef DWORD64 sqfs_off_t;
struct dirent
{
  long d_namlen;
  ino_t d_ino;
  char *d_name;
  char *d_altname;
  short d_altlen;
  uint8_t d_type;
};
#endif
extern "C" {
  #include "enclose_io.h"
}
// ======= [Enclose.io Hack end] =========

#ifdef _WIN32
#include <VersionHelpers.h>
#include <WinError.h>

int wmain(int argc, wchar_t *wargv[]) {
  // ======= [Enclose.io Hack start] =========
  sqfs_err enclose_io_ret;
  enclose_io_ret = squash_start();
  assert(SQFS_OK == enclose_io_ret);
  enclose_io_fs = (sqfs *)malloc(sizeof(sqfs));
  assert(NULL != enclose_io_fs);
  memset(enclose_io_fs, 0, sizeof(sqfs));
  enclose_io_ret = sqfs_open_image(enclose_io_fs, enclose_io_memfs, 0);
  assert(SQFS_OK == enclose_io_ret);

  #ifdef ENCLOSE_IO_ENTRANCE
  int new_argc = argc;
  wchar_t **new_argv = wargv;
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
  // ======= [Enclose.io Hack end] =========

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
int main(int argc, char *argv[]) {
  // ======= [Enclose.io Hack start] =========
  sqfs_err enclose_io_ret;
  enclose_io_ret = squash_start();
  assert(SQFS_OK == enclose_io_ret);
  enclose_io_fs = (sqfs *)malloc(sizeof(sqfs));
  assert(NULL != enclose_io_fs);
  memset(enclose_io_fs, 0, sizeof(sqfs));
  enclose_io_ret = sqfs_open_image(enclose_io_fs, enclose_io_memfs, 0);
  assert(SQFS_OK == enclose_io_ret);

  #ifdef ENCLOSE_IO_ENTRANCE
  char *argv_memory = NULL;
  int new_argc = argc;
  char **new_argv = argv;
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
  // ======= [Enclose.io Hack end] =========

  // Disable stdio buffering, it interacts poorly with printf()
  // calls elsewhere in the program (e.g., any logging from V8.)
  setvbuf(stdout, nullptr, _IONBF, 0);
  setvbuf(stderr, nullptr, _IONBF, 0);
  return node::Start(argc, argv);
}
#endif
