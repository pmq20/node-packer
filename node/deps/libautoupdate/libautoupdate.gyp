# Copyright (c) 2017 Minqi Pan <pmq2001@gmail.com>
# 
# This file is part of libautoupdate, distributed under the MIT License
# For full terms see the included LICENSE file

{
  'targets': [
    {
      'target_name': 'libautoupdate',
      'type': 'static_library',
      'sources': [
        'include/autoupdate.h',
        'src/autoupdate_internal.h',
        'src/windows.c',
        'src/unix.c',
      ],
      'include_dirs': [
        'include',
        '../zlib',
      ],
    },
  ],
}
