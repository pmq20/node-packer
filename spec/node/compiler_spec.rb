# Copyright (c) 2016 Node.js Compiler contributors
# 
# This file is part of Node.js Compiler, distributed under the MIT License
# For full terms see the included LICENSE file

require "spec_helper"

tempdir = File.expand_path("./enclose-io/nodec/compiler_spec", Dir.tmpdir)

describe ::Node::Compiler do
  it "has a version number" do
    expect(::Node::Compiler::VERSION).not_to be nil
  end

  it "passes all original and enclose.io-added Node.js tests" do
    x = ::Node::Compiler::Test.new(tempdir)
    x.run!
  end

  it 'builds coffee out of coffee-script' do
    file = Tempfile.new('coffee-test-artifact')
    file.close

    npm = ::Node::Compiler::Npm.new('coffee-script', '1.11.1')
    entrance = npm.get_entrance('coffee')

    instance = ::Node::Compiler.new(entrance, output: file.path, tempdir: tempdir)
    instance.run!
    expect(File.exist?(file.path)).to be true
    expect(File.size(file.path)).to be >= 1_000_000
    File.chmod(0777, file.path)
    expect(`#{Shellwords.escape file.path} --help`).to include %q{If called without options, `coffee` will run your script.}
    expect(`#{Shellwords.escape file.path} --eval "console.log(((x) -> x * x)(8))"`.to_i).to be 64
  end
end
