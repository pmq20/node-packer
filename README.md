# Node.js Compiler

Compiler for Node.js that compiles your Node.js application into a single executable.

[![Travis CI](https://travis-ci.org/pmq20/node-compiler.svg?branch=master)](https://travis-ci.org/pmq20/node-compiler)
[![Appveyor CI](https://ci.appveyor.com/api/projects/status/gap9xne0rayjtynp/branch/master?svg=true)](https://ci.appveyor.com/project/pmq20/node-compiler/branch/master)

## Installation

    gem install node-compiler

You might need to `sudo` if prompted with no-permission errors.

## Usage

    nodec [OPTION]... [ENTRANCE]
      -r, --root=DIR                   Speicifies the path to the root of the application
      -o, --output=FILE                Speicifies the path of the output file
      -d, --tmpdir=DIR                 Speicifies the directory for temporary files
          --make-args=ARGS             Passes extra arguments to make
          --vcbuild-args=ARGS          Passes extra arguments to vcbuild.bat
          --npm=FILE                   Speifices the path of npm
      -v, --version                    Prints the version of nodec and exit
          --node-version               Prints the version of the Node.js runtime and exit
      -h, --help                       Prints this help and exit

## Examples

### Compiling a command-line application

    git clone https://github.com/jashkenas/coffeescript.git
    cd coffeescript
    nodec bin/coffee

### Compiling a web application

    git clone https://github.com/cnodejs/nodeclub.git
    cd nodeclub
    cp config.default.js config.js
    nodec app.js

## Development

After checking out the repo, run `bin/setup` to install dependencies. Then, run `rake` to run the tests. You can also run `bin/console` for an interactive prompt that will allow you to experiment.

To install this gem onto your local machine, run `bundle exec rake install`. Or without installing, run `bundle exec nodec` from the root of your project directory.

To release a new version, update the version number in `version.rb`, and then run `bundle exec rake release`, which will create a git tag for the version, push git commits and tags, and push the `.gem` file to [rubygems.org](https://rubygems.org).

## Contributing

Bug reports and pull requests are welcome on GitHub at https://github.com/pmq20/node-compiler.

## License

Copyright (c) 2016-2017 **Minqi Pan** &lt;pmq2001@gmail.com&gt;, under terms of the [MIT License](http://opensource.org/licenses/MIT).
