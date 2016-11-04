require 'simplecov'
SimpleCov.start

if ENV['CI']
  require 'codecov'
  SimpleCov.formatter = SimpleCov::Formatter::Codecov
  if ENV['ENCLOSE_IO_TEST_OpenSSL_VERIFY_NONE']
    SimpleCov.at_exit do
      # Fix appveyor's buggy Ruby
      STDERR.puts "OpenSSL::SSL::VERIFY_PEER = OpenSSL::SSL::VERIFY_NONE"
      OpenSSL::SSL::VERIFY_PEER = OpenSSL::SSL::VERIFY_NONE
      SimpleCov.result.format!
    end
  end
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
