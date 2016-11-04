require "spec_helper"

describe ::Enclose::IO::Compiler do
  it "has a version number" do
    expect(::Enclose::IO::Compiler::VERSION).not_to be nil
  end
  
  it 'builds coffee-script and passes all Node.js tests' do
    file = Tempfile.new('coffee-test-artifact')
    argv = [
      'node-v6.8.0',
      'coffee-script',
      '1.11.1',
      'cake',
      file.path
    ]
    instance = ::Enclose::IO::Compiler.new argv
    instance.run!
    expect(File.exist?(file.path)).to be true
    expect(File.size(file.path)).to be >= 1_000_000
    expect(`#{Shellwords.escape file.path} --help`).to include %q{If called without options, `coffee` will run your script.}
    expect(`#{Shellwords.escape file.path} --eval 'console.log(((x) -> x * x)(8))'`.to_i).to be 64
    instance.test!
  end
end
