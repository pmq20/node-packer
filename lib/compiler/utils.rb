# frozen_string_literal: true

# Copyright (c) 2017 - 2020 Minqi Pan et al.
#
# This file is part of Node.js Packer, distributed under the MIT License
# For full terms see the included LICENSE file

require 'shellwords'
require 'tmpdir'
require 'fileutils'
require 'open3'

class Compiler
  # A collection of utility functions used by the packer
  class Utils
    def initialize(options = {})
      @options = options

      @capture_io = nil
    end

    def capture_run_io(log_name)
      log_file = File.join @options[:tmpdir], "#{log_name}.log"

      warn "=> Saving output to #{log_file}"

      open log_file, 'w' do |io| # rubocop:disable Security/Open
        @capture_io = io

        yield
      end
    rescue Error
      IO.copy_stream log_file, $stdout
      raise
    ensure
      @capture_io = nil
    end

    def escape(arg)
      Shellwords.escape(arg)
    end

    def run(*args)
      unless @options[:quiet]
        message =
          if args.first.is_a? Hash
            env = args.first
            env = env.map do |name, value|
              value = escape(value)
              [name, value].join('=')
            end.join(' ')
            "#{env} #{args[1..].join(' ')}"
          else
            args
          end
        warn "-> #{message}"
      end

      options = {}

      if @capture_io
        options[:out] = @capture_io
        options[:err] = @capture_io
      end

      success = system(*args, **options)

      return if success

      raise Error, "Failed running #{args}"
    end

    def run_allow_failures(*args)
      warn "-> Running (allowing failures) #{args}" unless @options[:quiet]
      pid = spawn(*args)
      _pid, status = Process.wait2(pid)
      status
    end

    def chdir(path, &block)
      warn "-> cd #{path}" unless @options[:quiet]
      Dir.chdir(path, &block)
      warn "-> cd #{Dir.pwd}" unless @options[:quiet]
    end

    def cp(from, to)
      warn "-> cp #{from.inspect} #{to.inspect}" unless @options[:quiet]
      FileUtils.cp(from, to)
    end

    def cp_r(from, to, options = {})
      warn "-> cp -r #{from.inspect} #{to.inspect}" unless @options[:quiet]
      FileUtils.cp_r(from, to, options)
    end

    def rm(path)
      warn "-> rm #{path}" unless @options[:quiet]
      FileUtils.rm(path)
    end

    def rm_f(path)
      warn "-> rm -f #{path}" unless @options[:quiet]
      FileUtils.rm_f(path)
    end

    def rm_rf(path)
      warn "-> rm -rf #{path}" unless @options[:quiet]
      FileUtils.rm_rf(path)
    end

    def mkdir(path)
      warn "-> mkdir #{path}" unless @options[:quiet]
      FileUtils.mkdir(path)
    end

    def mkdir_p(path)
      warn "-> mkdir -p #{path}" unless @options[:quiet]
      FileUtils.mkdir_p(path)
    end

    def remove_dynamic_libs(path)
      %w[dll dylib so].each do |extname|
        Dir["#{path}/**/*.#{extname}"].each do |x|
          rm_f(x)
        end
      end
    end

    def copy_static_libs(path, target)
      %w[lib a].each do |extname|
        Dir["#{path}/*.#{extname}"].each do |x|
          cp(x, target)
        end
      end
    end

    def default_make_j_arg
      begin
        possible_val = `nproc --all`.to_s.strip.to_i
      rescue Errno::ENOENT
        begin
          possible_val = `sysctl -n hw.activecpu`.to_s.strip.to_i
        rescue Errno::ENOENT
          return 4
        end
      end
      return possible_val if possible_val.positive?

      4
    end
  end
end
