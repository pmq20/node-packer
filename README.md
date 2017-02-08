# Node.js Compiler

Compiler for Node.js that compiles your Node.js application into a single executable.

[![Travis CI status](https://travis-ci.org/pmq20/node-compiler.svg?branch=master)](https://travis-ci.org/pmq20/node-compiler)
[![AppVeyor status](https://ci.appveyor.com/api/projects/status/gap9xne0rayjtynp/branch/master?svg=true)](https://ci.appveyor.com/project/pmq20/node-compiler/branch/master)

## Download

| Operating System | Architecture | Link                                                               |
|:----------------:|:------------:|--------------------------------------------------------------------|
|      Windows     |      x86     | http://www.enclose.io/pmq20/node-compiler/master/nodec.exe         |
|     Mac OS X     |     x86-64   | http://www.enclose.io/pmq20/node-compiler/master/nodec-darwin-x64  |
|       Linux      |     x86-64   | http://www.enclose.io/pmq20/node-compiler/master/nodec-linux-x64   |

## Usage

    nodec [...OPTIONS..] ENTRANCE
      -r, --root=DIR                   Speicifies the path to the root of the application
      -o, --output=FILE                Speicifies the path of the output file
      -d, --tmpdir=DIR                 Speicifies the directory for temporary files
          --make-args=ARGS             Passes extra arguments to make
          --vcbuild-args=ARGS          Passes extra arguments to vcbuild.bat
          --npm=FILE                   Speifices the path of npm
          --debug                      Enable debug mode
      -v, --version                    Prints the version of nodec and exit
          --node-version               Prints the version of the Node.js runtime and exit
      -h, --help                       Prints this help and exit

## Example

    git clone --depth 1 https://github.com/jashkenas/coffeescript.git
    cd coffeescript
    nodec bin/coffee
