# Node.js Compiler

Compiling your Node.js application into a single executable.

http://enclose.io

[![Travis CI status](https://travis-ci.org/pmq20/node-compiler.svg?branch=master)](https://travis-ci.org/pmq20/node-compiler)
[![AppVeyor status](https://ci.appveyor.com/api/projects/status/gap9xne0rayjtynp/branch/master?svg=true)](https://ci.appveyor.com/project/pmq20/node-compiler/branch/master)

## Latest Release: v0.9.3

| Operating System |  Arch.  | Download Link                                                                     |
|:----------------:|:-------:|-----------------------------------------------------------------------------------|
|      Windows     |   x86   | https://github.com/pmq20/node-compiler/releases/download/v0.9.3/nodec.exe         |
|       macOS      |  x86-64 | https://github.com/pmq20/node-compiler/releases/download/v0.9.3/nodec-darwin-x64  |
|       Linux      |  x86-64 | https://github.com/pmq20/node-compiler/releases/download/v0.9.3/nodec-linux-x64   |

## Install

- Windows: download the `nodec.exe` and run it from the Visual Studio Command Prompt
- macOS: `curl https://github.com/pmq20/node-compiler/releases/download/v0.9.3/nodec-darwin-x64 > nodec && chmod +x nodec`
- Linux: `curl https://github.com/pmq20/node-compiler/releases/download/v0.9.3/nodec-linux-x64 > nodec && chmod +x nodec`

## Usage

    nodec [OPTION]... ENTRANCE
      -r, --root=DIR                   Specifies the path to the root of the application
      -o, --output=FILE                Specifies the path of the output file
      -d, --tmpdir=DIR                 Specifies the directory for temporary files
          --make-args=ARGS             Passes extra arguments to make
          --vcbuild-args=ARGS          Passes extra arguments to vcbuild.bat
          --npm=FILE                   Specifies the path of npm
          --npm-package=NAME           Downloads and compiles the specified npm package
          --npm-package-version=VER    Downloads and compiles the specified version of the npm package
          --debug                      Enable debug mode
      -v, --version                    Prints the version of nodec and exit
          --node-version               Prints the version of the Node.js runtime and exit
      -h, --help                       Prints this help and exit

## Example

    git clone --depth 1 https://github.com/jashkenas/coffeescript.git
    cd coffeescript
    nodec bin/coffee
    ./a.out (or a.exe on Windows)

## See Also

- [Libsquash](https://github.com/pmq20/libsquash): portable, user-land SquashFS that can be easily linked and embedded within your application.
- [SquashFS](http://squashfs.sourceforge.net/): a compressed read-only filesystem for Linux.
