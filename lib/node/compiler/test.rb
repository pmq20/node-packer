# Copyright (c) 2016 Node.js Compiler contributors
# 
# This file is part of Node.js Compiler, distributed under the MIT License
# For full terms see the included LICENSE file

require 'shellwords'
require 'tmpdir'
require 'fileutils'
require 'json'
require 'open3'

module Node
  class Compiler
    class Test
      def initialize(tempdir, options = {})
        Utils.prepare_tempdir(tempdir)
        @vendor_dir = File.join(tempdir, NODE_VERSION)
        @options = options
      end
      
      def run!
        Utils.chdir(@vendor_dir) do
          Utils.inject_memfs(File.expand_path('./test/fixtures', @vendor_dir), @vendor_dir)
          STDERR.puts "-> FileUtils.rm_f(#{Gem.win_platform? ? 'Release\\node.exe' : 'out/Release/node'})"
          FileUtils.rm_f(Gem.win_platform? ? 'Release\\node.exe' : 'out/Release/node')
          File.open(File.expand_path('./lib/enclose_io_entrance.js', @vendor_dir), "w") { |f| f.puts 'module.exports = false;' }
          test_env = {
                       'FLAKY_TESTS_MODE' => 'dontcare',
                       'FLAKY_TESTS' => 'dontcare',
                       'ENCLOSE_IO_USE_ORIGINAL_NODE' => '1',
                       'ENCLOSE_IO_ALWAYS_USE_ORIGINAL_NODE' => '1',
                     }
          if Gem.win_platform?
            Utils.run(test_env, 'call vcbuild.bat nosign test-ci ignore-flaky')
          else
            Utils.run("./configure")
            Utils.run("make")
            Utils.run(test_env, "make test-ci")
          end
        end
      end
    end
  end
end
