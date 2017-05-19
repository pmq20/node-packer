#include "node.h"

// ======= [Enclose.io Hack start] =========
extern "C" {
  #include "enclose_io.h"
}
// ======= [Enclose.io Hack end] =========

#ifdef _WIN32
#include <VersionHelpers.h>
#include <WinError.h>

#if ENCLOSE_IO_AUTO_UPDATE
	void enclose_io_autoupdate()
	{
		// TODO
	}
#endif

int wmain(int argc, wchar_t *wargv[]) {
  // ======= [Enclose.io Hack start] =========
  #if ENCLOSE_IO_AUTO_UPDATE
    enclose_io_autoupdate();
  #endif
  sqfs_err enclose_io_ret;
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

#if ENCLOSE_IO_AUTO_UPDATE
	#include <stdio.h> /* printf, sprintf */
	#include <stdlib.h> /* exit */
	#include <unistd.h> /* read, write, close */
	#include <string.h> /* memcpy, memset */
	#include <sys/socket.h> /* socket, connect */
	#include <netinet/in.h> /* struct sockaddr_in, struct sockaddr */
	#include <netdb.h> /* struct hostent, gethostbyname */
	void enclose_io_autoupdate()
	{
		struct hostent *server;
		struct sockaddr_in serv_addr;
		int sockfd, bytes, sent, received, total;
		char response[1024 * 10];

		sockfd = socket(AF_INET, SOCK_STREAM, 0);
		if (sockfd < 0) {
			return;
		}
		server = gethostbyname(ENCLOSE_IO_AUTO_UPDATE_URL_Host);
		if (server == NULL) {
			return;
		}
		memset(&serv_addr, 0, sizeof(serv_addr));
		serv_addr.sin_family = AF_INET;
		serv_addr.sin_port = htons(ENCLOSE_IO_AUTO_UPDATE_URL_Port);
		memcpy(&serv_addr.sin_addr.s_addr, server->h_addr, server->h_length);
		if (connect(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
			return;
		}
		if (5 != write(sockfd, "HEAD ", 5)) {
			return;
		}
		if (strlen(ENCLOSE_IO_AUTO_UPDATE_URL_Path) != write(sockfd, ENCLOSE_IO_AUTO_UPDATE_URL_Path, strlen(ENCLOSE_IO_AUTO_UPDATE_URL_Path))) {
			return;
		}
		if (13 != write(sockfd, " HTTP/1.0\r\n\r\n", 13)) {
			return;
		}
		total = sizeof(response) - 1;
		response[sizeof(response) - 1] = 0;
		received = 0;
		do {
			bytes = read(sockfd, response + received, total - received);
			if (bytes < 0) {
				return;
			}
			if (bytes == 0) {
				*(response + received) = 0;
				break;
			}
			received += bytes;
		} while (received < total);
		if (received >= total) {
			return;
		}
		close(sockfd);
		size_t len = strlen(response);
		assert(len <= total);
		char *new_line = NULL;
		char *found = NULL;
		size_t i = 0;
		while (i < len) {
			new_line = strstr(response + i, "\r\n");
			if (NULL == new_line) {
				break;
			}
			*new_line = 0;
			if (0 == strncmp(response + i, "Location: ", 10)) {
				found = response + i + 10;
				break;
			}
			*new_line = '\n';
			i = new_line - response + 2;
		}
		if (!found) {
			return;
		}
		if (strstr(found, ENCLOSE_IO_AUTO_UPDATE_BASE)) {
			return;
		}
	}
#endif

int main(int argc, char *argv[]) {
  // ======= [Enclose.io Hack start] =========
  #if ENCLOSE_IO_AUTO_UPDATE
    enclose_io_autoupdate();
  #endif
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
