require 'simplecov'
SimpleCov.start

if ENV['CI']
  require 'codecov'
  SimpleCov.formatter = SimpleCov::Formatter::Codecov
end

$LOAD_PATH.unshift File.expand_path("../../lib", __FILE__)
require "enclose/io/compiler"
require 'tempfile'

