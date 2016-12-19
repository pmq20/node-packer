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

        def inject_memfs(source, target)
          copydir = File.expand_path("./lib#{MEMFS}", target)
          if File.exist?(copydir)
            STDERR.puts "-> FileUtils.remove_entry_secure(#{copydir})"
            FileUtils.remove_entry_secure(copydir)
          end
          STDERR.puts "-> FileUtils.cp_r(#{source}, #{copydir})"
          FileUtils.cp_r(source, copydir)
          manifest = File.expand_path('./enclose_io_manifest.txt', target)
          File.open(manifest, "w") do |f|
            Dir["#{copydir}/**/*"].each do |fullpath|
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
          return copydir
        end
      end
    end
  end
end
