# Libautoupdate

Cross-platform C library that enables your application to auto-update itself in place.

[![Build Status](https://travis-ci.org/pmq20/libautoupdate.svg?branch=master)](https://travis-ci.org/pmq20/libautoupdate)
[![Build status](https://ci.appveyor.com/api/projects/status/sjdyfwd768lh187f/branch/master?svg=true)](https://ci.appveyor.com/project/pmq20/libautoupdate/branch/master)

![Terminal simulation of a simple auto-update](https://github.com/pmq20/libautoupdate/raw/master/doc/libautoupdate.gif)

## API

There is only one public API, i.e. `autoupdate()`.

```C
int autoupdate(argc, argv, host, port, path, current)
```

It accepts the following arguments:

- the 1st and 2nd arguments are the same as those passed to `main()`
- `host` is the host name of the update server to communicate with
- `port` is the port of the server, which is a string on Windows and a 16-bit integer on macOS / Linux
- `path` is the paramater passed to the HTTP/1.0 HEAD request of the Round 1
- `current` is the current version string to be compared with what is returned from the server
  - a new version is considered detected if this string is not a substring of the server's reply

It never returns if a new version was detected and auto-update was successfully performed.
Otherwise, it returns one of the following integers to indicate the situation:

|  Return Value  | Indication                                                                                  |
|:--------------:|---------------------------------------------------------------------------------------------|
|        0       | Latest version confirmed. No need to update                                                 |
|        1       | Auto-update shall not proceed due to environment variable `CI` being set                    |
|        2       | Auto-update process failed prematurely and detailed errors are printed to stderr            |
|        3       | Failed to restart after replacing itself with the new version                               |
|        4       | Auto-update shall not proceed due to being already checked in the last 24 hours             |

## Communication

### Round 1

Libautoupdate first makes a HTTP/1.0 HEAD request to the server, in order to peek the latest version string.

    Libautoupdate -- HTTP/1.0 HEAD request --> Server

The server is expected to repond with `HTTP 302 Found` and provide a `Location` header.

It then compares the content of `Location` header with the current version string.
It proceeds to Round 2 if the current version string is NOT a sub-string of the `Location` header.

### Round 2

Libautoupdate makes a full HTTP/1.0 GET request to the `Location` header of the last round.

    Libautoupdate -- HTTP/1.0 GET request --> Server

The server is expected to respond with `200 OK` transferring the new release itself.

Based on the `Content-Type` header received, an addtional inflation operation might be performed:
- `Content-Type: application/x-gzip`: Gzip Inflation is performed
- `Content-Type: application/zip`: Deflate compression is assumed and the first file is inflated and used

## Self-replacing

After 2 rounds of communication with the server,
libautoupdate will then proceeds with a self-replacing process,
i.e. the program replaces itself in-place with the help of the system temporary directory,
after which it restarts itself with the new release.

## Examples

Just call `autoupdate()` at the beginning of your `main()`,
before all actual logic of your application.
See the following code for examples.

### Windows

```C
#include <autoupdate.h>

int wmain(int argc, wchar_t *wargv[])
{
	int autoupdate_result;

	autoupdate_result = autoupdate(
		argc,
		wargv,
		"enclose.io",
		"80",
		"/nodec/nodec-x64.exe"
		"v1.1.0"
	);

	/* 
		actual logic of your application
		...
	*/
}
```

### macOS / Linux

```C
#include <autoupdate.h>

int main(int argc, char *argv[])
{
	int autoupdate_result;

	autoupdate_result = autoupdate(
		argc,
		argv,
		"enclose.io",
		80,
		"/nodec/nodec-darwin-x64"
		"v1.1.0"
	);

	/* 
		actual logic of your application
		...
	*/
}
```

## Hints

- Set environment variable `CI=true` will prevent auto-updating
- Remove the file `~/.libautoupdate` will remove the once-per-24-hours check limit

## To-do

- Cater to bad network connection situations
- Print more information about the new version
- Use better error messages when the user did not have permissions to move the new version into the destination directory
- Move the old binary to the system temporary directory, yet not deleting it.
  - The Operating System will delete it when restarted/tmpdir-full
  - Add facility to restore/rollback to the old file once the new version went wrong

## Authors

[Minqi Pan et al.](https://raw.githubusercontent.com/pmq20/libautoupdate/master/AUTHORS)

## License

[MIT](https://raw.githubusercontent.com/pmq20/libautoupdate/master/LICENSE)

## See Also

- [Node.js Packer](https://github.com/pmq20/node-packer): Packing your Node.js application into a single executable.
- [Ruby Packer](https://github.com/pmq20/ruby-packer): Packing your Ruby application into a single executable.
