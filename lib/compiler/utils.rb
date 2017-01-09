# Copyright (c) 2016-2017 Minqi Pan <pmq2001@gmail.com>
# 
# This file is part of Node.js Compiler, distributed under the MIT License
# For full terms see the included LICENSE file

require 'shellwords'
require 'tmpdir'
require 'fileutils'
require 'json'
require 'open3'

class Compiler
  module Utils
    def self.escape(arg)
      if Gem.win_platform?
        if arg.include?('"')
          raise NotImplementedError
        end
        %Q{"#{arg}"}
      else
        Shellwords.escape(arg)
      end
    end
    
    def self.run(*args)
      STDERR.puts "-> Running #{args}"
      pid = spawn(*args)
      pid, status = Process.wait2(pid)
      raise Error, "Failed running #{args}" unless status.success?
    end

    def self.chdir(path)
      STDERR.puts "-> cd #{path}"
      Dir.chdir(path) { yield }
      STDERR.puts "-> cd #{Dir.pwd}"
    end

    def self.prepare_tmpdir(tmpdir)
      STDERR.puts "-> FileUtils.mkdir_p(#{tmpdir})"
      FileUtils.mkdir_p(tmpdir)
      Dir[VENDOR_DIR + '/*'].each do |dirpath|
        target = File.join(tmpdir, File.basename(dirpath))
        unless Dir.exist?(target)
          STDERR.puts "-> FileUtils.cp_r(#{dirpath}, #{target})"
          FileUtils.cp_r(dirpath, target)
        end
      end
    end
    
    def self.remove_dynamic_libs(path)
      ['dll', 'dylib', 'so'].each do |extname|
        Dir["#{path}/**/*.#{extname}"].each do |x|
          STDERR.puts "-> FileUtils.rm_f #{x}"
          FileUtils.rm_f(x)
        end
      end
    end

    def self.copy_static_libs(path, target)
      ['lib', 'a'].each do |extname|
        Dir["#{path}/*.#{extname}"].each do |x|
          STDERR.puts "-> FileUtils.cp #{x}, #{target}"
          FileUtils.cp(x, target)
        end
      end
    end
  end
end
