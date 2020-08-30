# frozen_string_literal: true

# Copyright (c) 2017 - 2020 Minqi Pan et al.
#
# This file is part of Node.js Packer, distributed under the MIT License
# For full terms see the included LICENSE file

class Compiler
  # The version is prefixed by the supported versions of the Current & LTS Node.js
  VERSION = '140800.121803.dev'
  PRJ_ROOT = File.expand_path('../..', __dir__)
  MEMFS = '/__enclose_io_memfs__'
end
