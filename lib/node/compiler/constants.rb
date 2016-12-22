# Copyright (c) 2016-2017 Node.js Compiler contributors
# 
# This file is part of Node.js Compiler, distributed under the MIT License
# For full terms see the included LICENSE file

module Node
  class Compiler
    VERSION = '0.9.1'
    VENDOR_DIR = File.expand_path('../../../../vendor', __FILE__)
    MEMFS = '/__enclose_io_memfs__'
  end
end
