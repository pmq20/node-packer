# Node.js Compiler

Compiling your Node.js application into a single executable. 

![nodec.gif](https://github.com/pmq20/node-compiler/raw/master/nodec.gif)

## How it works

## Presentation

[Node.js Compiler: compiling your Node.js application into a single executable](https://speakerdeck.com/pmq20/node-dot-js-compiler-compiling-your-node-dot-js-application-into-a-single-executable)

## Comparing with Similar Projects

| Project   | Difference                                                                                                                                                                                                                                                                                           |
|-----------|------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------|
| [pkg](https://github.com/zeit/pkg)       | pkg hacked `fs.*` standard API on the JavaScript level, whereas Node.js Compiler keeps them untouched while works on a deeper level via libsquash. pkg uses a big JSON to store in-package files while Node.js Compiler uses the more sophisticated and widely used SquashFS as its data structure. |
| [EncloseJS](http://enclosejs.com/) | EncloseJS only allows 5 fs functions to access in-package files, whereas Node.js Compiler supports all fs.* APIs. EncloseJS is proprietary licensed and charge your money when used while Node.js Compiler is MIT-licensed and you are both free to use it and free to modify it.                    |
| [Nexe](https://github.com/nexe/nexe)      | Nexe does not support dynamic `require` because of its use of browserify, whereas Node.js Compiler supports all kinds of require including require.resolve by using libsquash that hacks on a much deeper layer.                                                                                     |
| [asar](https://github.com/electron/asar) | asar uses JSON to store files' information while Node.js Compiler uses the more sophisticated and widely used SquashFS as its data structure. asar keeps the code archive and the executable seperate while Node.js Compiler links your JavaScript source code together with the Node.js virtual machine and generates a single executable as the final product. |
| [AppImage](http://appimage.org/)  | AppImage supports only Linux with a kernel that enabled support for SquashFS, while Node.js Compiler supports all three platforms of Linux, macOS and Windows, without any special feature requirements about the kernel.                                                                                                                                                                                       |

## Download

|                       |                                                       Master&#160;CI                                                                                                  |                                                                    RAM&#160;Test                                                                                          |                                                             Black&#x2011;box&#160;Test                                                         |                              Latest&#160;Stable                                        |
|:---------------------:|:---------------------------------------------------------------------------------------------------------------------------------------------------------------------:|:-------------------------------------------------------------------------------------------------------------------------------------------------------------------------:|:----------------------------------------------------------------------------------------------------------------------------------------------:|----------------------------------------------------------------------------------------|
|      **Windows**      |  [![status](https://ci.appveyor.com/api/projects/status/gap9xne0rayjtynp/branch/master?svg=true)](https://ci.appveyor.com/project/pmq20/node-compiler/branch/master)  |  [![status](https://ci.appveyor.com/api/projects/status/thpogkfsvij3r278/branch/master?svg=true)](https://ci.appveyor.com/project/pmq20/node-compiler-ram/branch/master)  |  [![status](https://ci.appveyor.com/api/projects/status/83a2wt22mfejiehe?svg=true)](https://ci.appveyor.com/project/pmq20/node-compiler-blbt)  | https://sourceforge.net/projects/node-compiler/files/v0.9.5/nodec.exe/download         |
|       **macOS**       |  [![status](https://travis-ci.org/pmq20/node-compiler.svg?branch=master)](https://travis-ci.org/pmq20/node-compiler)                                                  |  [![Status](https://travis-ci.org/pmq20/node-compiler-ram.svg?branch=master)](https://travis-ci.org/pmq20/node-compiler-ram)                                              |  [![Status](https://travis-ci.org/pmq20/node-compiler-blbt.svg?branch=master)](https://travis-ci.org/pmq20/node-compiler-blbt)                 | https://sourceforge.net/projects/node-compiler/files/v0.9.5/nodec-darwin-x64/download  |
|       **Linux**       |  [![status](https://travis-ci.org/pmq20/node-compiler.svg?branch=master)](https://travis-ci.org/pmq20/node-compiler)                                                  |  [![Status](https://travis-ci.org/pmq20/node-compiler-ram.svg?branch=master)](https://travis-ci.org/pmq20/node-compiler-ram)                                              |  [![Status](https://travis-ci.org/pmq20/node-compiler-blbt.svg?branch=master)](https://travis-ci.org/pmq20/node-compiler-blbt)                 | https://sourceforge.net/projects/node-compiler/files/v0.9.5/nodec-linux-x64/download   |

## Install

### Windows

First install the prerequisites:

* [SquashFS Tools 4.3](https://github.com/pmq20/squashfuse/files/691217/sqfs43-win32.zip)
* [Python 2.6 or 2.7](https://www.python.org/downloads/)
* [Visual Studio 2015 Update 3](https://www.visualstudio.com/), all editions
  including the Community edition (remember to select
  "Common Tools for Visual C++ 2015" feature during installation).

Then download the executable [nodec.exe](https://sourceforge.net/projects/node-compiler/files/v0.9.5/nodec.exe/download) and run it from the VC++ or VS Command Prompt.

### macOS

First install the prerequisites:

* [SquashFS Tools 4.3](http://squashfs.sourceforge.net/): `brew install squashfs`
* [Xcode](https://developer.apple.com/xcode/download/)
  * You also need to install the `Command Line Tools` via Xcode. You can find
    this under the menu `Xcode -> Preferences -> Downloads`
  * This step will install `gcc` and the related toolchain containing `make`
* Python 2.6 or 2.7
* GNU Make 3.81 or newer

Then,

    curl -L https://sourceforge.net/projects/node-compiler/files/v0.9.5/nodec-darwin-x64/download > nodec
    chmod +x nodec
    ./nodec

### Linux

First install the prerequisites:

* [SquashFS Tools 4.3](http://squashfs.sourceforge.net/)
* `gcc` and `g++` 4.8 or newer, or
* `clang` and `clang++` 3.4 or newer
* Python 2.6 or 2.7
* GNU Make 3.81 or newer

Then,

    curl -L https://sourceforge.net/projects/node-compiler/files/v0.9.5/nodec-linux-x64/download > nodec
    chmod +x nodec
    ./nodec

## Usage

    nodec [OPTION]... ENTRANCE
      -r, --root=DIR                   Specifies the path to the root of the application
      -o, --output=FILE                Specifies the path of the output file
      -d, --tmpdir=DIR                 Specifies the directory for temporary files
          --clean-tmpdir               Cleans all temporary files that were generated last time
          --keep-tmpdir                Keeps all temporary files that were generated last time
          --make-args=ARGS             Passes extra arguments to make
          --vcbuild-args=ARGS          Passes extra arguments to vcbuild.bat
      -n, --npm=FILE                   Specifies the path of npm
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

- [SquashFS](http://squashfs.sourceforge.net/): a compressed read-only filesystem for Linux.
- [Libsquash](https://github.com/pmq20/libsquash): portable, user-land SquashFS that can be easily linked and embedded within your application.
- [Enclose.IO](http://enclose.io/): cloud-based service that compiles your application into a single executable.
