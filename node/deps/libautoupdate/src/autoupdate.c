/*
 * Copyright (c) 2017 - 2020 Minqi Pan et al.
 *
 * This file is part of libautoupdate, distributed under the MIT License
 * For full terms see the included LICENSE file
 */

#include "autoupdate.h"
#include "autoupdate_internal.h"
#include "zlib.h"

#ifdef _WIN32

#include <assert.h>
#include <string.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdio.h>
#include <conio.h>
#include <stdint.h>
#include <stdlib.h> /* exit */
#include <wchar.h>

int autoupdate(
	int argc,
	wchar_t *wargv[],
	const char *host,
	const char *port,
	const char *path,
	const char *current,
	short force
)
{
	WSADATA wsaData;

	if (!force && !autoupdate_should_proceed()) {
		return 1;
	}

	if (!force && !autoupdate_should_proceed_24_hours(argc, wargv, 0)) {
		return 4;
	}

	// Initialize Winsock
	int iResult = WSAStartup(MAKEWORD(2,2), &wsaData);
	if (iResult != 0) {
		fprintf(stderr, "Auto-update Failed: WSAStartup failed with %d\n", iResult);
		return 2;
	}

	struct addrinfo *result = NULL,
		*ptr = NULL,
		hints;

	ZeroMemory(&hints, sizeof(hints));
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;

	// Resolve the server address and port
	iResult = getaddrinfo(host, port, &hints, &result);
	if (iResult != 0) {
		fprintf(stderr, "Auto-update Failed: getaddrinfo failed with %d\n", iResult);
		WSACleanup();
		return 2;
	}

	SOCKET ConnectSocket = INVALID_SOCKET;

	// Attempt to connect to the first address returned by
	// the call to getaddrinfo
	ptr = result;

	// Create a SOCKET for connecting to server
	ConnectSocket = socket(ptr->ai_family, ptr->ai_socktype,
		ptr->ai_protocol);

	if (ConnectSocket == INVALID_SOCKET) {
		fprintf(stderr, "Auto-update Failed: Error at socket() with %d\n", WSAGetLastError());
		freeaddrinfo(result);
		WSACleanup();
		return 2;
	}

	// Connect to server.
	iResult = connect(ConnectSocket, ptr->ai_addr, (int)ptr->ai_addrlen);
	if (iResult == SOCKET_ERROR) {
		closesocket(ConnectSocket);
		ConnectSocket = INVALID_SOCKET;
	}
	freeaddrinfo(result);
	if (ConnectSocket == INVALID_SOCKET) {
		fprintf(stderr, "Auto-update Failed: connect failed on %s and port %s\n", host, port);
		WSACleanup();
		return 2;
	}
	if (5 != send(ConnectSocket, "HEAD ", 5, 0) ||
		strlen(path) != send(ConnectSocket, path, strlen(path), 0) ||
		11 != send(ConnectSocket, " HTTP/1.0\r\n", 11, 0) ||
		6 != send(ConnectSocket, "Host: ", 6, 0) ||
		strlen(host) != send(ConnectSocket, host, strlen(host), 0) ||
		4 != send(ConnectSocket, "\r\n\r\n", 4, 0)) {
			fprintf(stderr, "Auto-update Failed: send failed with %d\n", WSAGetLastError());
			closesocket(ConnectSocket);
			WSACleanup();
			return 2;
	}

	char response[1024 * 10 + 1]; // 10KB
	int bytes, total;
	total = sizeof(response) - 2;
	long long received = 0;
	do {
		bytes = recv(ConnectSocket, response + received, total - received, 0);
		if (bytes < 0) {
			fprintf(stderr, "Auto-update Failed: recv failed with %d\n", WSAGetLastError());
			closesocket(ConnectSocket);
			WSACleanup();
			return 2;
		}
		if (bytes == 0) {
			/* EOF */
			*(response + received) = 0;
			break;
		}
		received += bytes;
	} while (received < total);
	if (received == total) {
		fprintf(stderr, "Auto-update Failed: read causes buffer full\n");
		closesocket(ConnectSocket);
		WSACleanup();
		return 2;
	}

	// shutdown the connection for sending since no more data will be sent
	// the client can still use the ConnectSocket for receiving data
	iResult = shutdown(ConnectSocket, SD_SEND);
	if (iResult == SOCKET_ERROR) {
		fprintf(stderr, "Auto-update Failed: shutdown failed with %d\n", WSAGetLastError());
		closesocket(ConnectSocket);
		WSACleanup();
		return 2;
	}

	assert(received < total);
	size_t len = strlen(response);
	short again_302 = 0;
parse_location_header:
	assert(len <= total);
	char *new_line = NULL;
	char *found = NULL;
	size_t i = 0;
	response[sizeof(response) - 1] = 0;
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
		*new_line = '\r';
		i = new_line - response + 2;
	}
	if (!found) {
		fprintf(stderr, "Auto-update Failed: failed to find a Location header\n");
		return 2;
	}
	if (!again_302) {
		if (strstr(found, current)) {
			/* Latest version confirmed. No need to update */
			autoupdate_should_proceed_24_hours(argc, wargv, 1);
			return 0;
		} else {
			fprintf(stderr, "Hint: to disable auto-update, run with environment variable CI=true\n");
			fflush(stderr);
		}
	}

	char *url = found;
	fprintf(stderr, "Downloading update from %s\n", url);
	fflush(stderr);

	char *host2;
	char *port2 = "80";
	if (strlen(url) >= 8 && 0 == strncmp("https://", url, 8)) {
		host2 = url + 8;
	} else if (strlen(url) >= 7 && 0 == strncmp("http://", url, 7)) {
		host2 = url + 7;
	} else {
		fprintf(stderr, "Auto-update Failed: failed to find http:// or https:// at the beginning of URL %s\n", url);
		return 2;
	}
	char *found_slash = strchr(host2, '/');
	char *request_path;
	if (NULL == found_slash) {
		request_path = "/";
	} else {
		request_path = found_slash;
		*found_slash = 0;
	}

	result = NULL;
	ptr = NULL;
	ZeroMemory(&hints, sizeof(hints));
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;

	// Resolve the server address and port
	iResult = getaddrinfo(host2, port2, &hints, &result);
	if (iResult != 0) {
		fprintf(stderr, "Auto-update Failed: getaddrinfo failed with %d\n", iResult);
		WSACleanup();
		return 2;
	}

	ConnectSocket = INVALID_SOCKET;

	// Attempt to connect to the first address returned by
	// the call to getaddrinfo
	ptr = result;

	// Create a SOCKET for connecting to server
	ConnectSocket = socket(ptr->ai_family, ptr->ai_socktype, ptr->ai_protocol);

	if (ConnectSocket == INVALID_SOCKET) {
		fprintf(stderr, "Auto-update Failed: Error at socket() with %d\n", WSAGetLastError());
		freeaddrinfo(result);
		WSACleanup();
		return 2;
	}
	// Connect to server.
	iResult = connect(ConnectSocket, ptr->ai_addr, (int)ptr->ai_addrlen);
	if (iResult == SOCKET_ERROR) {
		closesocket(ConnectSocket);
		ConnectSocket = INVALID_SOCKET;
	}
	freeaddrinfo(result);
	if (ConnectSocket == INVALID_SOCKET) {
		fprintf(stderr, "Auto-update Failed: connect failed on %s and port %s\n", host2, port2);
		WSACleanup();
		return 2;
	}
	if (NULL != found_slash) {
		*found_slash = '/';
	}
	if (4 != send(ConnectSocket, "GET ", 4, 0) ||
		strlen(request_path) != send(ConnectSocket, request_path, strlen(request_path), 0) ||
		11 != send(ConnectSocket, " HTTP/1.0\r\n", 11, 0)) {
			fprintf(stderr, "Auto-update Failed: send failed with %d\n", WSAGetLastError());
			closesocket(ConnectSocket);
			WSACleanup();
			return 2;
	}
	if (NULL != found_slash) {
		*found_slash = 0;
	}
	if (6 != send(ConnectSocket, "Host: ", 6, 0) ||
		strlen(host2) != send(ConnectSocket, host2, strlen(host2), 0) ||
		4 != send(ConnectSocket, "\r\n\r\n", 4, 0)) {
			fprintf(stderr, "Auto-update Failed: send failed with %d\n", WSAGetLastError());
			closesocket(ConnectSocket);
			WSACleanup();
			return 2;
	}

	// Read the header
	total = sizeof(response) - 2;
	response[sizeof(response) - 1] = 0;
	received = 0;
	char *header_end = NULL;
	do {
		bytes = recv(ConnectSocket, response + received, total - received, 0);
		if (bytes < 0) {
			fprintf(stderr, "Auto-update Failed: recv failed with %d\n", WSAGetLastError());
			closesocket(ConnectSocket);
			WSACleanup();
			return 2;
		}
		if (bytes == 0) {
			/* EOF */
			*(response + received) = 0;
			break;
		}
		*(response + received + bytes) = 0;
		header_end = strstr(response + received, "\r\n\r\n");
		received += bytes;
		if (header_end) {
			break;
		}
	} while (received < total);
	if (NULL == header_end) {
		fprintf(stderr, "Auto-update Failed: failed to find the end of the response header\n");
		closesocket(ConnectSocket);
		WSACleanup();
		return 2;
	}
	assert(received <= total);

	// Possible new 302
	if (received > 13 && (
		0 == strncmp(response, "HTTP/1.1 302 ", 13) ||
		0 == strncmp(response, "HTTP/1.0 302 ", 13))) {
			len = received;
			again_302 = 1;
			goto parse_location_header;
	}

	// Parse the header
	len = received;
	assert(len <= total);
	new_line = NULL;
	long long found_length = -1;
	i = 0;
	response[sizeof(response) - 1] = 0;
	while (i < len) {
		new_line = strstr(response + i, "\r\n");
		if (NULL == new_line) {
			break;
		}
		*new_line = 0;
		if (0 == strncmp(response + i, "Content-Length: ", 16)) {
			found_length = atoll(response + i + 16);
			break;
		}
		*new_line = '\r';
		i = new_line - response + 2;
	}
	if (-1 == found_length) {
		fprintf(stderr, "Auto-update Failed: failed to find a Content-Length header\n");
		closesocket(ConnectSocket);
		WSACleanup();
		return 2;
	}
	if (0 == found_length) {
		fprintf(stderr, "Auto-update Failed: found a Content-Length header of zero\n");
		closesocket(ConnectSocket);
		WSACleanup();
		return 2;
	}
	assert(found_length > 0);
	// Read the body
	// header_end -> \r\n\r\n
	assert(header_end);
	assert(header_end + 4 <= response + received);
	// put the rest of over-read content when reading header
	size_t the_rest = response + received - (header_end + 4);
	char *body_buffer = (char *)(malloc(found_length));
	if (NULL == body_buffer) {
		fprintf(stderr, "Auto-update Failed: Insufficient memory\n");
		closesocket(ConnectSocket);
		WSACleanup();
		return 2;
	}
	memcpy(body_buffer, (header_end + 4), the_rest);
	char *body_buffer_ptr = body_buffer + the_rest;
	char *body_buffer_end = body_buffer + found_length;
	// read the remaining body
	received = the_rest;
	fprintf(stderr, "\r%lld / %lld bytes finished (%lld%%)",  received, found_length, received*100LL/found_length);
	fflush(stderr);
	while (received < found_length) {
		size_t space = 100 * 1024;
		if (space > body_buffer_end - body_buffer_ptr) {
			space = body_buffer_end - body_buffer_ptr;
		}
		bytes = recv(ConnectSocket, body_buffer_ptr, space, 0);
		if (bytes < 0) {
			fprintf(stderr, "Auto-update Failed: read failed\n");
			free(body_buffer);
			closesocket(ConnectSocket);
			WSACleanup();
			return 2;
		}
		if (bytes == 0) {
			/* EOF */
			break;
		}
		received += bytes;
		body_buffer_ptr += bytes;
		fprintf(stderr, "\r%lld / %lld bytes finished (%lld%%)",  received, found_length, received*100LL/found_length);
		fflush(stderr);
	}
	if (received != found_length) {
		assert(received < found_length);
		fprintf(stderr, "Auto-update Failed: prematurely reached EOF after reading %lld bytes\n", received);
		closesocket(ConnectSocket);
		WSACleanup();
		free(body_buffer);
		return 2;
	}
	fprintf(stderr, "\n");
	fflush(stderr);
	// shutdown the connection for sending since no more data will be sent
	// the client can still use the ConnectSocket for receiving data
	iResult = shutdown(ConnectSocket, SD_SEND);
	if (iResult == SOCKET_ERROR) {
		fprintf(stderr, "Auto-update Failed: shutdown failed with %d\n", WSAGetLastError());
		closesocket(ConnectSocket);
		WSACleanup();
		return 2;
	}
	// Inflate to a file
	fprintf(stderr, "Inflating");
	fflush(stderr);
	struct ZIPLocalFileHeader *h = (struct ZIPLocalFileHeader *)body_buffer;
	if (!(0x04034b50 == h->signature && 8 == h->compressionMethod)) {
		fprintf(stderr, "Auto-update Failed: We only support a zip file containing "
			"one Deflate compressed file for the moment.\n"
			"Pull requests are welcome on GitHub at "
			"https://github.com/pmq20/libautoupdate\n");
	}
	// skip the Local File Header
	unsigned full_length = found_length - sizeof(struct ZIPLocalFileHeader) - h->fileNameLength;
	unsigned half_length = full_length / 2;
	unsigned uncompLength = full_length;

	/* windowBits is passed < 0 to tell that there is no zlib header.
	* Note that in this case inflate *requires* an extra "dummy" byte
	* after the compressed stream in order to complete decompression and
	* return Z_STREAM_END.
	*/
	char* uncomp = (char*)calloc(sizeof(char), uncompLength + 1);
	if (NULL == uncomp) {
		fprintf(stderr, "Auto-update Failed: Insufficient memory\n");
		free(body_buffer);
		return 2;
	}

	z_stream strm;
	strm.next_in = (Bytef *)(body_buffer + sizeof(struct ZIPLocalFileHeader) + h->fileNameLength);
	strm.avail_in = found_length;
	strm.total_out = 0;
	strm.zalloc = Z_NULL;
	strm.zfree = Z_NULL;

	short done = 0;

	if (inflateInit2(&strm, -MAX_WBITS) != Z_OK) {
		free(uncomp);
		free(body_buffer);
		fprintf(stderr, "Auto-update Failed: inflateInit2 failed\n");
		return 2;
	}

	while (!done) {
		// If our output buffer is too small
		if (strm.total_out >= uncompLength) {
			// Increase size of output buffer
			char* uncomp2 = (char*)calloc(sizeof(char), uncompLength + half_length + 1);
			if (NULL == uncomp2) {
				free(uncomp);
				free(body_buffer);
				fprintf(stderr, "Auto-update Failed: calloc failed\n");
				return 2;
			}
			memcpy(uncomp2, uncomp, uncompLength);
			uncompLength += half_length;
			free(uncomp);
			uncomp = uncomp2;
		}

		strm.next_out = (Bytef *)(uncomp + strm.total_out);
		strm.avail_out = uncompLength - strm.total_out;

		// Inflate another chunk.
		int err = inflate(&strm, Z_SYNC_FLUSH);
		if (err == Z_STREAM_END) {
			done = 1;
		}
		else if (err != Z_OK) {
			fprintf(stderr, "Auto-update Failed: inflate failed with %d\n", err);
			free(uncomp);
			free(body_buffer);
			return 2;
		}
	}

	if (inflateEnd(&strm) != Z_OK) {
		fprintf(stderr, "Auto-update Failed: inflateInit2 failed\n");
		free(uncomp);
		free(body_buffer);
		return 2;
	}

	wchar_t *tmpdir = autoupdate_tmpdir();
	if (NULL == tmpdir) {
		fprintf(stderr, "Auto-update Failed: no temporary folder found\n");
		free(uncomp);
		free(body_buffer);
		return 2;
	}
	/* Windows paths can never be longer than this. */
	const size_t exec_path_len = 32768;
	wchar_t exec_path[32768];
	DWORD utf16_len = GetModuleFileNameW(NULL, exec_path, exec_path_len);
	if (0 == utf16_len) {
		fprintf(stderr, "Auto-update Failed: GetModuleFileNameW failed with GetLastError=%d\n", GetLastError());
		free((void*)(tmpdir));
		free(uncomp);
		free(body_buffer);
		return 2;
	}
	if (tmpdir[0] != exec_path[0]) {
		free((void*)(tmpdir));
		tmpdir = wcsdup(exec_path);
		wchar_t *backslash = wcsrchr(tmpdir, L'\\');
		if (NULL == backslash) {
			fprintf(stderr, "Auto-update Failed: Cannot find an approriate tmpdir with %S\n", tmpdir);
			free((void*)(tmpdir));
			free(uncomp);
			free(body_buffer);
			return 2;
		}
		*backslash = 0;
	}
	wchar_t *tmpf = autoupdate_tmpf(tmpdir, "exe");
	if (NULL == tmpf) {
		fprintf(stderr, "Auto-update Failed: no temporary file available\n");
		free((void*)(tmpdir));
		free(uncomp);
		free(body_buffer);
		return 2;
	}
	FILE *fp = _wfopen(tmpf, L"wb");
	if (NULL == fp) {
		fprintf(stderr, "Auto-update Failed: cannot open temporary file %S\n", tmpf);
		free((void*)(tmpdir));
		free((void*)(tmpf));
		free(uncomp);
		free(body_buffer);
		return 2;
	}
	fprintf(stderr, " to %S\n", tmpf);
	size_t fwrite_ret = fwrite(uncomp, sizeof(char), strm.total_out, fp);
	if (fwrite_ret != strm.total_out) {
		fprintf(stderr, "Auto-update Failed: fwrite failed %S\n", tmpf);
		fclose(fp);
		DeleteFileW(tmpf);
		free((void*)(tmpdir));
		free((void*)(tmpf));
		free(uncomp);
		free(body_buffer);
		return 2;
	}
	fclose(fp);
	free(uncomp);
	free(body_buffer);
	// Backup
	wchar_t *selftmpf = autoupdate_tmpf(tmpdir, "exe");
	if (NULL == selftmpf) {
		fprintf(stderr, "Auto-update Failed: no temporary file available\n");
		DeleteFileW(tmpf);
		free((void*)(tmpdir));
		free((void*)(tmpf));
		return 2;
	}
	fprintf(stderr, "Moving the old version from %S to %S\n", exec_path, selftmpf);
	BOOL ret = MoveFileExW(exec_path, selftmpf, MOVEFILE_COPY_ALLOWED | MOVEFILE_WRITE_THROUGH);
	if (!ret) {
		fprintf(stderr, "Auto-update Failed: MoveFileW failed with GetLastError=%d\n", GetLastError());
		DeleteFileW(tmpf);
		free((void*)(tmpdir));
		free((void*)(tmpf));
		free((void*)(selftmpf));
		return 2;
	}
	// Move the new version into the original place
	fprintf(stderr, "Moving the new version from %S to %S \n", tmpf, exec_path);
	ret = MoveFileExW(tmpf, exec_path, MOVEFILE_COPY_ALLOWED | MOVEFILE_WRITE_THROUGH);
	if (!ret) {
		fprintf(stderr, "Auto-update Failed: MoveFileW failed with GetLastError=%d\n", GetLastError());
		DeleteFileW(tmpf);
		free((void*)(tmpdir));
		free((void*)(tmpf));
		free((void*)(selftmpf));
		return 2;
	}
	// Restarting
	fprintf(stderr, "Restarting...\n");
	fflush(stderr);
	STARTUPINFO si;
	PROCESS_INFORMATION pi;
	ZeroMemory(&si, sizeof(si));
	si.cb = sizeof(si);
	ZeroMemory(&pi, sizeof(pi));
	ret = CreateProcess(
		NULL,	     // No module name (use command line)
		GetCommandLine(), // Command line
		NULL,	     // Process handle not inheritable
		NULL,	     // Thread handle not inheritable
		FALSE,	    // Set handle inheritance to FALSE
		0,		// No creation flags
		NULL,	     // Use parent's environment block
		NULL,	     // Use parent's starting directory 
		&si,	      // Pointer to STARTUPINFO structure
		&pi	       // Pointer to PROCESS_INFORMATION structure
	);
	if (!ret) {
		fprintf(stderr, "Auto-update Failed: CreateProcess failed with GetLastError=%d\n", GetLastError());
		DeleteFileW(tmpf);
		free((void*)(tmpdir));
		free((void*)(tmpf));
		free((void*)(selftmpf));
		return 3;
	}
	// Wait until child process exits.
	WaitForSingleObject(pi.hProcess, INFINITE);
	// Close process and thread handles. 
	CloseHandle(pi.hProcess);
	CloseHandle(pi.hThread);
	fprintf(stderr, "Deleting %S\n", selftmpf);
	fflush(stderr);
	_wexeclp(L"cmd", L"cmd", L"/c", L"ping", L"127.0.0.1", L"-n", L"3", L">nul", L"&", L"del", selftmpf, NULL);
	// we should never reach here
	assert(0);
	return 3;
}

