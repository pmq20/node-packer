# Copyright (c) 2017 Minqi Pan <pmq2001@gmail.com>
#                    Yuwei Ba <xiaobayuwei@gmail.com>
#                    Alessandro Agosto <agosto.alessandro@gmail.com>
# 
# This file is part of Node.js Compiler, distributed under the MIT License
# For full terms see the included LICENSE file

require 'shellwords'
require 'tmpdir'
require 'fileutils'
require 'open3'

class Compiler
  class Utils
    def initialize(options = {})
      @options = options
    end

    def escape(arg)
      if Gem.win_platform?
        if arg.include?('"')
          raise NotImplementedError
        end
        %Q{"#{arg}"}
      else
        Shellwords.escape(arg)
      end
    end
    
    def run(*args)
      STDERR.puts "-> Running #{args}" unless @options[:quiet]
      pid = spawn(*args)
      pid, status = Process.wait2(pid)
      raise Error, "Failed running #{args}" unless status.success?
    end

    def run_allow_failures(*args)
      STDERR.puts "-> Running (allowing failures) #{args}" unless @options[:quiet]
      pid = spawn(*args)
      pid, status = Process.wait2(pid)
      return status
    end

    def chdir(path)
      STDERR.puts "-> cd #{path}" unless @options[:quiet]
      Dir.chdir(path) { yield }
      STDERR.puts "-> cd #{Dir.pwd}" unless @options[:quiet]
    end
    
    def cp(x, y)
      STDERR.puts "-> cp #{x.inspect} #{y.inspect}" unless @options[:quiet]
      FileUtils.cp(x, y)
    end
    
    def cp_r(x, y, options = {})
      STDERR.puts "-> cp -r #{x.inspect} #{y.inspect}" unless @options[:quiet]
      FileUtils.cp_r(x, y, options)
    end

    def rm(x)
      STDERR.puts "-> rm #{x}" unless @options[:quiet]
      FileUtils.rm(x)
    end

    def rm_f(x)
      STDERR.puts "-> rm -f #{x}" unless @options[:quiet]
      FileUtils.rm_f(x)
    end

    def rm_rf(x)
      STDERR.puts "-> rm -rf #{x}" unless @options[:quiet]
      FileUtils.rm_rf(x)
    end

    def mkdir(x)
      STDERR.puts "-> mkdir #{x}" unless @options[:quiet]
      FileUtils.mkdir(x)
    end
    
    def mkdir_p(x)
      STDERR.puts "-> mkdir -p #{x}" unless @options[:quiet]
      FileUtils.mkdir_p(x)
    end
    
    def remove_dynamic_libs(path)
      ['dll', 'dylib', 'so'].each do |extname|
        Dir["#{path}/**/*.#{extname}"].each do |x|
          self.rm_f(x)
        end
      end
    end

    def copy_static_libs(path, target)
      ['lib', 'a'].each do |extname|
        Dir["#{path}/*.#{extname}"].each do |x|
          self.cp(x, target)
        end
      end
    end
  end
end
