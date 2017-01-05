# Copyright (c) 2016-2017 Minqi Pan
# 
# This file is part of Node.js Compiler, distributed under the MIT License
# For full terms see the included LICENSE file

module Node
  class Compiler
    VERSION = '1.0.0'
    VENDOR_DIR = File.expand_path('../../../../vendor', __FILE__)
    MEMFS = '/__enclose_io_memfs__'
  end
end
