# Copyright (c) 2016-2017 Minqi Pan <pmq2001@gmail.com>
# 
# This file is part of Node.js Compiler, distributed under the MIT License
# For full terms see the included LICENSE file

require "compiler/constants"
require "compiler/error"
require "compiler/utils"
require 'shellwords'
require 'tmpdir'
require 'fileutils'
require 'json'
require 'open3'

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

    check_base_ruby_version!

    init_options
    init_entrance
    init_tmpdir
    init_libsquash

    if RbConfig::CONFIG['host_os'] =~ /darwin|mac os/i
      @extra_cc_arg = '-mmacosx-version-min=10.7'
    else
      @extra_cc_arg = ''
    end
  end

  def check_base_ruby_version!
    expectation = "ruby 2.4.0"
    got = `ruby -v`
    unless got.include?(expectation)
      msg  = "Please make sure to have installed the correct version of ruby in your environment\n"
      msg += "Expecting #{expectation}; yet got #{got}"
      raise Error, msg
    end
  end

  def init_entrance
    # Important to expand_path; otherwiser the while would not be right
    @entrance = File.expand_path(@entrance)
    raise Error, "Cannot find entrance #{@entrance}." unless File.exist?(@entrance)
    if @options[:root]
      @root = File.expand_path(@options[:root])
    else
      @root = File.dirname(@entrance)
      # this while has to correspond with the expand_path above
      while !File.exist?(File.expand_path('./package.json', @root))
        break if '/' == @root
        @root = File.expand_path('..', @root)
      end
    end
    unless File.exist?(File.expand_path('./package.json', @root))
      raise Error, "Cannot find a package.json inside #{@root}"
    end
  end

  def init_options
    @options[:npm] ||= 'npm'
    if Gem.win_platform?
      @options[:output] ||= 'a.exe'
    else
      @options[:output] ||= 'a.out'
    end
    @options[:output] = File.expand_path(@options[:output])

    @options[:tmpdir] ||= File.expand_path("nodec", Dir.tmpdir)
    @options[:tmpdir] = File.expand_path(@options[:tmpdir])
  end

  def init_tmpdir
    if @options[:tmpdir].include? @root
      raise Error, "tmpdir #{@options[:tmpdir]} cannot reside inside #{@root}."
    end

    Utils.prepare_tmpdir(@options[:tmpdir])
    @vendor_node = File.join(@options[:tmpdir], 'node')
    @vendor_node_zlib = File.join(@vendor_node, 'deps', 'zlib')
    @vendor_node_uv = File.join(@vendor_node, 'deps', 'uv')
    @vendor_node_src = File.join(@vendor_node, 'src')
    @vendor_node_enclose_io = File.join(@vendor_node, 'enclose_io')
    @vendor_node_squash_include = File.join(@vendor_node, 'squash_include')
  end
  
  def init_libsquash
    @vendor_squash_dir = File.join @options[:tmpdir], 'libsquash'
    raise "#{@vendor_squash_dir} does not exist" unless Dir.exist?(@vendor_squash_dir)
    @vendor_squash_include_dir = File.join(@vendor_squash_dir, 'include')
    @vendor_squash_build_dir = File.join(@vendor_squash_dir, 'build')
    @vendor_squash_sample_dir = File.join(@vendor_squash_dir, 'sample')
    STDERR.puts "-> FileUtils.mkdir_p #{@vendor_squash_build_dir}"
    FileUtils.mkdir_p(@vendor_squash_build_dir)
    raise "#{@vendor_squash_build_dir} does not exist" unless Dir.exist?(@vendor_squash_build_dir)
  end
  
  def compile_libsquash
    Utils.chdir(@vendor_squash_build_dir) do
      Utils.run("cmake -DCMAKE_BUILD_TYPE=Release -DCMAKE_C_FLAGS=#{Shellwords.escape @extra_cc_arg} -DZLIB_INCLUDE_DIR:PATH=#{Shellwords.escape @vendor_node_zlib} ..")
      Utils.run("cmake --build . --config Release")
      Utils.remove_dynamic_libs(@vendor_squash_build_dir)
      if Gem.win_platform?
        Utils.copy_static_libs(File.join(@vendor_squash_build_dir, 'Release'), @vendor_node)
      else
        Utils.copy_static_libs(@vendor_squash_build_dir, @vendor_node)
      end
    end
  end

  def prepared?
    Utils.chdir(@vendor_node) do
      if Gem.win_platform? && File.exists?('squash.lib')
        STDERR.puts "-> FileUtils.mv('squash.lib', 'libsquash.a')"
        FileUtils.mv('squash.lib', 'libsquash.a')
      end
      return (
        %w{
          libsquash.a
        }.map { |x| File.exist?(x) }.reduce(true) { |m,o| m && o }
      )
    end
  end

  def prepare!
    compile_libsquash
  end

  def run!
    prepare! unless prepared?
    raise 'Unable to prepare' unless prepared?
    npm_install
    make_enclose_io_memfs
    make_enclose_io_vars
    Gem.win_platform? ? compile_win : compile
  end

  def npm_install
    @work_dir = File.join(@options[:tmpdir], '__work_dir__')
    STDERR.puts "-> FileUtils.rm_rf(#{@work_dir})"
    FileUtils.rm_rf(@work_dir)
    STDERR.puts "-> FileUtils.mkdir_p #{@work_dir}"
    FileUtils.mkdir_p(@work_dir)

    @work_dir_inner = File.join(@work_dir, '__enclose_io_memfs__')
    STDERR.puts "-> FileUtils.mkdir_p #{@work_dir_inner}"
    FileUtils.mkdir_p(@work_dir_inner)

    FileUtils.cp_r(@root, @work_dir_inner)
    Utils.chdir(@work_dir_inner) do
      Utils.run("#{Shellwords.escape @options[:npm]} install")
      STDERR.puts `git status`
      STDERR.puts "-> FileUtils.rm_rf('.git')"
      FileUtils.rm_rf('.git')
    end
  end

  def make_enclose_io_memfs
    STDERR.puts "-> FileUtils.cp_r(#{@vendor_squash_sample_dir}, #{@vendor_node_enclose_io})"
    FileUtils.cp_r(@vendor_squash_sample_dir, @vendor_node_enclose_io)

    STDERR.puts "-> FileUtils.cp_r(#{@vendor_squash_include_dir}, #{@vendor_node_squash_include})"
    FileUtils.cp_r(@vendor_squash_include_dir, @vendor_node_squash_include)

    Utils.chdir(@vendor_node) do
      Utils.run("mksquashfs -version")
      Utils.run("mksquashfs #{Shellwords.escape @work_dir} enclose_io/enclose_io_memfs.squashfs")
      bytes = IO.binread('enclose_io/enclose_io_memfs.squashfs').bytes
      # TODO slow operation
      # remember to change vendor/libsquash/sample/enclose_io_memfs.c as well
      File.open("enclose_io/enclose_io_memfs.c", "w") do |f|
        f.puts '#include <stdint.h>'
        f.puts '#include <stddef.h>'
        f.puts ''
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
      # TODO slow operation
      Utils.run("cc #{@extra_cc_arg} -c enclose_io/enclose_io_memfs.c -o enclose_io/enclose_io_memfs.o")
      raise 'failed to compile enclose_io/enclose_io_memfs.c' unless File.exist?('enclose_io/enclose_io_memfs.o')
      Utils.run("cc #{@extra_cc_arg} -Ienclose_io -Isquash_include -c enclose_io/enclose_io_intercept.c -o enclose_io/enclose_io_intercept.o")
      raise 'failed to compile enclose_io/enclose_io_intercept.c' unless File.exist?('enclose_io/enclose_io_intercept.o')
    end
  end
  
  def make_enclose_io_vars
    Utils.chdir(@vendor_node) do
      File.open("enclose_io/enclose_io.h", "w") do |f|
        # remember to change vendor/libsquash/sample/enclose_io.h as well
        f.puts '#ifndef ENCLOSE_IO_H_999BC1DA'
        f.puts '#define ENCLOSE_IO_H_999BC1DA'
        f.puts ''
        f.puts '#include "enclose_io_common.h"'
        f.puts '#include "enclose_io_intercept.h"'
        f.puts ''
        chr_type = Gem.win_platform? ? 'wchar_t' : 'char'
        f.puts %Q!
