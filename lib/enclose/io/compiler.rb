require "enclose/io/compiler/version"
require "enclose/io/compiler/error"
require 'tmpdir'
require 'fileutils'

module Enclose
  module IO
    class Compiler
      VENDOR_DIR = File.expand_path('../../../../vendor', __FILE__)

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
        @work_dir = Dir.mktmpdir
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

      def inject_entrance
        source = File.expand_path("./node_modules/.bin/#{@bin_name}", @work_dir)
        target = File.expand_path('./lib/_third_party_main.js', @vendor_dir)
        lines = File.read(source).lines
        lines.shift if '#!' == lines[0][0..1]
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
    end
  end
end
