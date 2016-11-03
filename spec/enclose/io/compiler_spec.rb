require "spec_helper"

describe ::Enclose::IO::Compiler do
  it "has a version number" do
    expect(::Enclose::IO::Compiler::VERSION).not_to be nil
  end
  
  it 'builds coffee-script and passes all Node.js tests' do
    argv = %w{node-v6.8.0 coffee-script 1.11.1 cake /tmp/coffee-1.11.1-node-v6.8.0-darwin-x64}
    instance = ::Enclose::IO::Compiler.new argv
    instance.run!
    instance.test!
  end
end
