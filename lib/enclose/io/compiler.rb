require "enclose/io/compiler/version"
require "enclose/io/compiler/error"

module Enclose
  module IO
    class Compiler
      VENDOR_DIR = File.expand_path('../../../../vendor', __FILE__)

      def initialize argv0, argv1
        @node_version = argv0
        @module_name = argv1
        @vendor_dir = File.expand_path("./#{@node_version}", VENDOR_DIR)
        unless File.exists?(@vendor_dir)
          list = Dir[VENDOR_DIR+'/node*'].map {|x| x.gsub(VENDOR_DIR+'/', '')}
          msg = "Does not support #{argv0}, supported are #{list}"
          raise Error, msg
        end
      end
      
      def run!
        chdir(@vendor_dir) do
          run("./configure #{ENV['ENCLOSE_IO_CONFIGURE_ARGS']}")
          run("make #{ENV['ENCLOSE_IO_MAKE_ARGS']}")
        end
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
