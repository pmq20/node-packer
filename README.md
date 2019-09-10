# Node.js Packer

*Packing your Node.js application into a single executable.*

[![Build status](https://ci.appveyor.com/api/projects/status/fsuhe9x6mamohuym/branch/Series12?svg=true)](https://ci.appveyor.com/project/slee047/node-packer/branch/Series12)
[![Status](https://travis-ci.org/slee047/node-packer.svg?branch=Series10)](https://travis-ci.org/slee047/node-packer)
[![GitHub version](https://img.shields.io/badge/version-1.6.0%20(12.10.0)-blue.svg)](https://badge.fury.io/gh/slee047%2Fnode-packer)

![Terminal simulation of a simple compilation](https://github.com/pmq20/node-compiler/raw/master/doc/nodec.gif)

## Features

- Works on Linux, Mac and Windows
- Creates a binary distribution of your application
- Supports natively any form of `require`, including dynamic ones (e.g. `require(myPath + 'module.js'`)
- Supports any module, including direct download and compilation from npm
- Native C++ modules are fully supported
- Features zero-config auto-update capabilities to make your compiled project to stay updated
- Open Source, MIT Licensed

## Get Started

It takes less than 5 minutes to compile any project with `node-compiler`.

You won't need to modify a single line of code in your application, no matter how you developed it as long as it works in plain node.js!

|    Operating System   | Architecture |           Latest&#160;Stable                 |
|:---------------------:|:------------:|----------------------------------------------|
|        Windows        |    x86-64    | http://enclose.io/nodec/nodec-x64.zip        |
|         macOS         |    x86-64    | http://enclose.io/nodec/nodec-darwin-x64.gz  |
|         Linux         |    x86-64    | http://enclose.io/nodec/nodec-linux-x64.gz   |

For previous releases, cf. http://enclose.io/nodec

### Install on Windows

First install the prerequisites:

* [SquashFS Tools 4.3](https://github.com/pmq20/squashfuse/files/691217/sqfs43-win32.zip)
* [Python 2.6 or 2.7](https://www.python.org/downloads/)
* Either of,
  - [Visual Studio 2015 Update 3](https://www.visualstudio.com/), all editions
  including the Community edition (remember to select
  "Common Tools for Visual C++ 2015" feature during installation).
  - [Visual Studio 2017](https://www.visualstudio.com/downloads/), any edition (including the Build Tools SKU).
  __Required Components:__ "MSbuild", "VC++ 2017 v141 toolset" and one of the Windows SDKs (10 or 8.1).

Then download [nodec-x64.zip](http://enclose.io/nodec/nodec-x64.zip), and this zip file contains only one executable. Unzip it. Optionally, rename it to `nodec.exe` and put it under `C:\Windows` (or any other directory that is part of `PATH`). Execute `nodec` from the command line.

### Install on Linux

First install the prerequisites:

* [SquashFS Tools 4.3](http://squashfs.sourceforge.net/)
  - `sudo yum install squashfs-tools`
  - `sudo apt-get install squashfs-tools`
* `gcc` and `g++` 4.9.4 or newer, or
* `clang` and `clang++` 3.4.2 or newer
* Python 2.6 or 2.7
* GNU Make 3.81 or newer

Then,

    curl -L http://enclose.io/nodec/nodec-linux-x64.gz | gunzip > nodec
    chmod +x nodec
    ./nodec

#### Additional Notes on the compatibility between RHEL based (CentOS) / Ubuntu
It is known that the default repo for Red Hat and CentOS distros contains a very outdated gcc / g++ (3.8.5) while the latest Long Term Support (LTS) of Ubuntu as of 15 Feb 2018 (Ubuntu 18.04 LTS) contains a relatively updated gcc / g++ (7.3.0).

It is known that compilation can fail when using unsupported configuration where the version of prerequisites is older than prescribed.

Therefore, it is crucial for the users of Red Hat based distros to install gcc / g++ outside from official repos.
For starters, one may look at: 
- https://developers.redhat.com/blog/2018/07/07/yum-install-gcc7-clang/
- http://blog.stevedoria.net/20180214/how-to-install-gcc-7-on-centos-7

Additionally, binaries that are compiled from Ubuntu 18.04 LTS is known NOT to work in Red Hat 7 based distro (Including CentOS) due to 'glibcxx_3.4.20' not found' related error. However, binaries that are compiled from either Red Hat or CentOS 7 are known to work with Ubuntu 18.04 LTS based on my internal experiment.

Having said that, I will still recommend that binaries distributors should compile 2 versions for Linux where one caters for RHEL based and the other for Ubuntu based.


### Install on macOS

First install the prerequisites:

* [SquashFS Tools 4.3](http://squashfs.sourceforge.net/): `brew install squashfs`
* [Xcode](https://developer.apple.com/xcode/download/)
  * You also need to install the `Command Line Tools` via Xcode. You can find
    this under the menu `Xcode -> Preferences -> Downloads`
  * This step will install `gcc` and the related toolchain containing `make`
* Python 2.6 or 2.7
* GNU Make 3.81 or newer

Then,

    curl -L http://enclose.io/nodec/nodec-darwin-x64.gz | gunzip > nodec
    chmod +x nodec
    ./nodec

## Usage

    nodec [OPTION]... [ENTRANCE]
      -r, --root=DIR                   Specifies the path to the root of the application
      -o, --output=FILE                Specifies the path of the output file
      -d, --tmpdir=DIR                 Specifies the directory for temporary files
          --clean-tmpdir               Cleans all temporary files that were generated last time
          --keep-tmpdir                Keeps all temporary files that were generated last time
          --make-args=ARGS             Passes extra arguments to make
          --vcbuild-args=ARGS          Passes extra arguments to vcbuild.bat
      -n, --npm=FILE                   Specifies the path of npm
          --skip-npm-install           Skips the npm install process
          --npm-package=NAME           Downloads and compiles the specified npm package
          --npm-package-version=VER    Downloads and compiles the specified version of the npm package
          --auto-update-url=URL        Enables auto-update and specifies the URL to get the latest version
          --auto-update-base=STRING    Enables auto-update and specifies the base version string
          --msi                        Generates a .msi installer for Windows
          --pkg                        Generates a .pkg installer for macOS
          --debug                      Enable debug mode
          --quiet                      Enable quiet mode
      -v, --version                    Prints the version of nodec and exit
      -V, --node-version               Prints the version of the Node.js runtime and exit
      -h, --help                       Prints this help and exit

Note: if `ENTRANCE` was not provided, then a single Node.js interpreter executable will be produced.

Note: To compile to 32-bit windows OS compatible programs on a 64-bit machine, you should use a x64 x32 cross compiling environment. You should be able to find it in your Start Menu after installing Visual Studio. Also, you have to use a 32-bit Node.js, because the arch information is detected via `node -pe process.arch`.

## Examples

### Compile a CLI tool

    git clone --depth 1 https://github.com/jashkenas/coffeescript.git
    cd coffeescript
    nodec bin/coffee
    ./a.out (or a.exe on Windows)

### Compile a web application

    git clone --depth 1 https://github.com/eggjs/examples.git
    cd examples/helloworld
    npm install
    nodec node_modules/egg-bin/bin/egg-bin.js
    ./a.out dev (or a.exe dev on Windows)

### Compile a npm package

    nodec --npm-package=coffee-script coffee
    ./a.out (or a.exe on Windows)

## Learn More

### How it works

- [Node.js Compiler: compiling your Node.js application into a single executable](https://speakerdeck.com/pmq20/node-dot-js-compiler-compiling-your-node-dot-js-application-into-a-single-executable).

### Comparing with Similar Projects

|            Project                       | Differences                                                                                                                                                                                                                                                                                                                                             |
|:----------------------------------------:|---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------|
| [pkg](https://github.com/zeit/pkg)       | Pkg hacked `fs.*` API's dynamically in order to access in-package files, whereas Node.js Compiler leaves them alone and instead works on a deeper level via [libsquash](https://github.com/pmq20/libsquash). Pkg uses JSON to store in-package files while Node.js Compiler uses the more sophisticated and widely used SquashFS as its data structure. |
| [EncloseJS](http://enclosejs.com/)       | EncloseJS restricts access to in-package files to only five `fs.*` API's, whereas Node.js Compiler supports all `fs.*` API's. EncloseJS is proprietary licensed and charges money when used while Node.js Compiler is MIT-licensed and users are both free to use it and free to modify it.                                                             |
| [Nexe](https://github.com/nexe/nexe)     | Nexe does not support dynamic `require` because of its use of `browserify`, whereas Node.js Compiler supports all kinds of `require` including `require.resolve`.                                                                                                                                                                                       |
| [asar](https://github.com/electron/asar) | Asar keeps the code archive and the executable separate while Node.js Compiler links all JavaScript source code together with the Node.js virtual machine and generates a single executable as the final product. Asar uses JSON to store files' information while Node.js Compiler uses SquashFS.                                                      |
| [AppImage](http://appimage.org/)         | AppImage supports only Linux with a kernel that supports SquashFS, while Node.js Compiler supports all three platforms of Linux, macOS and Windows, meanwhile without any special feature requirements from the kernel.                                                                                                                                 |

## See Also

- [Libsquash](https://github.com/pmq20/libsquash): portable, user-land SquashFS that can be easily linked and embedded within your application.
- [Libautoupdate](https://github.com/pmq20/libautoupdate): cross-platform C library to enable your application to auto-update itself in place.
