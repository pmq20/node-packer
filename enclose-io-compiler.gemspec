lib = File.expand_path('../lib', __FILE__)
$LOAD_PATH.unshift(lib) unless $LOAD_PATH.include?(lib)
require 'enclose/io/compiler/version'

Gem::Specification.new do |spec|
  spec.name          = "enclose-io-compiler"
  spec.version       = Enclose::IO::Compiler::VERSION
  spec.authors       = ["Minqi Pan"]
  spec.email         = ["pmq2001@gmail.com"]

  spec.summary       = %q{Compiler of Enclose.IO}
  spec.description   = %q{Compiler of Enclose.IO which packs your Node.js app into a single executable.}
  spec.homepage      = "http://enclose.io/"
  spec.license       = "MIT"

  spec.files         = `git ls-files -z`.split("\x0")
  spec.bindir        = "exe"
  spec.executables   = spec.files.grep(%r{^exe/}) { |f| File.basename(f) }
  spec.require_paths = ["lib"]

  spec.add_development_dependency "bundler", ">= 1.13.3"
  spec.add_development_dependency "rake", ">= 11.3.0"
  spec.add_development_dependency "rspec", ">= 3.5.0"
end
