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

        def prepare_tempdir(tempdir)
          STDERR.puts "-> FileUtils.mkdir_p(#{tempdir})"
          FileUtils.mkdir_p(tempdir)
          Dir[::Node::Compiler::VENDOR_DIR + '/*'].each do |dirpath|
            target = File.join(tempdir, File.basename(dirpath))
            unless Dir.exist?(target)
              STDERR.puts "-> FileUtils.cp_r(#{dirpath}, #{target})"
              FileUtils.cp_r(dirpath, target)
            end
          end
        end
      end
    end
  end
end
