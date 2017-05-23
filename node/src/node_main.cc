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
void enclose_io_autoupdate(int argc, wchar_t *wargv[]);
#endif // ENCLOSE_IO_AUTO_UPDATE

int wmain(int argc, wchar_t *wargv[]) {
  // ======= [Enclose.io Hack start] =========
  #if ENCLOSE_IO_AUTO_UPDATE
    enclose_io_autoupdate(argc, wargv);
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
#if ENCLOSE_IO_AUTO_UPDATE
void enclose_io_autoupdate(int argc, wchar_t *wargv[])
{
	// TODO
}
#endif // ENCLOSE_IO_AUTO_UPDATE
#else
// UNIX
#if ENCLOSE_IO_AUTO_UPDATE
void enclose_io_autoupdate(int argc, char *argv[]);
#endif // ENCLOSE_IO_AUTO_UPDATE
int main(int argc, char *argv[]) {
  // ======= [Enclose.io Hack start] =========
  #if ENCLOSE_IO_AUTO_UPDATE
    enclose_io_autoupdate(argc, argv);
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

#if ENCLOSE_IO_AUTO_UPDATE
#include <stdio.h> /* printf, sprintf */
#include <stdlib.h> /* exit */
#include <unistd.h> /* read, write, close */
#include <string.h> /* memcpy, memset */
#include <sys/socket.h> /* socket, connect */
#include <netinet/in.h> /* struct sockaddr_in, struct sockaddr */
#include <netdb.h> /* struct hostent, gethostbyname */
#include <unistd.h>
#include <sys/select.h>
#include <limits.h>  // PATH_MAX
#include "zlib.h"
#include <iostream>
#include <string>
#include <algorithm>
#include <functional>
#include <cctype>
#include <locale>
void enclose_io_autoupdate(int argc, char *argv[])
{
	struct hostent *server;
	struct sockaddr_in serv_addr;
	int sockfd, bytes, received, total;
	char response[1024 * 10]; // 10KB
	
	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if (sockfd < 0) {
		std::cerr << "AutoUpdate Failed: socket creation failed" << std::endl;
		return;
	}
	server = gethostbyname(ENCLOSE_IO_AUTO_UPDATE_URL_Host);
	if (server == NULL) {
		close(sockfd);
		std::cerr << "AutoUpdate Failed: gethostbyname failed for " << ENCLOSE_IO_AUTO_UPDATE_URL_Host << std::endl;
		return;
	}
	if (0 == strcmp("https", ENCLOSE_IO_AUTO_UPDATE_URL_Scheme)) {
		close(sockfd);
		std::cerr << "AutoUpdate Failed: " << std::endl;
		std::cerr << "  HTTPS is not supported yet." << std::endl;
		std::cerr << "  Pull requests are welcome on GitHub at" << std::endl;
		std::cerr << "  https://github.com/pmq20/node-compiler" << std::endl;
		return;
	}
	memset(&serv_addr, 0, sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_port = htons(ENCLOSE_IO_AUTO_UPDATE_URL_Port);
	memcpy(&serv_addr.sin_addr.s_addr, server->h_addr, server->h_length);
	if (connect(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
		close(sockfd);
		std::cerr << "AutoUpdate Failed: connect failed on " << ENCLOSE_IO_AUTO_UPDATE_URL_Host << " and port " << ENCLOSE_IO_AUTO_UPDATE_URL_Port << std::endl;
		return;
	}
	if (5 != write(sockfd, "HEAD ", 5) ||
	    strlen(ENCLOSE_IO_AUTO_UPDATE_URL_Path) != write(sockfd, ENCLOSE_IO_AUTO_UPDATE_URL_Path, strlen(ENCLOSE_IO_AUTO_UPDATE_URL_Path)) ||
	    13 != write(sockfd, " HTTP/1.0\r\n\r\n", 13)) {
		close(sockfd);
		std::cerr << "AutoUpdate Failed: write failed" << std::endl;
		return;
	}
	total = sizeof(response) - 1;
	received = 0;
	response[sizeof(response) - 1] = 0;
	do {
		bytes = read(sockfd, response + received, total - received);
		if (bytes < 0) {
			close(sockfd);
			std::cerr << "AutoUpdate Failed: read failed" << std::endl;
			return;
		}
		if (bytes == 0) {
			/* EOF */
			*(response + received) = 0;
			break;
		}
		received += bytes;
	} while (received < total);
	if (received == total) {
		close(sockfd);
		std::cerr << "AutoUpdate Failed: read causes buffer full" << std::endl;
		return;
	}
	close(sockfd);
	assert(received < total);
	size_t len = strlen(response);
	assert(len <= total);
	char *new_line = NULL;
	char *found = NULL;
	size_t i = 0;
	while (i < len) {
		new_line = strnstr(response + i, "\r\n", len - i);
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
		std::cerr << "AutoUpdate Failed: failed to find a Location header" << std::endl;
		return;
	}
	if (strstr(found, ENCLOSE_IO_AUTO_UPDATE_BASE)) {
		/* Latest version confirmed. No need to update */
		return;
	}
	std::string s;
	std::cerr << "New version detected. Would you like to update? [y/N]: " << std::flush;
	fd_set readSet;
	FD_ZERO(&readSet);
	FD_SET(STDIN_FILENO, &readSet);
	struct timeval tv = {10, 0};  // 10 seconds, 0 microseconds;
	if (select(STDIN_FILENO+1, &readSet, NULL, NULL, &tv) < 0) {
		std::cerr << std::endl;
		std::cerr << "AutoUpdate Failed: select failed" << std::endl;
		return;
	}
	if (!(FD_ISSET(STDIN_FILENO, &readSet))) {
		std::cerr << std::endl;
		std::cerr << "10 seconds timed out. Will not update." << std::endl;
		return;
	}
	std::getline(std::cin, s);
	s.erase(s.begin(), std::find_if(s.begin(), s.end(),
					std::not1(std::ptr_fun<int, int>(std::isspace))));
	s.erase(std::find_if(s.rbegin(), s.rend(),
			     std::not1(std::ptr_fun<int, int>(std::isspace))).base(), s.end());
	if ("Y" != s && "y" != s) {
		/* The user refused to update */
		return;
	}
	std::string url { found };
	std::cerr << "Downloading from " << url << std::endl;
	if (url.size() >= 8 && "https://" == url.substr(0,8)) {
		std::cerr << "AutoUpdate Failed: HTTPS is not supported yet." << std::endl;
		std::cerr << "Pull requests are welcome on GitHub at" << std::endl;
		std::cerr << "https://github.com/pmq20/node-compiler" << std::endl;
		return;
	}
	if (url.size() >= 7 && "http://" != url.substr(0,7)) {
		std::cerr << "AutoUpdate Failed: failed to find http:// at the beginning of URL " << url << std::endl;
		return;
	}
	std::string host = url.substr(7);
	std::size_t found_slash = host.find('/');
	std::string request_path;
	if (std::string::npos == found_slash) {
		request_path = '/';
	} else {
		request_path = host.substr(found_slash);
		host = host.substr(0, found_slash);
	}
	std::size_t found_colon = host.find(':');
	int port;
	if (std::string::npos == found_colon) {
		port = 80;
	} else {
		port = std::stoi(host.substr(found_colon + 1));
		host = host.substr(0, found_colon);
	}
	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if (sockfd < 0) {
		std::cerr << "AutoUpdate Failed: socket creation failed" << std::endl;
		return;
	}
	server = gethostbyname(host.c_str());
	if (server == NULL) {
		close(sockfd);
		std::cerr << "AutoUpdate Failed: gethostbyname failed for " << host << std::endl;
		return;
	}
	memset(&serv_addr, 0, sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_port = htons(port);
	memcpy(&serv_addr.sin_addr.s_addr, server->h_addr, server->h_length);
	if (connect(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
		close(sockfd);
		std::cerr << "AutoUpdate Failed: connect failed on " << host << " and port " << port << std::endl;
		return;
	}
	if (4 != write(sockfd, "GET ", 4) ||
	    request_path.size() != write(sockfd, request_path.c_str(), request_path.size()) ||
	    11 != write(sockfd, " HTTP/1.0\r\n", 11) ||
	    6 != write(sockfd, "Host: ", 6) ||
	    host.size() != write(sockfd, host.c_str(), host.size()) ||
	    4 != write(sockfd, "\r\n\r\n", 4)) {
		close(sockfd);
		std::cerr << "AutoUpdate Failed: write failed" << std::endl;
		return;
	}
	// Read the header
	total = sizeof(response) - 1;
	response[sizeof(response) - 1] = 0;
	received = 0;
	char *header_end = NULL;
	do {
		bytes = read(sockfd, response + received, total - received);
		if (bytes < 0) {
			close(sockfd);
			std::cerr << "AutoUpdate Failed: read failed" << std::endl;
			return;
		}
		if (bytes == 0) {
			/* EOF */
			*(response + received) = 0;
			break;
		}
		header_end = strnstr(response + received, "\r\n\r\n", bytes);
		if (header_end) {
			received += bytes;
			break;
		} else {
			received += bytes;
		}
	} while (received < total);
	if (received == total) {
		close(sockfd);
		std::cerr << "AutoUpdate Failed: read causes buffer full" << std::endl;
		return;
	}
	if (NULL == header_end) {
		close(sockfd);
		std::cerr << "AutoUpdate Failed: failed to find the end of the response header" << std::endl;
		return;
	}
	assert(received < total);
	// Parse the header
	len = received;
	assert(len <= total);
	new_line = NULL;
	long long found_length = -1;
	i = 0;
	while (i < len) {
		new_line = strnstr(response + i, "\r\n", len - i);
		if (NULL == new_line) {
			break;
		}
		*new_line = 0;
		if (0 == strncmp(response + i, "Content-Length: ", 16)) {
			found_length = std::stoll(response + i + 16);
			break;
		}
		*new_line = '\n';
		i = new_line - response + 2;
	}
	if (-1 == found_length) {
		close(sockfd);
		std::cerr << "AutoUpdate Failed: failed to find a Content-Length header" << std::endl;
		return;
	}
	if (0 == found_length) {
		close(sockfd);
		std::cerr << "AutoUpdate Failed: found a Content-Length header of zero" << std::endl;
		return;
	}
	assert(found_length > 0);
	// Read the body
	// header_end -> \r\n\r\n
	assert(header_end);
	assert(header_end + 4 <= response + received);
	// put the rest of over-read content when reading header
	size_t the_rest = response + received - (header_end + 4);
	char *body_buffer = static_cast<char *>(malloc(found_length));
	if (NULL == body_buffer) {
		close(sockfd);
		std::cerr << "AutoUpdate Failed: Insufficient memory" << std::endl;
		return;
	}
	memcpy(body_buffer, (header_end + 4), the_rest);
	char *body_buffer_ptr = body_buffer + the_rest;
	char *body_buffer_end = body_buffer + found_length;
	// read the remaining body
	received = the_rest;
	std::cerr << '\r' << received << " / " << found_length << " bytes finished (" << received*100/found_length << "%)";
	while (received < found_length) {
		size_t space = 100 * 1024;
		if (space > body_buffer_end - body_buffer_ptr) {
			space = body_buffer_end - body_buffer_ptr;
		}
		bytes = read(sockfd, body_buffer_ptr, space);
		if (bytes < 0) {
			std::cerr << "AutoUpdate Failed: read failed" << std::endl;
			free(body_buffer);
			close(sockfd);
			return;
		}
		if (bytes == 0) {
			/* EOF */
			break;
		}
		received += bytes;
		body_buffer_ptr += bytes;
		std::cerr << '\r' << received << " / " << found_length << " bytes finished (" << received*100/found_length << "%)";
	}
	if (received != found_length) {
		assert(received < found_length);
		std::cerr << "AutoUpdate Failed: prematurely reached EOF after reading " << received << " bytes" << std::endl;
		close(sockfd);
		free(body_buffer);
		return;
	}
	std::cerr << std::endl;
	close(sockfd);
	// Inflate to a file
	std::cerr << "Inflating" << std::flush;
	unsigned full_length = found_length;
	unsigned half_length = found_length / 2;
	unsigned uncompLength = full_length;
	char* uncomp = (char*) calloc( sizeof(char), uncompLength );
	if (NULL == uncomp) {
		std::cerr << "AutoUpdate Failed: Insufficient memory" << std::endl;
		free(body_buffer);
		return;
	}

	z_stream strm;
	strm.next_in = (z_const Bytef *)body_buffer;
	strm.avail_in = found_length;
	strm.total_out = 0;
	strm.zalloc = Z_NULL;
	strm.zfree = Z_NULL;
	
	bool done = false ;

	if (inflateInit2(&strm, (16+MAX_WBITS)) != Z_OK) {
		free(uncomp);
		free(body_buffer);
		std::cerr << "AutoUpdate Failed: inflateInit2 failed" << std::endl;
		return;
	}
	
	while (!done) {
		// If our output buffer is too small
		if (strm.total_out >= uncompLength ) {
			// Increase size of output buffer
			char* uncomp2 = (char*) calloc( sizeof(char), uncompLength + half_length );
			if (NULL == uncomp2) {
				free(uncomp);
				free(body_buffer);
				std::cerr << "AutoUpdate Failed: calloc failed" << std::endl;
				return;
			}
			memcpy( uncomp2, uncomp, uncompLength );
			uncompLength += half_length ;
			free( uncomp );
			uncomp = uncomp2 ;
		}
		
		strm.next_out = (Bytef *) (uncomp + strm.total_out);
		strm.avail_out = uncompLength - strm.total_out;
		
		// Inflate another chunk.
		int err = inflate (&strm, Z_SYNC_FLUSH);
		if (err == Z_STREAM_END) done = true;
		else if (err != Z_OK)  {
			break;
		}
	}

	if (inflateEnd (&strm) != Z_OK) {
		std::cerr << "AutoUpdate Failed: inflateInit2 failed" << std::endl;
		free(uncomp);
		free(body_buffer);
		return;
	}

	SQUASH_OS_PATH tmpdir = squash_tmpdir();
	if (NULL == tmpdir) {
		std::cerr << "AutoUpdate Failed: no temporary folder found" << std::endl;
		free(uncomp);
		free(body_buffer);
		return;
	}
	SQUASH_OS_PATH tmpf = squash_tmpf(tmpdir, NULL);
	if (NULL == tmpf) {
		std::cerr << "AutoUpdate Failed: no temporary file found" << std::endl;
		free((void*)(tmpdir));
		free(uncomp);
		free(body_buffer);
		return;
	}
	FILE *fp = fopen(tmpf, "wb");
	if (NULL == fp) {
		std::cerr << "AutoUpdate Failed: cannot open temporary file " << tmpf << std::endl;
		free((void*)(tmpdir));
		free((void*)(tmpf));
		free(uncomp);
		free(body_buffer);
		return;
	}
	std::cerr << " to " << tmpf << std::endl;
	size_t fwrite_ret = fwrite(uncomp, sizeof(char), strm.total_out, fp);
	if (fwrite_ret != strm.total_out) {
		std::cerr << "AutoUpdate Failed: fwrite failed " << tmpf << std::endl;
		fclose(fp);
		unlink(tmpf);
		free((void*)(tmpdir));
		free((void*)(tmpf));
		free(uncomp);
		free(body_buffer);
		return;
	}
	fclose(fp);
	free(uncomp);
	free(body_buffer);
	// chmod
	size_t exec_path_len = 2 * PATH_MAX;
	char* exec_path = static_cast<char*>(malloc(exec_path_len));
	if (uv_exepath(exec_path, &exec_path_len) != 0) {
		if (!argv[0]) {
			std::cerr << "AutoUpdate Failed: missing argv[0]" << std::endl;
			free((void*)(tmpdir));
			free((void*)(tmpf));
			unlink(tmpf);
			return;
		}
		assert(strlen(argv[0]) < 2 * PATH_MAX);
		memcpy(exec_path, argv[0], strlen(argv[0]));
	}
	struct stat current_st;
	int ret = stat(exec_path, &current_st);
	if (0 != ret) {
		std::cerr << "AutoUpdate Failed: stat failed for " << exec_path << std::endl;
		free(exec_path);
		free((void*)(tmpdir));
		free((void*)(tmpf));
		unlink(tmpf);
		return;
	}
	ret = chmod(tmpf, current_st.st_mode | S_IXUSR);
	if (0 != ret) {
		std::cerr << "AutoUpdate Failed: chmod failed for " << tmpf << std::endl;
		free(exec_path);
		free((void*)(tmpdir));
		free((void*)(tmpf));
		unlink(tmpf);
		return;
	}
	// move
	std::cerr << "Moving " << tmpf << " to " << exec_path << std::endl;
	ret = rename(tmpf, exec_path);
	if (0 != ret) {
		std::cerr << "AutoUpdate Failed: failed calling rename" << tmpf << " to " << exec_path << std::endl;
		free(exec_path);
		free((void*)(tmpdir));
		free((void*)(tmpf));
		unlink(tmpf);
		return;
	}
	std::cerr << "Restarting" << std::endl;
	ret = execv(exec_path, argv);
	// we should not reach this point
	std::cerr << "AutoUpdate Failed: execv failed" << std::endl;
	free(exec_path);
	free((void*)(tmpdir));
	free((void*)(tmpf));
	unlink(tmpf);
}
#endif // ENCLOSE_IO_AUTO_UPDATE
#endif
