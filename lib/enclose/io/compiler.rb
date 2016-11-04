require "enclose/io/compiler/version"
require "enclose/io/compiler/error"
require 'shellwords'
require 'tmpdir'
require 'fileutils'
require 'json'
require 'open3'

module Enclose
  module IO
    class Compiler
      VENDOR_DIR = File.expand_path('../../../../vendor', __FILE__)
      MEMFS = '__enclose_io_memfs__'

      def self.node_versions
        Dir[VENDOR_DIR+'/node*'].map {|x| x.gsub(VENDOR_DIR+'/', '')}
      end

      def initialize(node_version, module_name = nil,
                                   module_version = nil,
                                   bin_name = nil,
                                   output_path = nil)
        @node_version = node_version
        @module_name = module_name
        @module_version = module_version
        @bin_name = bin_name
        @output_path = output_path

        @vendor_dir = File.expand_path("./#{@node_version}", VENDOR_DIR)
        unless File.exist?(@vendor_dir)
          msg = "Does not support #{@node_version}, supported: #{::Enclose::IO::Compiler.node_versions.join ', '}"
          raise Error, msg
        end
      end

      def run!
        prepare_vars
        npm_install
        parse_binaries
        inject_entrance
        inject_memfs
        Gem.win_platform? ? compile_win : compile
      end

      def prepare_vars
        @work_dir = File.expand_path("./enclose-io-compiler/#{@module_name}-#{@module_version}", Dir.tmpdir)
        FileUtils.mkdir_p(@work_dir)
        @package_path = File.join(@work_dir, "node_modules/#{@module_name}/package.json")
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

      def inject_entrance
        target = File.expand_path('./lib/enclose_io_entrance.js', @vendor_dir)
        prj_home = File.expand_path("node_modules/#{@module_name}", @work_dir)
        bin = File.expand_path(@binaries[@bin_name], prj_home)
        path = mempath bin
        File.open(target, "w") { |f| f.puts %Q`module.exports = "#{path}";` }
        # remove shebang
        lines = File.read(bin).lines
        lines[0] = "// #{lines[0]}" if '#!' == lines[0][0..1]
        File.open(bin, "w") { |f| f.print lines.join }
      end

      def inject_memfs
        target = File.expand_path("./lib/#{MEMFS}", @vendor_dir)
        FileUtils.remove_entry_secure(target) if File.exist?(target)
        FileUtils.cp_r(@work_dir, target)
        manifest = File.expand_path('./enclose_io_manifest.txt', @vendor_dir)
        File.open(manifest, "w") do |f|
          Dir["#{target}/**/*"].each do |fullpath|
            next unless File.file?(fullpath)
            if 0 == File.size(fullpath)
              # Fix VC++ Error C2466
              File.open(fullpath, 'w') { |f| f.puts ' ' }
            end
            entry = "lib/#{fullpath[(fullpath.index MEMFS)..-1]}"
            f.puts entry
          end
        end
      end

      def compile_win
        chdir(@vendor_dir) do
          run("call vcbuild.bat #{ENV['ENCLOSE_VCBUILD_ARGS']}")
          FileUtils.cp('Release\\node.exe', @output_path)
        end
      end

      def compile
        chdir(@vendor_dir) do
          run("./configure #{ENV['ENCLOSE_IO_CONFIGURE_ARGS']}")
          run("make #{ENV['ENCLOSE_IO_MAKE_ARGS']}")
          FileUtils.cp('out/Release/node', @output_path)
        end
      end

      def test!
        chdir(@vendor_dir) do
          FileUtils.rm_f(Gem.win_platform? ? 'Release\\node.exe' : 'out/Release/node')
          target = File.expand_path('./lib/enclose_io_entrance.js', @vendor_dir)
          File.open(target, "w") { |f| f.puts %Q`module.exports = false;` }
          if Gem.win_platform?
            run('call vcbuild.bat release nosign x64 noprojgen nobuild test-ci ignore-flaky')
          else
            run("./configure #{ENV['ENCLOSE_IO_CONFIGURE_ARGS']}")
            run("make #{ENV['ENCLOSE_IO_MAKE_ARGS']}")
            run("FLAKY_TESTS_MODE=dontcare FLAKY_TESTS=dontcare ENCLOSE_IO_USE_ORIGINAL_NODE=1 make test-ci")
          end
        end
      end

      def clean_work_dir
        FileUtils.remove_entry_secure @work_dir
      end

      private

      def run(cmd)
        STDERR.puts "$ #{cmd}"
        pid = spawn(cmd)
        pid, status = Process.wait2(pid)
        raise Error, "#{cmd} failed!" unless status.success?
      end

      def chdir(path)
        STDERR.puts "$ cd #{path}"
        Dir.chdir(path) { yield }
        STDERR.puts "$ cd #{Dir.pwd}"
      end
      
      def mempath(path)
        path = File.expand_path(path)
        if @work_dir == path[0...(@work_dir.size)]
          return "#{MEMFS}#{path[(@work_dir.size)..-1]}"
        else
          raise Error, 'Logic error in mempath'
        end
      end
    end
  end
end
