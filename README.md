# Node.js Compiler

Compiling your Node.js application into a single executable. 

![nodec.gif](https://github.com/pmq20/node-compiler/raw/master/nodec.gif)

## Download

|                       |                                                       Master&#160;CI                                                                                                  |                                                              [RAM&#160;Test](https://github.com/pmq20/node-compiler-ram)                                                  |                                                             [Black&#x2011;box&#160;Test](https://github.com/pmq20/node-compiler-blbt)          |                              Latest&#160;Stable                                        |
|:---------------------:|:---------------------------------------------------------------------------------------------------------------------------------------------------------------------:|:-------------------------------------------------------------------------------------------------------------------------------------------------------------------------:|:----------------------------------------------------------------------------------------------------------------------------------------------:|----------------------------------------------------------------------------------------|
|      **Windows**      |  [![status](https://ci.appveyor.com/api/projects/status/gap9xne0rayjtynp/branch/master?svg=true)](https://ci.appveyor.com/project/pmq20/node-compiler/branch/master)  |  [![status](https://ci.appveyor.com/api/projects/status/thpogkfsvij3r278/branch/master?svg=true)](https://ci.appveyor.com/project/pmq20/node-compiler-ram/branch/master)  |  [![status](https://ci.appveyor.com/api/projects/status/83a2wt22mfejiehe?svg=true)](https://ci.appveyor.com/project/pmq20/node-compiler-blbt)  | https://sourceforge.net/projects/node-compiler/files/v0.9.6/nodec.exe/download         |
|       **macOS**       |  [![status](https://travis-ci.org/pmq20/node-compiler.svg?branch=master)](https://travis-ci.org/pmq20/node-compiler)                                                  |  [![Status](https://travis-ci.org/pmq20/node-compiler-ram.svg?branch=master)](https://travis-ci.org/pmq20/node-compiler-ram)                                              |  [![Status](https://travis-ci.org/pmq20/node-compiler-blbt.svg?branch=master)](https://travis-ci.org/pmq20/node-compiler-blbt)                 | https://sourceforge.net/projects/node-compiler/files/v0.9.6/nodec-darwin-x64/download  |
|       **Linux**       |  [![status](https://travis-ci.org/pmq20/node-compiler.svg?branch=master)](https://travis-ci.org/pmq20/node-compiler)                                                  |  [![Status](https://travis-ci.org/pmq20/node-compiler-ram.svg?branch=master)](https://travis-ci.org/pmq20/node-compiler-ram)                                              |  [![Status](https://travis-ci.org/pmq20/node-compiler-blbt.svg?branch=master)](https://travis-ci.org/pmq20/node-compiler-blbt)                 | https://sourceforge.net/projects/node-compiler/files/v0.9.6/nodec-linux-x64/download   |

## How it works?

### Presentation

[Node.js Compiler: compiling your Node.js application into a single executable](https://speakerdeck.com/pmq20/node-dot-js-compiler-compiling-your-node-dot-js-application-into-a-single-executable).

### Comparing with Similar Projects

| Project   | Differences                                                                                                                                                                                                                                                                                           |
|-----------|------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------|
| [pkg](https://github.com/zeit/pkg)       | Pkg hacked `fs.*` API's dynamically in order to access in-package files, whereas Node.js Compiler leaves them alone and instead works on a deeper level via [libsquash](https://github.com/pmq20/libsquash). Pkg uses JSON to store in-package files while Node.js Compiler uses the more sophisticated and widely used SquashFS as its data structure. |
| [EncloseJS](http://enclosejs.com/) | EncloseJS restricts access to in-package files to only five `fs.*` API's, whereas Node.js Compiler supports all `fs.*` API's. EncloseJS is proprietary licensed and charges money when used while Node.js Compiler is MIT-licensed and users are both free to use it and free to modify it.                    |
| [Nexe](https://github.com/nexe/nexe)      | Nexe does not support dynamic `require` because of its use of `browserify`, whereas Node.js Compiler supports all kinds of `require` including `require.resolve`.                                                                                     |
| [asar](https://github.com/electron/asar) | Asar keeps the code archive and the executable separate while Node.js Compiler links all JavaScript source code together with the Node.js virtual machine and generates a single executable as the final product. Asar uses JSON to store files' information while Node.js Compiler uses SquashFS. |
| [AppImage](http://appimage.org/)  | AppImage supports only Linux with a kernel that supports SquashFS, while Node.js Compiler supports all three platforms of Linux, macOS and Windows, meanwhile without any special feature requirements from the kernel.                                                                                                                                                                                       |

## Install

### Windows

First install the prerequisites:

* [SquashFS Tools 4.3](https://github.com/pmq20/squashfuse/files/691217/sqfs43-win32.zip)
* [Python 2.6 or 2.7](https://www.python.org/downloads/)
* [Visual Studio 2015 Update 3](https://www.visualstudio.com/), all editions
  including the Community edition (remember to select
  "Common Tools for Visual C++ 2015" feature during installation).

Then download the executable [nodec.exe](https://sourceforge.net/projects/node-compiler/files/v0.9.6/nodec.exe/download) and run it from the VC++ or VS Command Prompt.

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

    curl -L https://sourceforge.net/projects/node-compiler/files/v0.9.6/nodec-darwin-x64/download > nodec
    chmod +x nodec
    ./nodec

### Linux

First install the prerequisites:

* [SquashFS Tools 4.3](http://squashfs.sourceforge.net/)
* `gcc` and `g++` 4.8.5 or newer, or
* `clang` and `clang++` 3.4 or newer
* Python 2.6 or 2.7
* GNU Make 3.81 or newer

Then,

    curl -L https://sourceforge.net/projects/node-compiler/files/v0.9.6/nodec-linux-x64/download > nodec
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
      -o, --dest-os=OS                 Destination operating system (enum: win mac solaris freebsd openbsd linux android aix)
      -a, --dest-arch=ARCH             Destination CPU architecture (enum: arm arm64 ia32 mips mipsel ppc ppc64 x32 x64 x86 s390 s390x)
      -v, --version                    Prints the version of nodec and exit
          --node-version               Prints the version of the Node.js runtime and exit
      -h, --help                       Prints this help and exit

## Example

    git clone --depth 1 https://github.com/jashkenas/coffeescript.git
    cd coffeescript
    nodec bin/coffee
    ./a.out (or a.exe on Windows)

## Cross Compilation

`nodec` also support cross-compilation. Since node.js is built from sources you will need to setup properly a toolchain in order
to get valid compilers to produce binaries for the destination platform.

You can easily do this with by using [crosstool-ng](https://github.com/crosstool-ng/crosstool-ng) or any other tool you like.

Once you're done with the build of a valid toolchain (don't forget to enable c++ if you use crosstool-ng which by default excludes it)
you will be able to compile properly. Just set-up your environment so that it will know to use your cross-compile toolchain rather than
your system's default build tools.

An example (you may need to adjust values or specify additional variables):


    export AR="x86_64-unknown-linux-gnu-ar"
    export CC="x86_64-unknown-linux-gnu-gcc"
    export CXX="x86_64-unknown-linux-gnu-g++"
    export LINK="x86_64-unknown-linux-gnu-g++"
    export CPP="x86_64-unknown-linux-gnu-gcc -E"
    export LD="x86_64-unknown-linux-gnu-ld"
    export AS="x86_64-unknown-linux-gnu-as"
    export CCLD="ax86_64-unknown-linux-gnu-gcc ${TARGET_ARCH}"
    export NM="x86_64-unknown-linux-gnu-nm"
    export STRIP="x86_64-unknown-linux-gnu-strip"
    export OBJCOPY="x86_64-unknown-linux-gnu-objcopy"
    export RANLIB="x86_64-unknown-linux-gnu-ranlib"
    export F77="x86_64-unknown-linux-gnu-g77 ${TARGET_ARCH}"
    unset LIBC

    #Define flags
    #export CXXFLAGS="-march=armv7-a"
    export LDFLAGS="-L${CSTOOLS_LIB} -Wl,-rpath-link,${CSTOOLS_LIB} -Wl,-O1 -Wl,--hash-style=gnu"
    export CFLAGS="-isystem${CSTOOLS_INC} -fexpensive-optimizations -frename-registers -fomit-frame-pointer -O2 -ggdb3"
    export CPPFLAGS="-isystem${CSTOOLS_INC}"
    # export CCFLAGS="-march=armv7-a"

    #Tools
    export CSTOOLS=/Volumes/crosstools/x86_64-unknown-linux-gnu
    export CSTOOLS_INC=${CSTOOLS}/include
    export CSTOOLS_LIB=${CSTOOLS}/lib
    #export ARM_TARGET_LIB=$CSTOOLS_LIB
    # export GYP_DEFINES="armv7=1"

    #Define other things, those are not 'must' to have defined but we added
    export SHELL="/bin/bash"
    export TERM="screen"
    export LANG="en_US.UTF-8"
    export MAKE="make"

    #Export the path for your system
    #export HOME="/home/gioyik" #Change this one with the name of your user directory
    export PATH=${CSTOOLS}/bin:/usr/arm-linux-gnueabi/bin/:$PATH


## See Also

- [SquashFS](http://squashfs.sourceforge.net/): a compressed read-only filesystem for Linux.
- [Libsquash](https://github.com/pmq20/libsquash): portable, user-land SquashFS that can be easily linked and embedded within your application.
- [Enclose.IO](http://enclose.io/): cloud-based service that compiles your application into a single executable.
