require 'simplecov'
SimpleCov.start

if ENV['CI']
  require 'codecov'
  require 'certified' if Gem.win_platform?
  SimpleCov.formatter = SimpleCov::Formatter::Codecov
end

$LOAD_PATH.unshift File.expand_path("../../lib", __FILE__)
require "enclose/io/compiler"
require 'tempfile'
require 'tmpdir'

RSpec.configure do |config|
  config.before(:suite) do
    unless ENV['ENCLOSE_IO_TEST_NODE_VERSION']
      STDERR.puts %Q{
        Please set ENV['ENCLOSE_IO_TEST_NODE_VERSION']

        Possible values:
          #{::Enclose::IO::Compiler.node_versions.join(', ')}
      }
      exit -1
    end
  end
end

