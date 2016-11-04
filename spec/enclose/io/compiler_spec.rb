require "spec_helper"

describe ::Enclose::IO::Compiler do
  it "has a version number" do
    expect(::Enclose::IO::Compiler::VERSION).not_to be nil
  end

  it "passes all Node.js tests" do
    ::Enclose::IO::Compiler.node_versions.each do |node_version|
      instance = ::Enclose::IO::Compiler.new node_version
      instance.test!
    end
  end

  it 'builds coffee of coffee-script' do
    file = Tempfile.new('coffee-test-artifact')
    file.close
    instance = ::Enclose::IO::Compiler.new('node-v6.8.0',
                                           'coffee-script',
                                           '1.11.1',
                                           'cake',
                                           file.path)
    instance.run!
    expect(File.exist?(file.path)).to be true
    expect(File.size(file.path)).to be >= 1_000_000
    File.chmod(0777, file.path)
    expect(`#{Shellwords.escape file.path} --help`).to include %q{If called without options, `coffee` will run your script.}
    expect(`#{Shellwords.escape file.path} --eval 'console.log(((x) -> x * x)(8))'`.to_i).to be 64
  end
end
