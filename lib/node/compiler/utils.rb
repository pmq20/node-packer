# Copyright (c) 2016-2017 Minqi Pan <pmq2001@gmail.com>
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
    module Utils
      class << self
        def run(*args)
          STDERR.puts "-> Running #{args}"
          pid = spawn(*args)
          pid, status = Process.wait2(pid)
          raise Error, "Failed running #{args}" unless status.success?
        end

        def chdir(path)
          STDERR.puts "-> cd #{path}"
          Dir.chdir(path) { yield }
          STDERR.puts "-> cd #{Dir.pwd}"
        end

        def prepare_tmpdir(tmpdir)
          STDERR.puts "-> FileUtils.mkdir_p(#{tmpdir})"
          FileUtils.mkdir_p(tmpdir)
          Dir[::Node::Compiler::VENDOR_DIR + '/*'].each do |dirpath|
            target = File.join(tmpdir, File.basename(dirpath))
            unless Dir.exist?(target)
              STDERR.puts "-> FileUtils.cp_r(#{dirpath}, #{target})"
              FileUtils.cp_r(dirpath, target)
            end
          end
        end
        
        def remove_dynamic_libs(path)
          ['dll', 'dylib', 'so'].each do |extname|
            Dir["#{path}/**/*.#{extname}"].each do |x|
              STDERR.puts "-> FileUtils.rm_f #{x}"
              FileUtils.rm_f(x)
            end
          end
        end

        def copy_static_libs(path, target)
          ['lib', 'a'].each do |extname|
            Dir["#{path}/*.#{extname}"].each do |x|
              STDERR.puts "-> FileUtils.cp #{x}, #{target}"
              FileUtils.cp(x, target)
            end
          end
        end
      end
    end
  end
end
