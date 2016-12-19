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
  
        def mempath(path)
          path = File.expand_path(path)
          raise 'Logic error in mempath' unless @project_root == path[0...(@project_root.size)]
          "#{MEMFS}#{path[(@project_root.size)..-1]}"
        end
  
        def copypath(path)
          path = File.expand_path(path)
          raise 'Logic error 1 in copypath' unless @project_root == path[0...(@project_root.size)]
          ret = File.join(@copydir, path[(@project_root.size)..-1])
          raise 'Logic error 2 in copypath' unless File.exist?(ret)
          ret
        end

        def debug
          STDERR.puts "@entrance: #{@entrance}"
          STDERR.puts "@project_root: #{@project_root}"
          STDERR.puts "@options: #{@options}"
        end
      end
    end
  end
end
