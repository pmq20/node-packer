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
      def initialize(tempdir)
        Utils.prepare_tempdir(tempdir)
        @vendor_dir = File.join(tempdir, NODE_VERSION)
      end
      
      def run!
        Utils.chdir(@vendor_dir) do
          inject_memfs(File.expand_path('./test/fixtures', @vendor_dir))
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
  
      def inject_memfs(source)
        target = File.expand_path("./lib#{MEMFS}", @vendor_dir)
        FileUtils.remove_entry_secure(target) if File.exist?(target)
        FileUtils.cp_r(source, target)
        manifest = File.expand_path('./enclose_io_manifest.txt', @vendor_dir)
        File.open(manifest, "w") do |f|
          Dir["#{target}/**/*"].each do |fullpath|
            next unless File.file?(fullpath)
            if 0 == File.size(fullpath) && Gem.win_platform?
              # Fix VC++ Error C2466
              # TODO: what about empty file semantics?
              File.open(fullpath, 'w') { |f| f.puts ' ' }
            end
            entry = "lib#{fullpath[(fullpath.index MEMFS)..-1]}"
            f.puts entry
          end
        end
      end
    end
  end
end
