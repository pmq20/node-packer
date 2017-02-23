# Copyright (c) 2017 Minqi Pan <pmq2001@gmail.com>
# 
# This file is part of Node.js Compiler, distributed under the MIT License
# For full terms see the included LICENSE file

require 'shellwords'
require 'tmpdir'
require 'fileutils'
require 'json'

class Compiler
  class NpmPackage
    attr_reader :work_dir

    def initialize(options)
      @module_name = options[:npm_package]
      @module_version = options[:npm_package_version]
      @work_dir = File.expand_path("#{@module_name}-#{@module_version}", options[:tmpdir])
    end

    def stuff_tmpdir
      Utils.rm_rf(@work_dir)
      Utils.mkdir_p(@work_dir)
      Utils.chdir(@work_dir) do
        File.open("package.json", "w") do |f|
          package = %Q({"dependencies": {"#{@module_name}": "#{@module_version}"}})
          f.puts package
        end
      end
    end

    def get_entrance(bin_name)
      @package_path = "node_modules/#{@module_name}/package.json"
      @bin_name = bin_name
      unless File.exist?(@package_path)
        raise Error, "No package.json exist at #{@package_path}."
      end
      @package_json = JSON.parse File.read @package_path
      @binaries = @package_json['bin']
      if @binaries
        STDERR.puts "Detected binaries: #{@binaries}"
      else
        raise Error, "No binaries detected inside #{@package_path}."
      end
      if @binaries[@bin_name]
        STDERR.puts "Using #{@bin_name} at #{@binaries[@bin_name]}"
      else
        raise Error, "No such binary: #{@bin_name}"
      end
      ret = File.expand_path("node_modules/#{@module_name}/#{@binaries[@bin_name]}")
      unless File.exist?(ret)
        raise Error, "Npm install failed to generate #{ret}"
      end
      return File.expand_path("node_modules/#{@module_name}/#{@binaries[@bin_name]}", @work_dir)
    end
  end
end
