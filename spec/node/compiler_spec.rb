# Copyright (c) 2016-2017 Minqi Pan <pmq2001@gmail.com>
# 
# This file is part of Node.js Compiler, distributed under the MIT License
# For full terms see the included LICENSE file

require "spec_helper"

tmpdir = File.expand_path("nodec/compiler_spec", Dir.tmpdir)
FileUtils.mkdir_p(tmpdir)

describe ::Node::Compiler do
  it "has a version number" do
    expect(::Node::Compiler::VERSION).not_to be nil
  end

  it "passes all Node.js tests" do
    x = ::Node::Compiler::Test.new(tmpdir)
    x.run!
  end
end
