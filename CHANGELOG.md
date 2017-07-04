# Node.js Compiler Changelog

## v1.3.0

- add options to generate installers
  - add `--msi`: generates a msi installer for Windows

## v1.2.0

- upgrade Node.js runtime to v8.1.3
  - https://nodejs.org/en/blog/release/v8.1.3/
- start using libautoupdate to handle auto-update
  - https://github.com/pmq20/libautoupdate
  - remove user prompts when a new version is detected
  - keep the old version in the system temporary directory as a backup
  - send header Host in Round 1
  - handle possible new 302 responses after Round 1
- upgrade libsquash to v0.6.0
  - add `enclose_io_ifextract(const char* path, const char* ext_name)`
  - add `enclose_io_if(const char* path)`
  - fix a NULL-dereferencing in `EncloseIOFindFirstFileHelper`
  - convert macro `ENCLOSE_IO_DOS_RETURN` to a function
  - rename `squash_global_fdtable_mutex` to `squash_global_mutex`
  - intercept `mkdir()`
    - redirect `mkdir()` inside the memfs to a temporary directory
    - removes the temporary directory and files at exit
  - intercept `open()` with `O_CREAT`
    - redirect `open()` with `O_CREAT` inside the memfs to a temporary directory
    - removes the temporary directory and files at exit
  - handle the case where `bin` of package.json is a string, e.g. package.json of `npm`
- allow Windows to spawn binaries inside the compiled product
- add environment variable `ENCLOSE_IO_USE_ITSELF`, which avoids injecting `ENCLOSE_IO_USE_ORIGINAL_NODE` when spawning child processes
- equip nodec itself with auto-update abilities via cloud services provided by http://enclose.io
- removes Master CI Part 2, which is now part of Master CI

## v1.1.0

- designed and published the project homepage via Ant Design
  - http://enclose.io/
- upgrade Node.js runtime to v8.1.1
  - https://nodejs.org/en/blog/release/v8.1.1/
- upgrade Libsquash to v0.5.0
  - deprecate use of swprintf() to ease compiling on Windows
  - interpret system calls to access()
- disable auto-update when environment variable `CI` was set
- support spawning child processes via `sh -c "..."`, using the enclosed variable itself as the Node.js interpreter

Translations in Chinese:
- 用 Ant Design 为 Node.js Compiler 设计、发布了项目主页
  - http://enclose.io/
- 升级 Node.js 运行时到 v8.1.1
  - https://nodejs.org/en/blog/release/v8.1.1/
- 升级 Libsquash 到 v0.5.0
  - 为方便在 Windows 上编译而去掉了 swprintf()
  - 劫持 access() 系统调用
- 当环境变量 `CI` 被设定时，禁用自动更新
- 支持以 `sh -c "..."` 的形式派生子程序，并把打包后的进程自身作为 Node.js 解释器使用

## v1.0.0

- upgrade Node.js runtime to v8.0.0
- upgrade libsquash to v0.4.0
- add runtime support for native modules
- add CI tests for native modules
- make sure that we are able to compile web apps
- allow executing files within the enclosed package
- allow reusing the package itself as an Node.js interpreter
- on Windows, build the corresponding arch. with the node under use
- remove the `ENCLOSE_IO_ALWAYS_USE_ORIGINAL_NODE` hack
- add auto-update feature via --auto-update-url and --auto-update-base

Translations in Chinese:
- 升级 Node.js 运行时到 8.0.0
- 升级 libsquash 到 v0.4.0
- 支持使用 node-sass 等 C++ 扩展模块
- 支持编译 Egg 等框架开发的 Web 应用
- 支持执行包内的可执行文件，如 PhantomJS
- 支持包分发后原地自动更新

## v0.9.6

- relax node.js version requirement: https://github.com/pmq20/node-compiler/issues/27
- upgrade Node.js runtime to v7.10.0
- add hints about installing SquashFS Tools

## v0.9.5

- upgrade Node.js runtime to v7.7.3
- upgrade libsquash to https://github.com/pmq20/libsquash/commit/ec44808a0170edb8c8ff2cd5f337d5f8f317098a
- let Master CI use the correct Node.js version
- add Black-box Test
- add RAM Test
- make sure that the user have installed the correct version of node in her environment; it should match the enclosed Node.js runtime version of the compiler

## v0.9.4

