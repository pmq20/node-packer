# Copyright (c) 2016-2017 Minqi Pan
# 
# This file is part of Node.js Compiler, distributed under the MIT License
# For full terms see the included LICENSE file

require "node/compiler/constants"
require "node/compiler/error"
require "node/compiler/utils"
require "node/compiler/test"
require "node/compiler/npm"
require 'shellwords'
require 'tmpdir'
require 'fileutils'
require 'json'
require 'open3'

module Node
  class Compiler
    def self.node_version
      @node_version ||= peek_node_version
    end
    
    def self.peek_node_version
      version_info = File.read(File.join(VENDOR_DIR, 'node/src/node_version.h'))
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
    end
    
    def initialize(entrance, options = {})
      @options = options
      @entrance = entrance

      init_options
      init_entrance
      init_tmpdir
      init_libsquash
    end

    def init_entrance
      # Important to expand_path; otherwiser the while would not be right
      @entrance = File.expand_path(@entrance)
      raise Error, "Cannot find entrance #{@entrance}." unless File.exist?(@entrance)
      if @options[:project_root]
        @project_root = File.expand_path(@options[:project_root])
      else
        @project_root = File.dirname(@entrance)
        # this while has to correspond with the expand_path above
        while !File.exist?(File.expand_path('./package.json', @project_root))
          break if '/' == @project_root
          @project_root = File.expand_path('..', @project_root)
        end
      end
      unless File.exist?(File.expand_path('./package.json', @project_root))
        raise Error, "Cannot find a package.json at the project root #{@project_root}"
      end
    end

    def init_options
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
        npm = ::Node::Compiler::Npm.new(@options[:npm_package], @options[:npm_package_version], @options[:tmpdir])
        @entrance = npm.get_entrance(@entrance)
      end
    end

    def init_tmpdir
      if @options[:tmpdir].include? @project_root
        raise Error, "tmpdir #{@options[:tmpdir]} cannot reside inside the project root #{@project_root}."
      end

      Utils.prepare_tmpdir(@options[:tmpdir])
      @vendor_node = File.join(@options[:tmpdir], 'node')
    end
    
    def init_libsquash
      @vendor_squash_dir = File.join @options[:tmpdir], 'libsquash'
      raise "#{@vendor_squash_dir} does not exist" unless Dir.exist?(@vendor_squash_dir)
      @vendor_squash_include_dir = File.join(@vendor_squash_dir, 'include')
      @vendor_squash_build_dir = File.join(@vendor_squash_dir, 'build')
      STDERR.puts "-> FileUtils.mkdir_p #{@vendor_squash_build_dir}"
      FileUtils.mkdir_p(@vendor_squash_build_dir)
      raise "#{@vendor_squash_build_dir} does not exist" unless Dir.exist?(@vendor_squash_build_dir)

      @vendor_zlib_build_include_dir = File.join(@vendor_zlib_build_dir, 'include')
      @vendor_zlib_build_zlibfile = File.join(@vendor_zlib_build_dir, 'lib/libz.a')
    end
    
    def compile_libsquash
      Utils.chdir(@vendor_squash_build_dir) do
        # TODO ZLIB_LIBRARY_DEBUG:FILEPATH and ZLIB_LIBRARY_RELEASE:FILEPATH
        Utils.run("cmake -DZLIB_INCLUDE_DIR:PATH=#{Shellwords.escape @vendor_zlib_build_include_dir} -DZLIB_LIBRARY_RELEASE:FILEPATH=#{Shellwords.escape @vendor_zlib_build_zlibfile} ..")
        Utils.run("cmake --build .")
        Utils.remove_dynamic_libs(@vendor_squash_build_dir)
        Utils.copy_static_libs(@vendor_squash_build_dir, @vendor_ruby)
      end
    end

    def prepared?
      ret = false
      Utils.chdir(@vendor_node) do
        if Gem.win_platform?
          ret = %w{
            libsquash.lib
          }.map { |x| File.exist?(x) }.reduce(true) { |m,o| m && o }
        else
          ret = %w{
            libsquash.a
          }.map { |x| File.exist?(x) }.reduce(true) { |m,o| m && o }
        end
      end
      ret
    end

    def prepare!
      compile_libsquash
    end

    def run!
      prepare! unless prepared?
      Utils.chdir(@project_root) do
        Utils.run("npm install")
      end
      @copy_dir = Utils.inject_memfs(@project_root, @vendor_node)
      inject_entrance
      Gem.win_platform? ? compile_win : compile
    end

    def inject_entrance
      target = File.expand_path('./lib/enclose_io_entrance.js', @vendor_node)
      path = mempath @entrance
      File.open(target, "w") { |f| f.puts %Q`module.exports = "#{path}";` }
      # remove shebang
      lines = File.read(@entrance).lines
      lines[0] = "// #{lines[0]}" if '#!' == lines[0][0..1]
      File.open(copypath(@entrance), "w") { |f| f.print lines.join }
    end

    def compile_win
      Utils.chdir(@vendor_node) do
        Utils.run("call vcbuild.bat #{@options[:vcbuild_args]}")
      end
      STDERR.puts "-> FileUtils.cp(#{File.join(@vendor_node, 'Release\\node.exe')}, #{@options[:output]})"
      FileUtils.cp(File.join(@vendor_node, 'Release\\node.exe'), @options[:output])
    end

    def compile
      Utils.chdir(@vendor_node) do
        Utils.run("./configure")
        Utils.run("make #{@options[:make_args]}")
      end
      STDERR.puts "-> FileUtils.cp(#{File.join(@vendor_node, 'out/Release/node')}, #{@options[:output]})"
      FileUtils.cp(File.join(@vendor_node, 'out/Release/node'), @options[:output])
    end

    def mempath(path)
      path = File.expand_path(path)
      raise 'Logic error in mempath' unless @project_root == path[0...(@project_root.size)]
      "#{MEMFS}#{path[(@project_root.size)..-1]}"
    end

    def copypath(path)
      path = File.expand_path(path)
      raise 'Logic error 1 in copypath' unless @project_root == path[0...(@project_root.size)]
      ret = File.join(@copy_dir, path[(@project_root.size)..-1])
      raise 'Logic error 2 in copypath' unless File.exist?(ret)
      ret
    end
  end
end
