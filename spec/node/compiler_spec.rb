require "spec_helper"

describe ::Node::Compiler do
  it "has a version number" do
    expect(::Node::Compiler::VERSION).not_to be nil
  end

  it "passes all original and enclose.io-added Node.js tests" do
    instance = ::Node::Compiler.new
    instance.test!
  end

  it 'builds coffee out of coffee-script' do
    file = Tempfile.new('coffee-test-artifact')
    file.close
    instance = ::Node::Compiler.new('coffee-script',
                                    '1.11.1',
                                    'coffee',
                                    file.path)
    instance.run!
    expect(File.exist?(file.path)).to be true
    expect(File.size(file.path)).to be >= 1_000_000
    File.chmod(0777, file.path)
    expect(`#{Shellwords.escape file.path} --help`).to include %q{If called without options, `coffee` will run your script.}
    expect(`#{Shellwords.escape file.path} --eval "console.log(((x) -> x * x)(8))"`.to_i).to be 64
  end
end
