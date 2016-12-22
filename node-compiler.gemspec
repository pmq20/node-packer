lib = File.expand_path('../lib', __FILE__)
$LOAD_PATH.unshift(lib) unless $LOAD_PATH.include?(lib)

require 'node/compiler'

Gem::Specification.new do |spec|
  spec.name          = "node-compiler"
  spec.version       = ::Node::Compiler::VERSION
  spec.authors       = ["Minqi Pan"]
  spec.email         = ["pmq2001@gmail.com"]

  spec.summary       = %q{Compiler for Node.js}
  spec.description   = %q{Compiler for Node.js which compiles your project into a single executable.}
  spec.homepage      = "https://github.com/pmq20/node-compiler"
  spec.license       = "MIT"

  spec.files         = `git ls-files -z`.split("\x0")
  spec.bindir        = "exe"
  spec.executables   = spec.files.grep(%r{^exe/}) { |f| File.basename(f) }
  spec.require_paths = ["lib"]

  spec.add_development_dependency "bundler", ">= 1.13.3"
  spec.add_development_dependency "rake", ">= 11.3.0"
  spec.add_development_dependency "rspec", ">= 3.5.0"
  spec.add_development_dependency "pry", ">= 0.10.4"
end
