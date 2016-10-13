require "enclose/io/compiler/version"
require "enclose/io/compiler/error"
require 'tmpdir'
require 'fileutils'
require 'json'

module Enclose
  module IO
    class Compiler
      VENDOR_DIR = File.expand_path('../../../../vendor', __FILE__)
      MEMFS = '__enclose_io_memfs__'

      def initialize argv0, argv1, argv2, argv3
        @node_version = argv0
        @module_name = argv1
        @module_version = argv2
        @bin_name = argv3
        @vendor_dir = File.expand_path("./#{@node_version}", VENDOR_DIR)
        unless File.exists?(@vendor_dir)
          list = Dir[VENDOR_DIR+'/node*'].map {|x| x.gsub(VENDOR_DIR+'/', '')}
          msg = "Does not support #{argv0}, supported: #{list.join ', '}"
          raise Error, msg
        end
        @work_dir = File.expand_path("./enclose-io-compiler/#{@module_name}-#{@module_version}", ENV['TMPDIR'])
        FileUtils.mkdir_p(@work_dir)
        @package_path = File.join(@work_dir, "node_modules/#{@module_name}/package.json")
        @memroot = File.expand_path('./node_modules', @work_dir)
      end

      def npm_install
        chdir(@work_dir) do
          File.open("package.json", "w") do |f|
            package = %Q({"dependencies": {"#{@module_name}": "#{@module_version}"}})
            f.puts package
          end
          run('npm install')
        end
      end

      def parse_binaries
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
      end

      def inject_memfs
        target = File.expand_path("./lib/#{MEMFS}", @vendor_dir)
        FileUtils.remove_entry_secure(target) if File.exist?(target)
        FileUtils.cp_r(@memroot, target)
      end

      def inject_entrance
        target = File.expand_path('./lib/enclose_io_entrance.js', @vendor_dir)
        prj_home = File.expand_path("node_modules/#{@module_name}", @work_dir)
        bin = File.expand_path(@binaries[@bin_name], prj_home)
        lines = File.read(bin).lines
        lines[0] = "// #{lines[0]}" if '#!' == lines[0][0..1]
        File.open(target, "w") { |f| f.print lines.join }
      end

      def compile
        chdir(@vendor_dir) do
          run("./configure #{ENV['ENCLOSE_IO_CONFIGURE_ARGS']}")
          run("make #{ENV['ENCLOSE_IO_MAKE_ARGS']}")
        end
      end

      def die
        FileUtils.remove_entry_secure @work_dir
      end

      private

      def run(cmd)
        STDERR.puts "$ #{cmd}"
        STDERR.print `#{cmd}`
        raise Error, "#{cmd} failed!" unless $?.success?
      end

      def chdir(path)
        STDERR.puts "$ cd #{path}"
        Dir.chdir(path) { yield }
        STDERR.puts "$ cd #{Dir.pwd}"
      end
      
      def mempath(path)
        path = File.expand_path(path)
        if @memroot == path[0...(@memroot.size)]
          return "#{MEMFS}#{path[(@memroot.size)..-1]}"
        else
          raise Error, 'Logic error in mempath'
        end
      end
    end
  end
end
