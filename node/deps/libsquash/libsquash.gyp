# Copyright (c) 2017 Minqi Pan <pmq2001@gmail.com>
#                    Shengyuan Liu <sounder.liu@gmail.com>
# 
# This file is part of libsquash, distributed under the MIT License
# For full terms see the included LICENSE file

{
  'targets': [
    {
      'target_name': 'libsquash',
      'type': 'static_library',
      'sources': [
        'cache.c',
        'decompress.c',
        'dir.c',
        'dirent.c',
        'fd.c',
        'file.c',
        'fs.c',
        'hash.c',
        'nonstd-makedev.c',
        'nonstd-stat.c',
        'private.c',
        'readlink.c',
        'scandir.c',
        'squash.c',
        'stack.c',
        'stat.c',
        'table.c',
        'traverse.c',
        'util.c',
      ],
      'include_dirs': [
        'include',
      ],
    },
  ],
}
