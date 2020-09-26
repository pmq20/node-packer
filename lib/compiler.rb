# frozen_string_literal: true

# Copyright (c) 2017 - 2020 Minqi Pan et al.
#
# This file is part of Node.js Packer, distributed under the MIT License
# For full terms see the included LICENSE file

require_relative './compiler/constants'
require_relative './compiler/error'
require_relative './compiler/utils'

require 'shellwords'
require 'tmpdir'
require 'fileutils'
require 'open3'
require 'uri'
require 'erb'
require 'securerandom'

# A Compiler object corresponds to a Node.js Packer runtime instance
class Compiler
  def initialize(entrance = nil, options = {})
    @options = options
    @entrance = entrance
    @utils = Utils.new(options)

    init_options
    init_entrance_and_root if @entrance
    init_tmpdir

    warn "Node.js Compiler (nodec) v#{::Compiler::VERSION}" unless @options[:quiet]
    if @entrance
      warn "- entrance: #{@entrance}" unless @options[:quiet]
    else
      warn '- entrance: not provided, a single Node.js interpreter executable will be produced.' unless @options[:quiet]
      warn '- HINT: call nodec with --help to see more options and use case examples' unless @options[:quiet]
    end
    warn "- options: #{@options}" unless @options[:quiet]
    $stderr.puts unless @options[:quiet]
  end

  def init_entrance_and_root
    # Important to expand_path; otherwiser the while would be erroneous
    @entrance = File.expand_path(@entrance)
    raise Error, "Cannot find entrance #{@entrance}." unless File.exist?(@entrance)

    if @options[:root]
      @root = File.expand_path(@options[:root])
    else
      @root = File.dirname(@entrance)
      # this while has to correspond with the expand_path above
      until File.exist?(File.expand_path('./package.json', @root))
        break if @root == File.expand_path('..', @root)

        @root = File.expand_path('..', @root)
      end
    end

    # if we have to perform npm install we need a package.json in order to succeed
    raise Error, "Cannot find a package.json inside #{@root}" if (File.exist?(File.expand_path('./package.json', @root)) == false) && @options[:skip_npm_install].nil?
  end

  def init_options
    @current_or_lts = 'current'
    raise Error, 'Please specify either --current or --lts' if @options[:current] && @options[:lts]

    @current_or_lts = 'lts' if @options[:lts]

    @options[:npm] ||= 'npm'
    @node_dir = "node-#{VERSION}-#{@current_or_lts}"
    @options[:make_args] ||= '-j4'
    @options[:vcbuild_args] ||= `node -pe process.arch`.to_s.strip
    @options[:output] ||= if Gem.win_platform?
                            'a.exe'
                          else
                            'a.out'
                          end
    @options[:output] = File.expand_path(@options[:output])

    @options[:tmpdir] ||= File.expand_path('nodec', Dir.tmpdir)
    @options[:tmpdir] = File.expand_path(@options[:tmpdir])
  end

  def init_tmpdir
    @options[:tmpdir] = File.expand_path(@options[:tmpdir])
    @root = File.expand_path(@root) if @root
    raise Error, "tmpdir #{@options[:tmpdir]} cannot reside inside #{@root}." if @root && (@options[:tmpdir].include? @root)

    @work_dir = File.join(@options[:tmpdir], '__work_dir__')
    @work_dir_inner = File.join(@work_dir, '__enclose_io_memfs__')
  end

  def stuff_tmpdir
    @utils.rm_rf(@options[:tmpdir]) if @options[:clean_tmpdir]
    @utils.mkdir_p(@options[:tmpdir])
    @tmpdir_node = File.join(@options[:tmpdir], @node_dir)
    @utils.cp_r(File.join(PRJ_ROOT, @current_or_lts), @tmpdir_node, preserve: true) unless Dir.exist?(@tmpdir_node)
  end

  def run!
    stuff_tmpdir
    npm_install if @entrance && !@options[:keep_tmpdir]
    make_enclose_io_memfs if @entrance && !@options[:keep_tmpdir]
    make_enclose_io_vars
    if Gem.win_platform?
      compile_win
    elsif RbConfig::CONFIG['host_os'] =~ /darwin|mac os/i
      compile_mac
    else
      compile_linux
    end
  end

  def npm_install
    @utils.rm_rf(@work_dir)
    @utils.mkdir_p(@work_dir)
    @utils.cp_r(@root, @work_dir_inner)

    unless @options[:skip_npm_install]
      @utils.chdir(@work_dir_inner) do
        @utils.run(ENV, "#{@utils.escape @options[:npm]} -v")
        @utils.run(ENV, "#{@utils.escape @options[:npm]} install --production")
      end
    end

    @utils.chdir(@work_dir_inner) do
      if Dir.exist?('.git')
        warn `git status` unless @options[:quiet]
        @utils.rm_rf('.git')
      end
      if File.exist?('a.exe')
        warn `dir a.exe`
        @utils.rm_rf('a.exe')
      end
      if File.exist?('a.out')
        warn `ls -l a.out`
        @utils.rm_rf('a.out')
      end
      if File.exist?('node_modules/node/bin/node.exe')
        warn `dir node_modules\\node\\bin\\node.exe`
        @utils.rm_rf('node_modules\node\bin\node.exe')
      end
      if File.exist?('node_modules/.bin/node.exe')
        warn `dir node_modules\\.bin\\node.exe`
        @utils.rm_rf('node_modules\.bin\node.exe')
      end
      if File.exist?('node_modules/.bin/node')
        warn `ls -lh node_modules/.bin/node`
        @utils.rm_rf('node_modules/.bin/node')
      end
      if File.exist?('node_modules/node/bin/node')
        warn `ls -lh node_modules/node/bin/node`
        @utils.rm_rf('node_modules/node/bin/node')
      end
    end
  end

  def make_enclose_io_memfs
    @utils.chdir(@tmpdir_node) do
      @utils.rm_f('deps/libsquash/sample/enclose_io_memfs.squashfs')
      @utils.rm_f('deps/libsquash/sample/enclose_io_memfs.c')
      begin
        @utils.run(ENV, 'mksquashfs -version')
      rescue StandardError => e
        msg =  "=== HINT ===\n"
        msg += "Failed exectuing mksquashfs. Have you installed SquashFS Tools?\n"
        msg += "- On Windows, you could download it from https://github.com/pmq20/squashfuse/files/691217/sqfs43-win32.zip\n"
        msg += "- On macOS, you could install by using brew: brew install squashfs\n"
        msg += "- On Linux, you could install via apt or yum, or build from source after downloading source from http://squashfs.sourceforge.net/\n\n"
        warn msg unless @options[:quiet]
        raise e
      end
      @utils.run(ENV, "mksquashfs #{@utils.escape @work_dir} deps/libsquash/sample/enclose_io_memfs.squashfs")
      bytes = IO.binread('deps/libsquash/sample/enclose_io_memfs.squashfs').bytes
      # remember to change libsquash's sample/enclose_io_memfs.c as well
      File.open('deps/libsquash/sample/enclose_io_memfs.c', 'w') do |f|
        f.puts '#include <stdint.h>'
        f.puts '#include <stddef.h>'
        f.puts '#include "squash.h"'
        f.puts 'sqfs *enclose_io_fs;'
        f.puts "const uint8_t enclose_io_memfs[#{bytes.size}] = { #{bytes[0]}"
        i = 1
        while i < bytes.size
          f.print ','
          f.puts bytes[i..(i + 100)].join(',')
          i += 101
        end
        f.puts '};'
        f.puts ''
      end
    end
  end

  def make_enclose_io_vars
    @utils.chdir(@tmpdir_node) do
      if Gem.win_platform?
        # remove `node_main.obj` before compiling to avoid a MS toolchain bug
        @utils.rm_f('Release/obj/node/node_main.obj')
        @utils.rm_f('Debug/obj/node/node_main.obj')
      end
      File.open('deps/libsquash/sample/enclose_io.h', 'w') do |f|
        # remember to change libsquash's sample/enclose_io.h as well
        f.puts '#ifndef ENCLOSE_IO_H_999BC1DA'
        f.puts '#define ENCLOSE_IO_H_999BC1DA'
        f.puts ''
        f.puts '#include "enclose_io_prelude.h"'
        f.puts '#include "enclose_io_common.h"'
        f.puts '#include "enclose_io_win32.h"'
        f.puts '#include "enclose_io_unix.h"'
        if @entrance
          if Gem.win_platform?
            f.puts "#define ENCLOSE_IO_ENTRANCE L#{mempath(@entrance).inspect}"
            # TODO: remove this dirty hack some day
            squash_root_alias = @work_dir
            squash_root_alias += '/' unless squash_root_alias[-1] == '/'
            raise 'logic error' unless squash_root_alias[1..2] == ':/'

            squash_root_alias = "/cygdrive/#{squash_root_alias[0].downcase}/#{squash_root_alias[3..]}"
            f.puts "#define ENCLOSE_IO_ROOT_ALIAS #{squash_root_alias.inspect}"
            squash_root_alias2 = squash_root_alias[11..]
            f.puts "#define ENCLOSE_IO_ROOT_ALIAS2 #{squash_root_alias2.inspect}" if squash_root_alias2 && squash_root_alias2.length > 1
          else
            f.puts "#define ENCLOSE_IO_ENTRANCE #{mempath(@entrance).inspect}"
          end
        end
        f.puts '#endif'
        f.puts ''
      end
    end
  end

  def compile_win
    @utils.chdir(@tmpdir_node) do
      # --without-intl=none fixes: icutrim.py - it tries to run a binary made for linux on mac
      # --cross-compiling is required require host executables rather than target ones
      # --without-snapshot avoids mksnapshot to run on host platform after build
      @utils.run(ENV, "call vcbuild.bat #{@options[:debug] ? 'debug' : ''} #{@options[:vcbuild_args]}")
    end
    src = File.join(@tmpdir_node, (@options[:debug] ? 'Debug\\node.exe' : 'Release\\node.exe'))
    @utils.cp(src, @options[:output])
  end

  def compile_mac
    @utils.chdir(@tmpdir_node) do
      # --without-intl=none fixes: icutrim.py - it tries to run a binary made for linux on mac
      # --cross-compiling is required require host executables rather than target ones
      # --without-snapshot avoids mksnapshot to run on host platform after build
      @utils.run(ENV, "./configure #{@options[:debug] ? '--debug --xcode' : ''} #{@options[:os] ? '--cross-compiling --without-snapshot  --with-intl=none' : ''} #{@options[:os] ? "--dest-os=#{@options[:os]}" : ''} #{@options[:arch] ? "--dest-cpu=#{@options[:arch]}" : ''}")
      @utils.run(ENV, "make #{@options[:make_args]}")
    end
    src = File.join(@tmpdir_node, "out/#{@options[:debug] ? 'Debug' : 'Release'}/node")
    @utils.cp(src, @options[:output])
  end

  def compile_linux
    @utils.chdir(@tmpdir_node) do
      # --without-intl=none fixes: icutrim.py - it tries to run a binary made for linux on mac
      # --cross-compiling is required require host executables rather than target ones
      # --without-snapshot avoids mksnapshot to run on host platform after build
      @utils.run(ENV, "./configure #{@options[:debug] ? '--debug' : ''} #{@options[:os] ? '--cross-compiling --without-snapshot  --with-intl=none' : ''} #{@options[:os] ? "--dest-os=#{@options[:os]}" : ''} #{@options[:arch] ? "--dest-cpu=#{@options[:arch]}" : ''}")
      @utils.run(ENV, "make #{@options[:make_args]}")
    end
    src = File.join(@tmpdir_node, "out/#{@options[:debug] ? 'Debug' : 'Release'}/node")
    @utils.cp(src, @options[:output])
  end

  def mempath(path)
    path = File.expand_path(path)
    raise "path #{path} should start with #{@root}" unless @root == path[0...(@root.size)]

    "#{MEMFS}#{path[(@root.size)..]}"
  end

  def copypath(path)
    path = File.expand_path(path)
    raise 'Logic error 1 in copypath' unless @root == path[0...(@root.size)]

    ret = File.join(@copy_dir, path[(@root.size)..])
    raise 'Logic error 2 in copypath' unless File.exist?(ret)

    ret
  end
end
