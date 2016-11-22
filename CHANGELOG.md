# Node.js Compiler Changelog

## v0.7.0

* change command name to `nodec`
* change gem name to `node-compiler`

## v0.6.0

* hack spawn and spawnSync: https://github.com/enclose-io/compiler/pull/10
* hack fs.stat, fs.watch, fs.watchFile: https://github.com/enclose-io/compiler/pull/11

## v0.5.0

* Use node_javascript.cc and js2c.py from v6: https://github.com/enclose-io/compiler/commit/48428dd8e3ce12f6f001c5190c00965a5c696290
* resume using `__enclose_io_fork__`: https://github.com/enclose-io/compiler/commit/80e963f56e6688621d8e129761d4dd3dd52c5707
* Upgrade node v7.0.0 to node v7.1.0: https://github.com/enclose-io/compiler/pull/9
* unshift slash into MEMFS: https://github.com/enclose-io/compiler/pull/8
* Hack fs.readdir and fs.readdirSync: https://github.com/enclose-io/compiler/pull/7
* Hack fs.readFile: https://github.com/enclose-io/compiler/pull/4
* Prefer ENCLOSE_IO_USE_ORIGINAL_NODE to `__enclose_io_fork__`: https://github.com/enclose-io/compiler/pull/3
