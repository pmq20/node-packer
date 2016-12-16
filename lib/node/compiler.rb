require "node/compiler/version"
require "node/compiler/error"
require 'shellwords'
require 'tmpdir'
require 'fileutils'
require 'json'
require 'open3'

module Node
  class Compiler
    VENDOR_DIR = File.expand_path('../../../vendor', __FILE__)
    MEMFS = '/__enclose_io_memfs__'

    def initialize(entrance, options = {})
      @entrance = entrance
      init_entrance

      @options = options
      init_options
    end

    def init_entrance
      # Important to expand_path; otherwiser the while would not be right
      @entrance = File.expand_path(@entrance)
      raise Error, "Cannot find entrance #{@entrance}." unless File.exist?(@entrance)
      @project_root = File.dirname(@entrance)
      # this while has to correspond with the expand_path above
      while !File.exist?(File.expand_path('./package.json', @project_root))
        if '/' == @project_root
          raise Error, "Cannot locate the root of the project. Is #{@entrance} inside a Node.js project?"
        end
        @project_root = File.expand_path('..', @project_root)
      end
    end

    def init_options
      if Gem.win_platform?
        @options[:output] ||= 'a.exe'
      else
        @options[:output] ||= 'a.out'
      end
      @options[:output] = File.expand_path(@options[:output])

      @options[:tempdir] ||= '/tmp/nodec'
      @options[:tempdir] = File.expand_path(@options[:tempdir])
      if @options[:tempdir].include? @project_root
        raise Error, "tempdir #{@options[:tempdir]} cannot reside inside the project root #{@project_root}."
      end

      STDERR.puts "-> FileUtils.mkdir_p(#{@options[:tempdir]})"
      FileUtils.mkdir_p(@options[:tempdir])
      Dir[VENDOR_DIR + '/*'].each do |dirpath|
        target = File.join(@options[:tempdir], File.basename(dirpath))
        unless Dir.exist?(target)
          STDERR.puts "-> FileUtils.cp_r(#{dirpath}, #{target})"
          FileUtils.cp_r(dirpath, target)
        end
      end
      @vendor_dir = File.join(@options[:tempdir], NODE_VERSION)
    end

    def run!
      inject_memfs(@project_root)
      inject_entrance
      Gem.win_platform? ? compile_win : compile
    end

    def inject_memfs(source)
      @copydir = File.expand_path("./lib#{MEMFS}", @vendor_dir)
      if File.exist?(@copydir)
        STDERR.puts "-> FileUtils.remove_entry_secure(#{@copydir})"
        FileUtils.remove_entry_secure(@copydir)
      end
      STDERR.puts "-> FileUtils.cp_r(#{source}, #{@copydir})"
      FileUtils.cp_r(source, @copydir)
      manifest = File.expand_path('./enclose_io_manifest.txt', @vendor_dir)
      File.open(manifest, "w") do |f|
        Dir["#{@copydir}/**/*"].each do |fullpath|
          next unless File.file?(fullpath)
          if 0 == File.size(fullpath) && Gem.win_platform?
            # Fix VC++ Error C2466
            # TODO: what about empty file semantics?
            File.open(fullpath, 'w') { |f| f.puts ' ' }
          end
          entry = "lib#{fullpath[(fullpath.index MEMFS)..-1]}"
          f.puts entry
        end
      end
    end

    def inject_entrance
      target = File.expand_path('./lib/enclose_io_entrance.js', @vendor_dir)
      path = mempath @entrance
      File.open(target, "w") { |f| f.puts %Q`module.exports = "#{path}";` }
      # remove shebang
      lines = File.read(@entrance).lines
      lines[0] = "// #{lines[0]}" if '#!' == lines[0][0..1]
      File.open(copypath(@entrance), "w") { |f| f.print lines.join }
    end

    def compile_win
      chdir(@vendor_dir) do
        run("call vcbuild.bat #{@options[:vcbuild_args]}")
      end
      STDERR.puts "-> FileUtils.cp(#{File.join(@vendor_dir, 'Release\\node.exe')}, #{@options[:output]})"
      FileUtils.cp(File.join(@vendor_dir, 'Release\\node.exe'), @options[:output])
    end

    def compile
      chdir(@vendor_dir) do
        run("./configure")
        run("make #{@options[:make_args]}")
      end
      STDERR.puts "-> FileUtils.cp(#{File.join(@vendor_dir, 'out/Release/node')}, #{@options[:output]})"
      FileUtils.cp(File.join(@vendor_dir, 'out/Release/node'), @options[:output])
    end

    def test!
      chdir(@vendor_dir) do
        inject_memfs(File.expand_path('./test/fixtures', @vendor_dir))
        STDERR.puts "-> FileUtils.rm_f(#{Gem.win_platform? ? 'Release\\node.exe' : 'out/Release/node'})"
        FileUtils.rm_f(Gem.win_platform? ? 'Release\\node.exe' : 'out/Release/node')
        File.open(File.expand_path('./lib/enclose_io_entrance.js', @vendor_dir), "w") { |f| f.puts 'module.exports = false;' }
        test_env = {
                     'FLAKY_TESTS_MODE' => 'dontcare',
                     'FLAKY_TESTS' => 'dontcare',
                     'ENCLOSE_IO_USE_ORIGINAL_NODE' => '1',
                     'ENCLOSE_IO_ALWAYS_USE_ORIGINAL_NODE' => '1',
                   }
        if Gem.win_platform?
          run(test_env, 'call vcbuild.bat nosign test-ci ignore-flaky')
        else
          run("./configure")
          run("make #{@options[:make_args]}")
          run(test_env, "make test-ci")
        end
      end
    end
    
    def debug
      STDERR.puts "@entrance: #{@entrance}"
      STDERR.puts "@project_root: #{@project_root}"
      STDERR.puts "@options: #{@options}"
    end

    private

    def run(*args)
      STDERR.puts "-> Running #{args}"
      pid = spawn(*args)
      pid, status = Process.wait2(pid)
      raise Error, "Failed running #{args}" unless status.success?
    end

    def chdir(path)
      STDERR.puts "-> cd #{path}"
      Dir.chdir(path) { yield }
      STDERR.puts "-> cd #{Dir.pwd}"
    end
  
    def mempath(path)
      path = File.expand_path(path)
      raise 'Logic error in mempath' unless @project_root == path[0...(@project_root.size)]
      "#{MEMFS}#{path[(@project_root.size)..-1]}"
    end
  
    def copypath(path)
      path = File.expand_path(path)
      raise 'Logic error 1 in copypath' unless @project_root == path[0...(@project_root.size)]
      ret = File.join(@copydir, path[(@project_root.size)..-1])
      raise 'Logic error 2 in copypath' unless File.exist?(ret)
      ret
    end
  end
end
