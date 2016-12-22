# Node.js Compiler

Compiler for Node.js which compiles your project into a single executable.

http://nodec.enclose.io

[![Linux and Mac OS X Build Status](https://travis-ci.org/enclose-io/node-compiler.svg?branch=master)](https://travis-ci.org/enclose-io/node-compiler)
[![Windows Build status](https://ci.appveyor.com/api/projects/status/f4x3bq5hub3uu3ys/branch/master?svg=true)](https://ci.appveyor.com/project/pmq20/node-compiler/branch/master)
[![Gem Version](https://badge.fury.io/rb/node-compiler.svg)](https://badge.fury.io/rb/node-compiler)

## Installation

    gem install node-compiler

You might need to `sudo` if prompted with no-permission errors.

## Usage

    nodec [OPTION]... [ENTRANCE]
        -p, --project-root=DIR           Speicifies the path to the root of the project
        -o, --output=FILE                Speicifies the path of the output file (default: ./a.out or ./a.exe)
        -d, --tmpdir=DIR                Speicifies the directory for temporary files (default: /tmp/nodec)
            --make-args=ARGS             Passes extra arguments to make
            --vcbuild-args=ARGS          Passes extra arguments to vcbuild.bat
            --module-name=NAME           Compiles a npm module of the specified name
            --module-version=VERSION     Compiles a npm module of the speificied version
        -v, --version                    Prints the version of nodec and exit
            --node-version               Prints the version of the Node.js runtime and exit
        -h, --help                       Prints this help and exit

## Example 1: Compiling a CLI project

    git clone https://github.com/jashkenas/coffeescript.git
    cd coffeescript
    npm install
    nodec bin/coffee

## Example 2: Compiling a web application

    git clone https://github.com/cnodejs/nodeclub.git
    cd nodeclub
    npm install
    nodec app.js

## Example 3: Compiling a npm module

    nodec --module-name=coffee-script --module-version=1.11.1 coffee

## Development

After checking out the repo, run `bin/setup` to install dependencies. Then, run `rake` to run the tests. You can also run `bin/console` for an interactive prompt that will allow you to experiment.

To install this gem onto your local machine, run `bundle exec rake install`. Or without installing, run `bundle exec nodec` from the root of your project directory.

To release a new version, update the version number in `version.rb`, and then run `bundle exec rake release`, which will create a git tag for the version, push git commits and tags, and push the `.gem` file to [rubygems.org](https://rubygems.org).

## Contributing

Bug reports and pull requests are welcome on GitHub at https://github.com/enclose-io/node-compiler.

## Contributors

* [pmq20](https://github.com/pmq20) - **Minqi Pan** &lt;pmq2001@gmail.com&gt;
* [ibigbug](https://github.com/ibigbug) - **Yuwei Ba** &lt;akabyw@gmail.com&gt;

## License

Copyright (c) 2016 Node.js Compiler contributors, under terms of the [MIT License](http://opensource.org/licenses/MIT).
