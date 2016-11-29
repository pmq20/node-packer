require 'simplecov'
SimpleCov.start

if ENV['CI']
  require 'codecov'
  require 'certified' if Gem.win_platform?
  SimpleCov.formatter = SimpleCov::Formatter::Codecov
end

$LOAD_PATH.unshift File.expand_path("../../lib", __FILE__)
require "node/compiler"
require 'tempfile'
require 'tmpdir'