- upgrade Node.js runtime to v7.7.1
- add options --clean-tmpdir and --keep-tmpdir
- fix #18: https://github.com/pmq20/node-compiler/issues/18
- upgrade libsquash to https://github.com/pmq20/libsquash/commit/4cc90f9dfe83f988b982d805cec84da533bc6d33
- cf. https://github.com/pmq20/libsquash/compare/ea07909623b1e1f43e67acc3c7880dea6ba5854a...4cc90f9dfe83f988b982d805cec84da533bc6d33

Translations in Chinese:
除了上述变化，最重要的修改发生在 libsquash 中，相比于上个版本添加了对符号链接更好的支持、添加了对并发的加锁控制、添加了更多 API 如 pread 和 readv、添加了对 DOS errno 和 errno 的更完备的处理、添加了 IODeviceIoControl 和 CreateIoCompletionPort 等 Win32 API 等。

## v0.9.3

- upgrade Node.js runtime to v7.5.0
- distribute via binaries, i.e. nodec.exe, nodec-darwin-x64, and nodec-linux-x64
- upgrade libsquash to https://github.com/pmq20/libsquash/commit/ea07909623b1e1f43e67acc3c7880dea6ba5854a
- add --npm-package

Translations in Chinese:
接入 SquashFS 和 libsquash 后已经可以正常编译 Windows 下的某些包，编译后轻测可用。
其他平台和其他包还没来得及测试，可能还存在一些问题。

## v0.9.2

- upgrade Node.js runtime to v7.4.0
- use SquashFS and unobtrusive hacking techniques: https://github.com/pmq20/node-compiler/pull/14
- make libsquash + Node.js works under Windows: https://github.com/pmq20/node-compiler/pull/16

Translations in Chinese:
接入 SquashFS 和 libsquash，通过 libsquash，打包出来的产品自带压缩，才三四十兆，
而且是根据访问需求在内存中进行部分解压，用户完全无感知，试验发现把 nodec 自身编译好之后，
可执行文件大小仅比 node 大 9 MB，这在分发产品时是非常优雅的；
且支持多种数据结构，如符号链；且由于良好的数据结构设计，能解决之前目录遍历慢的问题；
而且可以最大限度地减小对 node.js 代码的侵入性，因为 libsquash 的 API 跟系统调用风格一致，
直接通过宏就可以统一改掉 libuv 中所有对文件系统的访问。

## v0.9.1

- upgrade Node.js runtime to v7.3.0
- add support to pack an entire Node.js project (e.g. cnpmjs.org)
- change the usage of the `nodec` command
- stop polluting the vendor directory of nodec itself

Translations in Chinese:
本次发布对 Node.js 编译器的命令行用法进行了大改，使它可以同时满足三种场景的通用需求，
亦即，编译 CLI 工程、编译 Web 工程、编译 npm 包。同时编译时只使用临时目录，
而不污染编译器自身的资源目录，这使得下一步实现编译器自举成为可能。
最后将运行时引擎版本升级到了 7.3.0。

## v0.9.0

- upgrade the runtime to node-v7.2.1
- let `enclose_io_memfs_exist_dir` and `enclose_io_memfs_readdir` fail fast
- make `ENCLOSE_IO_USE_ORIGINAL_NODE` non-contagious

## v0.8.0

- upgrade the runtime to node-v7.2.0: https://github.com/pmq20/node-compiler/pull/12
- remove node_version: https://github.com/pmq20/node-compiler/pull/13

## v0.7.0

- change command name to `nodec`
- change gem name to `node-compiler`

## v0.6.0

- hack spawn and spawnSync: https://github.com/pmq20/node-compiler/pull/10
- hack fs.stat, fs.watch, fs.watchFile: https://github.com/pmq20/node-compiler/pull/11

## v0.5.0

- upgrade the runtime to node v7.1.0: https://github.com/pmq20/node-compiler/pull/9
- use node_javascript.cc and js2c.py from v6: https://github.com/pmq20/node-compiler/commit/48428dd8e3ce12f6f001c5190c00965a5c696290
- resume using `__enclose_io_fork__`: https://github.com/pmq20/node-compiler/commit/80e963f56e6688621d8e129761d4dd3dd52c5707
- unshift slash into MEMFS: https://github.com/pmq20/node-compiler/pull/8
- hack fs.readdir and fs.readdirSync: https://github.com/pmq20/node-compiler/pull/7
- hack fs.readFile: https://github.com/pmq20/node-compiler/pull/4
- prefer ENCLOSE_IO_USE_ORIGINAL_NODE to `__enclose_io_fork__`: https://github.com/pmq20/node-compiler/pull/3