#define ENCLOSE_IO_ENTRANCE do { \\\n\
	new_argv = (#{chr_type} **)malloc( (argc + 1) * sizeof(#{chr_type} *)); \\\n\
	assert(new_argv); \\\n\
	new_argv[0] = argv[0]; \\\n\
	new_argv[1] = #{mempath(@entrance).inspect}; \\\n\
	for (size_t i = 1; i < argc; ++i) { \\\n\
		new_argv[2 + i - 1] = argv[i]; \\\n\
	} \\\n\
	new_argc = argc + 1; \\\n\
	/* argv memory should be adjacent. */ \\\n\
	size_t total_argv_size = 0; \\\n\
	for (size_t i = 0; i < new_argc; ++i) { \\\n\
		total_argv_size += strlen(new_argv[i]) + 1; \\\n\
	} \\\n\
	argv_memory = (#{chr_type} *)malloc( (total_argv_size) * sizeof(#{chr_type})); \\\n\
	assert(argv_memory); \\\n\
	for (size_t i = 0; i < new_argc; ++i) { \\\n\
		memcpy(argv_memory, new_argv[i], strlen(new_argv[i]) + 1); \\\n\
		new_argv[i] = argv_memory; \\\n\
		argv_memory += strlen(new_argv[i]) + 1; \\\n\
	} \\\n\
	assert(argv_memory - new_argv[0] == total_argv_size); \\\n\
} while(0)
        !.strip
        f.puts '#endif'
        f.puts ''
      end
    end
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
    raise 'Logic error in mempath' unless @root == path[0...(@root.size)]
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
