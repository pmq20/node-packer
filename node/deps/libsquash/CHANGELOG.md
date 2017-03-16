# Libsquash Changelog

## v0.3.0

- Implemented the following dirent APIs.
  - squash_opendir(fs, filename)
  - squash_closedir(dirp)
  - squash_readdir(dirp)
  - squash_telldir(dirp)
  - squash_seekdir(dirp, loc)
  - squash_rewinddir(dirp)
  - squash_dirfd(dirp)
  - squash_lstat(fs, path, buf)
  - squash_readlink(fs, path, buf, bufsize)
  - squash_scandir(fs, dirname, namelist, select, compar)
- Added better support for symbolic links
- Added locks to handle concurrent situations (locking four caches and one global vfd table)
- Intercept more API's like pread and readv on Unix
- Intercept more API's like IODeviceIoControl and CreateIoCompletionPort on Win32
- Added better support for DOS errno and errno on Windows
- Support symbolic paths like /a/b/../../c/d and /a/b/c/d/..
- Enlarge buffer to support longer path names
- Fix a buffer overflow found in wcstombs functions
- Dynamically change some absolute symbolic paths into relative ones via `root_alias`

中文注解：
- 增加了如下新 API 的实现
  - squash_opendir(fs, filename)
  - squash_closedir(dirp)
  - squash_readdir(dirp)
  - squash_telldir(dirp)
  - squash_seekdir(dirp, loc)
  - squash_rewinddir(dirp)
  - squash_dirfd(dirp)
  - squash_lstat(fs, path, buf)
  - squash_readlink(fs, path, buf, bufsize)
  - squash_scandir(fs, dirname, namelist, select, compar)
- 添加了对符号链接更好的支持
- 添加了对并发的加锁控制（对四个 SquashFS 缓存加锁、一个公共文件描述符表加锁）
- 添加了更多 API 如 pread 和 readv
- 添加了 IODeviceIoControl 和 CreateIoCompletionPort 等 Win32 API 等
- 添加了对 DOS errno 和 errno 的更完备的处理
- 跟踪链接的这个地方我增加了处理 /a/b/../../c/d 和 /a/b/c/d/.. 这种带双点的情况
- 扩大了缓冲区大小解决了路径名太长的问题
- 修复缓冲区溢出（wcstombs 等库函数的第三个参数限制了最大转换长度，然而“If max bytes are successfully translated, the resulting string stored in dest is not null-terminated.”，造成了后续对 dest 访问的缓冲区溢出）
- 通过 `root_alias` 统一对未出项目根部的路径的绝对路径链接在运行时动态改为相对路径（这个地方有点 ugly hack 了，更好的做法是在 mksquashfs 的时候就做这个操作，而不是运行时，这里以后改进）

## v0.2.0

Implemented the virutal file descriptors and the following new API's:

- squash_stat(fs, path, buf)
- squash_fstat(vfd, buf)
- squash_open(fs, path)
- squash_close(vfd)
- squash_read(vfd, buf, nbyte)
- squash_lseek(vfd, offset, whence)

中文注解：
实现了虚拟文件描述符表(vfd, virtual file descriptor)，
虚拟文件描述符与正常磁盘文件的文件描述符 (fd) 可以和平共处，
这使得上层运行时可以无缝接入对内存文件系统的访问，而不需要原有代码的调用风格。
新开发了以上 API 实现，实现与 open、lseek、read 等系统调用的风格相同的调用接口。
为基础 API、stat 系列 API、虚拟文件描述符表系列 API 全部编写了测试。

## v0.1.0

Added Tests for basic APIs from squashfuse; tagged the initial release.
