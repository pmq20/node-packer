# Node.js Compiler Changelog

## v0.9.1

[zh] 本次发布对 Node.js 编译器的命令行用法进行了大改，使它可以同时满足三种场景的通用需求，亦即，编译 CLI 工程、编译 Web 工程、编译 npm 包。同时编译时只使用临时目录，而不污染编译器自身的资源目录，这使得下一步实现编译器自举成为可能。最后将运行时引擎版本升级到了 7.3.0。

[en] See below.

* add support to pack an entire Node.js project (e.g. nodeclub)
* change the usage of the `nodec` command
* stop polluting the vendor directory of nodec itself
* upgrade to node-v7.3.0

## v0.9.0

* let `enclose_io_memfs_exist_dir` and `enclose_io_memfs_readdir` fail fast
* upgrade the runtime to node-v7.2.1
* make `ENCLOSE_IO_USE_ORIGINAL_NODE` non-contagious

## v0.8.0

* upgrade the runtime to node-v7.2.0: https://github.com/pmq20/node-compiler/pull/12
* remove node_version: https://github.com/pmq20/node-compiler/pull/13

## v0.7.0

* change command name to `nodec`
* change gem name to `node-compiler`

## v0.6.0

* hack spawn and spawnSync: https://github.com/pmq20/compiler/pull/10
* hack fs.stat, fs.watch, fs.watchFile: https://github.com/pmq20/compiler/pull/11

## v0.5.0

* use node_javascript.cc and js2c.py from v6: https://github.com/pmq20/compiler/commit/48428dd8e3ce12f6f001c5190c00965a5c696290
* resume using `__enclose_io_fork__`: https://github.com/pmq20/compiler/commit/80e963f56e6688621d8e129761d4dd3dd52c5707
* upgrade the runtime to node v7.0.0 to node v7.1.0: https://github.com/pmq20/compiler/pull/9
* unshift slash into MEMFS: https://github.com/pmq20/compiler/pull/8
* hack fs.readdir and fs.readdirSync: https://github.com/pmq20/compiler/pull/7
* hack fs.readFile: https://github.com/pmq20/compiler/pull/4
* prefer ENCLOSE_IO_USE_ORIGINAL_NODE to `__enclose_io_fork__`: https://github.com/pmq20/compiler/pull/3
