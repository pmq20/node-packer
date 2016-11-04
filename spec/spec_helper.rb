require 'simplecov'
SimpleCov.start

if ENV['CI']
  require 'codecov'
  require 'certified' if Gem.win_platform?
  SimpleCov.formatter = SimpleCov::Formatter::Codecov
end

unless ENV['ENCLOSE_IO_TEST_NODE_VERSION']
  STDERR.puts %Q{
    Please set ENV['ENCLOSE_IO_TEST_NODE_VERSION']

    Possible values:
      #{Compiler.node_versions.join(', ')}
  }
  exit -1
end

$LOAD_PATH.unshift File.expand_path("../../lib", __FILE__)
require "enclose/io/compiler"
require 'tempfile'
require 'tmpdir'
