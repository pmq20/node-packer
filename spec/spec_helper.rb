if ENV['ENCLOSE_IO_TEST_OpenSSL_VERIFY_NONE']
  OpenSSL::SSL::VERIFY_PEER = OpenSSL::SSL::VERIFY_NONE
end

require 'simplecov'
SimpleCov.start

if ENV['CI']
  require 'codecov'
  SimpleCov.formatter = SimpleCov::Formatter::Codecov
end

$LOAD_PATH.unshift File.expand_path("../../lib", __FILE__)
require "enclose/io/compiler"
require 'tempfile'
require 'tmpdir'

unless ENV['ENCLOSE_IO_TEST_NODE_VERSION']
  STDERR.puts %Q{
    Please set ENV['ENCLOSE_IO_TEST_NODE_VERSION']

    Possible values:
      #{Compiler.node_versions.join(', ')}
  }
  exit -1
end