#else

#include <assert.h>
#include <stdio.h> /* printf, sprintf */
#include <stdlib.h> /* exit */
#include <unistd.h> /* read, write, close */
#include <string.h> /* memcpy, memset */
#include <sys/socket.h> /* socket, connect */
#include <netinet/in.h> /* struct sockaddr_in, struct sockaddr */
#include <netdb.h> /* struct hostent, gethostbyname */
#include <unistd.h>
#include <sys/select.h>
#include <limits.h>  /* PATH_MAX */
#include <sys/stat.h> /* struct stat */
#include <errno.h>

int autoupdate(
	int argc,
	char *argv[],
	const char *host,
	uint16_t port,
	const char *path,
	const char *current,
	short force
)
{
	struct hostent *server;
	struct sockaddr_in serv_addr;
	int sockfd, bytes, total;
	char response[1024 * 10 + 1]; // 10KB

	if (!force && !autoupdate_should_proceed()) {
		return 1;
	}

	if (!force && !autoupdate_should_proceed_24_hours(argc, argv, 0)) {
		return 4;
	}

	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if (sockfd < 0) {
		fprintf(stderr, "Auto-update Failed: socket creation failed\n");
		return 2;
	}
	server = gethostbyname(host);
	if (server == NULL) {
		close(sockfd);
		fprintf(stderr, "Auto-update Failed: gethostbyname failed for %s\n", host);
		return 2;
	}
	memset(&serv_addr, 0, sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_port = htons(port);
	memcpy(&serv_addr.sin_addr.s_addr, server->h_addr, server->h_length);
	if (connect(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
		close(sockfd);
		fprintf(stderr, "Auto-update Failed: connect failed on %s and port %d\n", host, port);
		return 2;
	}
	if (5 != write(sockfd, "HEAD ", 5) ||
		strlen(path) != write(sockfd, path, strlen(path)) ||
		11 != write(sockfd, " HTTP/1.0\r\n", 11) ||
		6 != write(sockfd, "Host: ", 6) ||
		strlen(host) != write(sockfd, host, strlen(host)) ||
		4 != write(sockfd, "\r\n\r\n", 4)) {
			close(sockfd);
			fprintf(stderr, "Auto-update Failed: write failed\n");
			return 2;
	}
	total = sizeof(response) - 2;
	long long received = 0;
	do {
		bytes = read(sockfd, response + received, total - received);
		if (bytes < 0) {
			close(sockfd);
			fprintf(stderr, "Auto-update Failed: read failed\n");
			return 2;
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
		fprintf(stderr, "Auto-update Failed: read causes buffer full\n");
		return 2;
	}
	close(sockfd);
	assert(received < total);
	size_t len = strlen(response);
	short again_302 = 0;
parse_location_header:
	assert(len <= total);
	char *new_line = NULL;
	char *found = NULL;
	size_t i = 0;
	response[sizeof(response) - 1] = 0;
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
		*new_line = '\r';
		i = new_line - response + 2;
	}
	if (!found) {
		fprintf(stderr, "Auto-update Failed: failed to find a Location header\n");
		return 2;
	}
	if (!again_302) {
		if (strstr(found, current)) {
			/* Latest version confirmed. No need to update */
			autoupdate_should_proceed_24_hours(argc, argv, 1);
			return 0;
		} else {
			fprintf(stderr, "Hint: to disable auto-update, run with environment variable CI=true\n");
			fflush(stderr);
		}
	}

	char *url = found;
	fprintf(stderr, "Downloading update from %s\n", url);
	fflush(stderr);

	char *host2;
	uint16_t port2 = 80;
	if (strlen(url) >= 8 && 0 == strncmp("https://", url, 8)) {
		host2 = url + 8;
	} else if (strlen(url) >= 7 && 0 == strncmp("http://", url, 7)) {
		host2 = url + 7;
	} else {
		fprintf(stderr, "Auto-update Failed: failed to find http:// or https:// at the beginning of URL %s\n", url);
		return 2;
	}
	char *found_slash = strchr(host2, '/');
	char *request_path;
	if (NULL == found_slash) {
		request_path = "/";
	} else {
		request_path = found_slash;
		*found_slash = 0;
	}
	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if (sockfd < 0) {
		fprintf(stderr, "Auto-update Failed: socket creation failed\n");
		return 2;
	}
	server = gethostbyname(host2);
	if (server == NULL) {
		close(sockfd);
		fprintf(stderr, "Auto-update Failed: gethostbyname failed for %s\n", host2);
		return 2;
	}
	memset(&serv_addr, 0, sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_port = htons(port2);
	memcpy(&serv_addr.sin_addr.s_addr, server->h_addr, server->h_length);
	if (connect(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
		close(sockfd);
		fprintf(stderr, "Auto-update Failed: connect failed on %s and port %d\n", host2, port2);
		return 2;
	}
	if (NULL != found_slash) {
		*found_slash = '/';
	}
	if (4 != write(sockfd, "GET ", 4) ||
		strlen(request_path) != write(sockfd, request_path, strlen(request_path)) ||
		11 != write(sockfd, " HTTP/1.0\r\n", 11)) {
			close(sockfd);
			fprintf(stderr, "Auto-update Failed: write failed\n");
			return 2;
	}
	if (NULL != found_slash) {
		*found_slash = 0;
	}
	if (6 != write(sockfd, "Host: ", 6) ||
		strlen(host2) != write(sockfd, host2, strlen(host2)) ||
		4 != write(sockfd, "\r\n\r\n", 4)) {
			close(sockfd);
			fprintf(stderr, "Auto-update Failed: write failed\n");
			return 2;
	}

	// Read the header
	total = sizeof(response) - 2;
	response[sizeof(response) - 1] = 0;
	received = 0;
	char *header_end = NULL;
	do {
		bytes = read(sockfd, response + received, total - received);
		if (bytes < 0) {
			close(sockfd);
			fprintf(stderr, "Auto-update Failed: read failed\n");
			return 2;
		}
		if (bytes == 0) {
			/* EOF */
			*(response + received) = 0;
			break;
		}
		*(response + received + bytes) = 0;
		header_end = strstr(response + received, "\r\n\r\n");
		received += bytes;
		if (header_end) {
			break;
		}
	} while (received < total);
	if (NULL == header_end) {
		close(sockfd);
		fprintf(stderr, "Auto-update Failed: failed to find the end of the response header\n");
		return 2;
	}
	assert(received <= total);

	// Possible new 302
	if (received > 13 && (
		0 == strncmp(response, "HTTP/1.1 302 ", 13) ||
		0 == strncmp(response, "HTTP/1.0 302 ", 13))) {
			len = received;
			again_302 = 1;
			goto parse_location_header;
	}

	// Parse the header
	len = received;
	assert(len <= total);
	new_line = NULL;
	long long found_length = -1;
	i = 0;
	response[sizeof(response) - 1] = 0;
	while (i < len) {
		new_line = strstr(response + i, "\r\n");
		if (NULL == new_line) {
			break;
		}
		*new_line = 0;
		if (0 == strncmp(response + i, "Content-Length: ", 16)) {
			found_length = atoll(response + i + 16);
			break;
		}
		*new_line = '\r';
		i = new_line - response + 2;
	}
	if (-1 == found_length) {
		close(sockfd);
		fprintf(stderr, "Auto-update Failed: failed to find a Content-Length header\n");
		return 2;
	}
	if (0 == found_length) {
		close(sockfd);
		fprintf(stderr, "Auto-update Failed: found a Content-Length header of zero\n");
		return 2;
	}
	assert(found_length > 0);
	// Read the body
	// header_end -> \r\n\r\n
	assert(header_end);
	assert(header_end + 4 <= response + received);
	// put the rest of over-read content when reading header
	size_t the_rest = response + received - (header_end + 4);
	char *body_buffer = (char *)(malloc(found_length));
	if (NULL == body_buffer) {
		close(sockfd);
		fprintf(stderr, "Auto-update Failed: Insufficient memory\n");
		return 2;
	}
	memcpy(body_buffer, (header_end + 4), the_rest);
	char *body_buffer_ptr = body_buffer + the_rest;
	char *body_buffer_end = body_buffer + found_length;
	// read the remaining body
	received = the_rest;
	fprintf(stderr, "\r%lld / %lld bytes finished (%lld%%)",  received, found_length, received*100LL/found_length);
	fflush(stderr);
	while (received < found_length) {
		size_t space = 100 * 1024;
		if (space > body_buffer_end - body_buffer_ptr) {
			space = body_buffer_end - body_buffer_ptr;
		}
		bytes = read(sockfd, body_buffer_ptr, space);
		if (bytes < 0) {
			fprintf(stderr, "Auto-update Failed: read failed\n");
			free(body_buffer);
			close(sockfd);
			return 2;
		}
		if (bytes == 0) {
			/* EOF */
			break;
		}
		received += bytes;
		body_buffer_ptr += bytes;
		fprintf(stderr, "\r%lld / %lld bytes finished (%lld%%)",  received, found_length, received*100LL/found_length);
		fflush(stderr);
	}
	if (received != found_length) {
		assert(received < found_length);
		fprintf(stderr, "Auto-update Failed: prematurely reached EOF after reading %lld bytes\n", received);
		close(sockfd);
		free(body_buffer);
		return 2;
	}
	fprintf(stderr, "\n");
	fflush(stderr);
	close(sockfd);
	// Inflate to a file
	fprintf(stderr, "Inflating");
	fflush(stderr);
	unsigned full_length = found_length;
	unsigned half_length = found_length / 2;
	unsigned uncompLength = full_length;
	char* uncomp = (char*) calloc( sizeof(char), uncompLength );
	if (NULL == uncomp) {
		fprintf(stderr, "Auto-update Failed: Insufficient memory\n");
		free(body_buffer);
		return 2;
	}

	z_stream strm;
	strm.next_in = (Bytef *)body_buffer;
	strm.avail_in = found_length;
	strm.total_out = 0;
	strm.zalloc = Z_NULL;
	strm.zfree = Z_NULL;
	
	short done = 0;

	if (inflateInit2(&strm, (16+MAX_WBITS)) != Z_OK) {
		free(uncomp);
		free(body_buffer);
		fprintf(stderr, "Auto-update Failed: inflateInit2 failed\n");
		return 2;
	}
	
	while (!done) {
		// If our output buffer is too small
		if (strm.total_out >= uncompLength ) {
			// Increase size of output buffer
			char* uncomp2 = (char*) calloc( sizeof(char), uncompLength + half_length );
			if (NULL == uncomp2) {
				free(uncomp);
				free(body_buffer);
				fprintf(stderr, "Auto-update Failed: calloc failed\n");
				return 2;
			}
			memcpy( uncomp2, uncomp, uncompLength );
			uncompLength += half_length ;
			free( uncomp );
			uncomp = uncomp2 ;
		}
		
		strm.next_out = (Bytef *) (uncomp + strm.total_out);
		strm.avail_out = uncompLength - strm.total_out;
		
		// Inflate another chunk.
		int err = inflate(&strm, Z_SYNC_FLUSH);
		if (err == Z_STREAM_END) {
			done = 1;
		}
		else if (err != Z_OK)  {
			fprintf(stderr, "Auto-update Failed: inflate failed with %d\n", err);
			free(uncomp);
			free(body_buffer);
			return 2;
		}
	}

	if (inflateEnd (&strm) != Z_OK) {
		fprintf(stderr, "Auto-update Failed: inflateInit2 failed\n");
		free(uncomp);
		free(body_buffer);
		return 2;
	}

	char *tmpdir = autoupdate_tmpdir();
	if (NULL == tmpdir) {
		fprintf(stderr, "Auto-update Failed: no temporary folder found\n");
		free(uncomp);
		free(body_buffer);
		return 2;
	}
	char *tmpf = autoupdate_tmpf(tmpdir, NULL);
	if (NULL == tmpf) {
		fprintf(stderr, "Auto-update Failed: no temporary file available\n");
		free((void*)(tmpdir));
		free(uncomp);
		free(body_buffer);
		return 2;
	}
	FILE *fp = fopen(tmpf, "wb");
	if (NULL == fp) {
		fprintf(stderr, "Auto-update Failed: cannot open temporary file %s\n", tmpf);
		free((void*)(tmpdir));
		free((void*)(tmpf));
		free(uncomp);
		free(body_buffer);
		return 2;
	}
	fprintf(stderr, " to %s\n", tmpf);
	size_t fwrite_ret = fwrite(uncomp, sizeof(char), strm.total_out, fp);
	if (fwrite_ret != strm.total_out) {
		fprintf(stderr, "Auto-update Failed: fwrite failed %s\n", tmpf);
		fclose(fp);
		unlink(tmpf);
		free((void*)(tmpdir));
		free((void*)(tmpf));
		free(uncomp);
		free(body_buffer);
		return 2;
	}
	fclose(fp);
	free(uncomp);
	free(body_buffer);
	// chmod
	size_t exec_path_len = 2 * PATH_MAX;
	char* exec_path = (char*)(malloc(exec_path_len));
	if (NULL == exec_path) {
		fprintf(stderr, "Auto-update Failed: Insufficient memory allocating exec_path\n");
		free((void*)(tmpdir));
		free((void*)(tmpf));
		unlink(tmpf);
		return 2;
	}
	if (autoupdate_exepath(exec_path, &exec_path_len) != 0) {
		if (!argv[0]) {
			fprintf(stderr, "Auto-update Failed: missing argv[0]\n");
			free((void*)(tmpdir));
			free((void*)(tmpf));
			unlink(tmpf);
			return 2;
		}
		assert(strlen(argv[0]) < 2 * PATH_MAX);
		memcpy(exec_path, argv[0], strlen(argv[0]));
	}
	struct stat current_st;
	int ret = stat(exec_path, &current_st);
	if (0 != ret) {
		fprintf(stderr, "Auto-update Failed: stat failed for %s\n", exec_path);
		free(exec_path);
		free((void*)(tmpdir));
		free((void*)(tmpf));
		unlink(tmpf);
		return 2;
	}
	ret = chmod(tmpf, current_st.st_mode | S_IXUSR);
	if (0 != ret) {
		fprintf(stderr, "Auto-update Failed: chmod failed for %s\n", tmpf);
		free(exec_path);
		free((void*)(tmpdir));
		free((void*)(tmpf));
		unlink(tmpf);
		return 2;
	}
	// Move the new version into the original place
	fprintf(stderr, "Moving the new version from %s to %s\n", tmpf, exec_path);
	ret = rename(tmpf, exec_path);
	if (0 != ret) {
		fprintf(stderr, "Auto-update Failed: failed calling rename %s to %s\n", tmpf, exec_path);
		free(exec_path);
		free((void*)(tmpdir));
		free((void*)(tmpf));
		unlink(tmpf);
		return 2;
	}
	fprintf(stderr, "Restarting...\n");
	ret = execv(exec_path, argv);
	// we should not reach this point
	fprintf(stderr, "Auto-update Failed: execv failed with %d (errno %d)\n", ret, errno);
	free(exec_path);
	free((void*)(tmpdir));
	free((void*)(tmpf));
	unlink(tmpf);
	return 3;
}

#endif // _WIN32
