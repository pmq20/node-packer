# Copyright (c) 2016-2017 Minqi Pan
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

  it "passes all original and our tests" do
    x = ::Node::Compiler::Test.new(tmpdir)
    x.run!
  end

  it 'builds coffee out of coffee-script' do
    file = Tempfile.new('coffee-test-artifact')
    file.close
    opts = {
      npm_package: 'coffee-script',
      npm_package_version: '1.11.1',
      output: file.path,
      tmpdir: tmpdir,
      vcbuild_args: 'nosign',
    }
    instance = ::Node::Compiler.new('coffee', opts)
    instance.run!
    expect(File.exist?(file.path)).to be true
    expect(File.size(file.path)).to be >= 1_000_000
    File.chmod(0777, file.path)
    expect(`#{Shellwords.escape file.path} --help`).to include %q{If called without options, `coffee` will run your script.}
    expect(`#{Shellwords.escape file.path} --eval "console.log(((x) -> x * x)(8))"`.to_i).to be 64
  end
end
