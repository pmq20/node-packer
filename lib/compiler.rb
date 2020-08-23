# Copyright (c) 2017 - 2020 Minqi Pan et al.
# 
# This file is part of Node.js Packer, distributed under the MIT License
# For full terms see the included LICENSE file

require "compiler/constants"
require "compiler/error"
require "compiler/utils"
require "compiler/npm_package"
require 'shellwords'
require 'tmpdir'
require 'fileutils'
require 'open3'
require 'uri'
require 'erb'
require 'securerandom'

class Compiler
  def initialize(entrance = nil, options = {})
    @options = options
    @entrance = entrance
    @utils = Utils.new(options)

    init_options
    init_entrance_and_root if @entrance
    init_tmpdir

    STDERR.puts "Node.js Compiler (nodec) v#{::Compiler::VERSION}" unless @options[:quiet]
    if @entrance
      STDERR.puts "- entrance: #{@entrance}" unless @options[:quiet]
    else
      STDERR.puts "- entrance: not provided, a single Node.js interpreter executable will be produced." unless @options[:quiet]
      STDERR.puts "- HINT: call nodec with --help to see more options and use case examples" unless @options[:quiet]
    end
    STDERR.puts "- options: #{@options}" unless @options[:quiet]
    STDERR.puts unless @options[:quiet]
  end

  def node_version
    @node_version ||= (
      version_info = File.read(File.join(PRJ_ROOT, "node/src/node_version.h"))
      versions = []
      if version_info =~ /NODE_MAJOR_VERSION\s+(\d+)/
        versions << $1.dup
      else
        raise 'Cannot peek NODE_MAJOR_VERSION'
      end
      if version_info =~ /NODE_MINOR_VERSION\s+(\d+)/
        versions << $1.dup
      else
        raise 'Cannot peek NODE_MINOR_VERSION'
      end
      if version_info =~ /NODE_PATCH_VERSION\s+(\d+)/
        versions << $1.dup
      else
        raise 'Cannot peek NODE_PATCH_VERSION'
      end
      versions.join('.')
    )
  end

  def check_base_node_version!
    expectation = "v#{node_version}"
    got = `node -v`.to_s.strip
    unless got.include?(expectation)
      msg =  "=== WARNING ===\n"
      msg += "Please make sure to have installed the correct version of node in your environment.\n"
      msg += "It should match the enclosed Node.js runtime version of the compiler.\n"
      msg += "Expecting #{expectation}; yet got #{got}.\n\n"
      STDERR.puts msg unless @options[:quiet]
    end
  end

  def init_entrance_and_root
    if @npm_package
      @root = @npm_package.work_dir
      return
    end
    # Important to expand_path; otherwiser the while would be erroneous
    @entrance = File.expand_path(@entrance)
    raise Error, "Cannot find entrance #{@entrance}." unless File.exist?(@entrance)
    if @options[:root]
      @root = File.expand_path(@options[:root])
    else
      @root = File.dirname(@entrance)
      # this while has to correspond with the expand_path above
      while !File.exist?(File.expand_path('./package.json', @root))
        break if @root == File.expand_path('..', @root)
        @root = File.expand_path('..', @root)
      end
    end

    # if we have to perform npm install we need a package.json in order to succeed
    if File.exist?(File.expand_path('./package.json', @root)) == false and @options[:skip_npm_install] == nil
      raise Error, "Cannot find a package.json inside #{@root}"
    end
  end

  def init_options
    @options[:npm] ||= 'npm'
    @node_dir = "node-#{node_version}-#{VERSION}"
    @options[:make_args] ||= '-j4'
    @options[:vcbuild_args] ||= `node -pe process.arch`.to_s.strip
    if Gem.win_platform?
      @options[:output] ||= 'a.exe'
    else
      @options[:output] ||= 'a.out'
    end
    @options[:output] = File.expand_path(@options[:output])

    @options[:tmpdir] ||= File.expand_path("nodec", Dir.tmpdir)
    @options[:tmpdir] = File.expand_path(@options[:tmpdir])
    
    if @options[:npm_package]
      @options[:npm_package_version] ||= 'latest'
      @npm_package = NpmPackage.new(@options, @utils)
    end
    
    if @options[:auto_update_url] || @options[:auto_update_base]
      unless @options[:auto_update_url].length > 0 && @options[:auto_update_base].length > 0
        raise Error, "Please provide both --auto-update-url and --auto-update-base"
      end
    end
  end

  def init_tmpdir
    @options[:tmpdir] = File.expand_path(@options[:tmpdir])
    @root = File.expand_path(@root) if @root
    if @root && (@options[:tmpdir].include? @root)
      raise Error, "tmpdir #{@options[:tmpdir]} cannot reside inside #{@root}."
    end
    @work_dir = File.join(@options[:tmpdir], '__work_dir__')
    @work_dir_inner = File.join(@work_dir, '__enclose_io_memfs__')
  end

  def stuff_tmpdir
    @utils.rm_rf(@options[:tmpdir]) if @options[:clean_tmpdir]
    @utils.mkdir_p(@options[:tmpdir])
    @tmpdir_node = File.join(@options[:tmpdir], @node_dir)
    unless Dir.exist?(@tmpdir_node)
      @utils.cp_r(File.join(PRJ_ROOT, 'node'), @tmpdir_node, preserve: true)
    end
    @npm_package.stuff_tmpdir if @npm_package
  end

  def set_package_json
    @package_json = @npm_package&.package_json
    if @package_json
      # dirty hack for MSI generation
      @package_json['name'] = File.basename(@original_entrance).gsub('-', '_')
    else
      path = File.join @work_dir_inner, 'package.json'
      if File.exist?(path)
        @package_json = JSON.parse File.read path
      else
        @package_json = {}
      end
      # dirty hack for MSI generation
      @package_json['name'] = File.basename(@package_json['name']).gsub('-', '_') if @package_json['name']
    end
    if @package_json['version']
      suffix = '.'
      @package_json['version'] = (@package_json['version'].gsub('-', '.').split('.').map { |x|
        if x =~ /\d+/
          x
        else
          suffix += x.chars.each.map { |y|
            "#{y.ord}"
          }.join('.')
          nil
        end
      }.compact.join('.') + suffix).split('.')[0..3].join('.')
    end
  end

  def run!
    check_base_node_version!
    stuff_tmpdir
    npm_install if @entrance && !@options[:keep_tmpdir]
    npm_package_set_entrance if @npm_package
    set_package_json
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

  def npm_package_set_entrance
    @utils.chdir(@work_dir_inner) do
      @original_entrance = @entrance
      @entrance = @npm_package.get_entrance(@entrance)
      STDERR.puts "-> Setting entrance to #{@entrance}" unless @options[:quiet]
    end
  end

  def npm_install
    @utils.rm_rf(@work_dir)
    @utils.mkdir_p(@work_dir)
    @utils.cp_r(@root, @work_dir_inner)

    unless @options[:skip_npm_install]
      @utils.chdir(@work_dir_inner) do
        @utils.run("#{@utils.escape @options[:npm]} -v")
        @utils.run("#{@utils.escape @options[:npm]} install --production")
      end
    end

    @utils.chdir(@work_dir_inner) do
      if Dir.exist?('.git')
        STDERR.puts `git status` unless @options[:quiet]
        @utils.rm_rf('.git')
      end
      if File.exist?('a.exe')
        STDERR.puts `dir a.exe`
        @utils.rm_rf('a.exe')
      end
      if File.exist?('a.out')
        STDERR.puts `ls -l a.out`
        @utils.rm_rf('a.out')
      end
      if File.exist?('node_modules/node/bin/node.exe')
        STDERR.puts `dir node_modules\\node\\bin\\node.exe`
        @utils.rm_rf('node_modules\node\bin\node.exe')
      end
      if File.exist?('node_modules/.bin/node.exe')
        STDERR.puts `dir node_modules\\.bin\\node.exe`
        @utils.rm_rf('node_modules\.bin\node.exe')
      end
      if File.exist?('node_modules/.bin/node')
        STDERR.puts `ls -lh node_modules/.bin/node`
        @utils.rm_rf('node_modules/.bin/node')
      end
      if File.exist?('node_modules/node/bin/node')
        STDERR.puts `ls -lh node_modules/node/bin/node`
        @utils.rm_rf('node_modules/node/bin/node')
      end
    end
  end

  def make_enclose_io_memfs
    @utils.chdir(@tmpdir_node) do
      @utils.rm_f('deps/libsquash/sample/enclose_io_memfs.squashfs')
      @utils.rm_f('deps/libsquash/sample/enclose_io_memfs.c')
      begin
        @utils.run("mksquashfs -version")
      rescue => e
        msg =  "=== HINT ===\n"
        msg += "Failed exectuing mksquashfs. Have you installed SquashFS Tools?\n"
        msg += "- On Windows, you could download it from https://github.com/pmq20/squashfuse/files/691217/sqfs43-win32.zip\n"
        msg += "- On macOS, you could install by using brew: brew install squashfs\n"
        msg += "- On Linux, you could install via apt or yum, or build from source after downloading source from http://squashfs.sourceforge.net/\n\n"
        STDERR.puts msg unless @options[:quiet]
        raise e
      end
      @utils.run("mksquashfs #{@utils.escape @work_dir} deps/libsquash/sample/enclose_io_memfs.squashfs")
      bytes = IO.binread('deps/libsquash/sample/enclose_io_memfs.squashfs').bytes
      # remember to change libsquash's sample/enclose_io_memfs.c as well
      File.open("deps/libsquash/sample/enclose_io_memfs.c", "w") do |f|
        f.puts '#include <stdint.h>'
        f.puts '#include <stddef.h>'
        f.puts '#include "squash.h"'
        f.puts 'sqfs *enclose_io_fs;'
        f.puts "const uint8_t enclose_io_memfs[#{bytes.size}] = { #{bytes[0]}"
        i = 1
        while i < bytes.size
          f.print ','
          f.puts bytes[(i)..(i + 100)].join(',')
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
      File.open("deps/libsquash/sample/enclose_io.h", "w") do |f|
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
            # TODO remove this dirty hack some day
            squash_root_alias = @work_dir
            squash_root_alias += '/' unless '/' == squash_root_alias[-1]
            raise 'logic error' unless ':/' == squash_root_alias[1..2]
            squash_root_alias = "/cygdrive/#{squash_root_alias[0].downcase}/#{squash_root_alias[3..-1]}"
            f.puts "#define ENCLOSE_IO_ROOT_ALIAS #{squash_root_alias.inspect}"
            squash_root_alias2 = squash_root_alias[11..-1]
            if squash_root_alias2 && squash_root_alias2.length > 1
              f.puts "#define ENCLOSE_IO_ROOT_ALIAS2 #{squash_root_alias2.inspect}"
            end
          else
            f.puts "#define ENCLOSE_IO_ENTRANCE #{mempath(@entrance).inspect}"
          end
        end
        if @options[:auto_update_url] && @options[:auto_update_base]
          f.puts "#define ENCLOSE_IO_AUTO_UPDATE 1"
          f.puts "#define ENCLOSE_IO_AUTO_UPDATE_BASE #{@options[:auto_update_base].inspect}"
          urls = URI.split(@options[:auto_update_url])
          raise 'logic error' unless 9 == urls.length
          port = urls[3]
          if port.nil?
            if 'https' == urls[0]
              port = 443
            else
              port = 80
            end
          end
          f.puts "#define ENCLOSE_IO_AUTO_UPDATE_URL_Scheme #{urls[0].inspect}" if urls[0]
          f.puts "#define ENCLOSE_IO_AUTO_UPDATE_URL_Userinfo #{urls[1].inspect}" if urls[1]
          f.puts "#define ENCLOSE_IO_AUTO_UPDATE_URL_Host #{urls[2].inspect}" if urls[2]
          if Gem.win_platform?
            f.puts "#define ENCLOSE_IO_AUTO_UPDATE_URL_Port #{port.to_s.inspect}"
          else
            f.puts "#define ENCLOSE_IO_AUTO_UPDATE_URL_Port #{port}"
          end
          f.puts "#define ENCLOSE_IO_AUTO_UPDATE_URL_Registry #{urls[4].inspect}" if urls[4]
          f.puts "#define ENCLOSE_IO_AUTO_UPDATE_URL_Path #{urls[5].inspect}" if urls[5]
          f.puts "#define ENCLOSE_IO_AUTO_UPDATE_URL_Opaque #{urls[6].inspect}" if urls[6]
          f.puts "#define ENCLOSE_IO_AUTO_UPDATE_URL_Query #{urls[7].inspect}" if urls[7]
          f.puts "#define ENCLOSE_IO_AUTO_UPDATE_URL_Fragment #{urls[8].inspect}" if urls[8]
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
      @utils.run("call vcbuild.bat #{@options[:debug] ? 'debug' : ''} #{@options[:vcbuild_args]}")
    end
    src = File.join(@tmpdir_node, (@options[:debug] ? 'Debug\\node.exe' : 'Release\\node.exe'))
    @utils.cp(src, @options[:output])
  end

  def compile_mac
    @utils.chdir(@tmpdir_node) do
      # --without-intl=none fixes: icutrim.py - it tries to run a binary made for linux on mac
      # --cross-compiling is required require host executables rather than target ones
      # --without-snapshot avoids mksnapshot to run on host platform after build
      @utils.run("./configure #{@options[:debug] ? '--debug --xcode' : ''} #{@options[:os] ? '--cross-compiling --without-snapshot  --with-intl=none': ''} #{@options[:os] ? '--dest-os=' + @options[:os]: ''} #{@options[:arch] ? '--dest-cpu=' + @options[:arch]: ''}")
      @utils.run("make #{@options[:make_args]}")
    end
    src = File.join(@tmpdir_node, "out/#{@options[:debug] ? 'Debug' : 'Release'}/node")
    @utils.cp(src, @options[:output])
  end

  def compile_linux
    @utils.chdir(@tmpdir_node) do
      # --without-intl=none fixes: icutrim.py - it tries to run a binary made for linux on mac
      # --cross-compiling is required require host executables rather than target ones
      # --without-snapshot avoids mksnapshot to run on host platform after build
      @utils.run("./configure #{@options[:debug] ? '--debug' : ''} #{@options[:os] ? '--cross-compiling --without-snapshot  --with-intl=none': ''} #{@options[:os] ? '--dest-os=' + @options[:os]: ''} #{@options[:arch] ? '--dest-cpu=' + @options[:arch]: ''}")
      @utils.run("make #{@options[:make_args]}")
    end
    src = File.join(@tmpdir_node, "out/#{@options[:debug] ? 'Debug' : 'Release'}/node")
    @utils.cp(src, @options[:output])
  end

  def mempath(path)
    path = File.expand_path(path)
    raise "path #{path} should start with #{@root}" unless @root == path[0...(@root.size)]
    "#{MEMFS}#{path[(@root.size)..-1]}"
  end

  def copypath(path)
    path = File.expand_path(path)
    raise 'Logic error 1 in copypath' unless @root == path[0...(@root.size)]
    ret = File.join(@copy_dir, path[(@root.size)..-1])
    raise 'Logic error 2 in copypath' unless File.exist?(ret)
    ret
  end
end
