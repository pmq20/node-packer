# Libsquash Changelog

## v0.3.0

Implemented the following dirent APIs.

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

## v0.2.0

Implemented the virutal file descriptors and the following new API's:

- squash_stat(fs, path, buf)
- squash_fstat(vfd, buf)
- squash_open(fs, path)
- squash_close(vfd)
- squash_read(vfd, buf, nbyte)
- squash_lseek(vfd, offset, whence)

中文注解：实现了虚拟文件描述符表(vfd, virtual file descriptor)，
虚拟文件描述符与正常磁盘文件的文件描述符 (fd) 可以和平共处，
这使得上层运行时可以无缝接入对内存文件系统的访问，而不需要原有代码的调用风格。
新开发了以上 API 实现，实现与 open、lseek、read 等系统调用的风格相同的调用接口。
为基础 API、stat 系列 API、虚拟文件描述符表系列 API 全部编写了测试。

## v0.1.0

Added Tests for basic APIs from squashfuse; tagged the initial release.
