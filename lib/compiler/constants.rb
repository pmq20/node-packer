# Copyright (c) 2017 Minqi Pan <pmq2001@gmail.com>
# 
# This file is part of Node.js Compiler, distributed under the MIT License
# For full terms see the included LICENSE file

class Compiler
  VERSION = '0.9.2'
  PRJ_ROOT = File.expand_path('../../..', __FILE__)
  VENDOR_DIR = File.expand_path('vendor', PRJ_ROOT)
  MEMFS = '/__enclose_io_memfs__'
end
