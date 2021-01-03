# Node.js Packer

*Packing your Node.js application into a single executable.*

[![Windows](https://github.com/pmq20/node-packer/workflows/Windows/badge.svg)](https://github.com/pmq20/node-packer/actions?query=workflow%3A"Windows")
[![macOS](https://github.com/pmq20/node-packer/workflows/macOS/badge.svg)](https://github.com/pmq20/node-packer/actions?query=workflow%3A"macOS")
[![Linux](https://github.com/pmq20/node-packer/workflows/Linux/badge.svg)](https://github.com/pmq20/node-packer/actions?query=workflow%3A"Linux")

## Features

It takes less than 5 minutes to compile any project with `node-packer`.

You won't need to modify a single line of code in your application, no matter how you developed it as long as it works in plain node.js!

- Works on ![win](resource/win_sm.png) Windows, ![macOS](resource/apple_sm.png) macOS and ![linux](resource/linux_sm.png) Linux
- Creates a binary distribution of your application
- Supports natively any form of `require`, including dynamic ones (e.g. `require(myPath + 'module.js'`)
- Native C++ modules are fully supported
- Open Source, MIT Licensed

## Download

### Stable Releases

Here is the latest stable Node.js Packer release:

|    OS     | Arch. |                                              Executable                                                        |
|:---------:|:-----:|----------------------------------------------------------------------------------------------------------------|
|  Windows  |  x64  | https://gw.alipayobjects.com/os/enclose-prod/0d0ec8fd-dc9c-4b0a-85df-8bf4af0e8b8d/nodec-v1.5.0-x64.zip         |
|   macOS   |  x64  | https://gw.alipayobjects.com/os/enclose-prod/bc2022ef-4b88-4c12-9980-394945c9c198/nodec-v1.5.0-darwin-x64.gz   |
|   Linux   |  x64  | https://gw.alipayobjects.com/os/enclose-prod/b6aa41a6-f6b5-4542-b777-06e4bc292c5e/nodec-v1.5.0-linux-x64.gz    |

### Unstable Pre-release

Whenever the `master` branch CI succeeded, a Node.js Packer pre-release binary would be automatically generated. Here is the latest unstable pre-release build:

|    OS     | Arch. |                                              Executable                                                        |
|:---------:|:-----:|----------------------------------------------------------------------------------------------------------------|
|  Windows  |  x64  | https://github.com/pmq20/node-packer/releases/download/windows-x64/pre-release-nodec-v140800.121803-x64.exe    |
|   macOS   |  x64  | https://github.com/pmq20/node-packer/releases/download/darwin-x64/pre-release-nodec-v140800.121803-darwin-x64  |
|   Linux   |  x64  | https://github.com/pmq20/node-packer/releases/download/linux-x64/pre-release-nodec-v140800.121803-linux-x64    |

## Install

### ![win](resource/win_med.png) Install on Windows

First install the prerequisites:

* [SquashFS Tools 4.3](https://github.com/pmq20/squashfuse/files/691217/sqfs43-win32.zip)
* [Python 2.6 or 2.7](https://www.python.org/downloads/)
* Either of,
  - [Visual Studio 2015 Update 3](https://www.visualstudio.com/), all editions
  including the Community edition (remember to select
  "Common Tools for Visual C++ 2015" feature during installation).
  - [Visual Studio 2017](https://www.visualstudio.com/downloads/), any edition (including the Build Tools SKU).
  __Required Components:__ "MSbuild", "VC++ 2017 v141 toolset" and one of the Windows SDKs (10 or 8.1).

Then download `nodec-x64.exe`.

Optionally, put it under `C:\Windows` or any other `PATH` directories. Open Visual Studio's "x64 Native Tools Command Prompt" and execute `nodec --help` therein.

### ![macOS](resource/apple_med.png) Install on macOS

First install the prerequisites:

* [SquashFS Tools 4.3](http://squashfs.sourceforge.net/): `brew install squashfs`
* [Xcode](https://developer.apple.com/xcode/download/)
  * You also need to install the `Command Line Tools` via Xcode. You can find
    this under the menu `Xcode -> Preferences -> Downloads`
  * This step will install `gcc` and the related toolchain containing `make`
* Python 2.6 or 2.7
* GNU Make 3.81 or newer

Then download `nodec-darwin-x64`.

Run `chmod +x` to give it execution permissions and execute `./nodec --help`.

#### Additional Notes on Build failure in macOS XCode 11

According to recent Travis Build, the test cases will fail shortly after its launch occurs when the Build Environement is XCode 11. Currently, it is not known whether the issue is caused by XCode 11 or other factors within Travis CI that may not impact on actual macOS deployment.

Therefore, the build environment for macOS under Travis is XCode 10.2 so as to ensure the test case can be successfully executed and completed.

### ![linux](resource/linux_med.png) Install on Linux

First install the prerequisites:

* [SquashFS Tools 4.3](http://squashfs.sourceforge.net/)
  - `sudo yum install squashfs-tools`
  - `sudo apt-get install squashfs-tools`
* `gcc` and `g++` 4.9.4 or newer, or
* `clang` and `clang++` 3.4.2 or newer
* Python 2.6 or 2.7
* GNU Make 3.81 or newer

Then download `nodec-linux-x64`.

Run `chmod +x` to give it execution permissions and execute `./nodec --help`.

#### Additional Notes on the compatibility between RHEL based (CentOS) / Ubuntu

It is known that the default repo for Red Hat and CentOS distros contains a very outdated gcc / g++ (3.8.5) while the latest Long Term Support (LTS) of Ubuntu as of 15 Feb 2018 (Ubuntu 18.04 LTS) contains a relatively updated gcc / g++ (7.3.0).

It is known that compilation can fail when using unsupported configuration where the version of prerequisites is older than prescribed.

Therefore, it is crucial for the users of Red Hat based distros to install gcc / g++ outside from official repos.
For starters, one may look at: 
- https://developers.redhat.com/blog/2018/07/07/yum-install-gcc7-clang/
- http://blog.stevedoria.net/20180214/how-to-install-gcc-7-on-centos-7

Additionally, binaries that are compiled from Ubuntu 18.04 LTS is known NOT to work in Red Hat 7 based distro (Including CentOS) due to 'glibcxx_3.4.20' not found' related error. However, binaries that are compiled from either Red Hat or CentOS 7 are known to work with Ubuntu 18.04 LTS based on my internal experiment.

Having said that, I will still recommend that binaries distributors should compile 2 versions for Linux where one caters for RHEL based and the other for Ubuntu based.

#### Additional Notes on Build failure in Linux

According to recent Travis Build, Linux has been failing to build since nodec-1.6.0-10.16.0 (Node.js 10.16.0).
The root cause is yet to be determined, and the last known good build is 10.15.3 which can be downloaded here:
https://github.com/slee047/node-packer/releases/tag/1.6.0-10.15.3-1

The issue can be found here:
https://github.com/slee047/node-packer/issues/11

NOTE: This gz file ([nodec-darwin-x64.gz](http://enclose.io/nodec/nodec-darwin-x64.gz)) contains an outdated version of nodec (nodec 1.5.0 with Node.js 8.3.0).
The original maintainer did not specify how to build this repo into single executable, therefore newer versions can only be run on source code directly.

## Usage

    nodec [OPTION]... [ENTRANCE]
          --current                    Uses the current Node.js release
          --lts                        Uses the LTS Node.js release
      -r, --root=DIR                   Specifies the path to the root of the application
          --output=FILE                Specifies the path of the output file
      -d, --tmpdir=DIR                 Specifies the directory for temporary files
          --clean-tmpdir               Cleans all temporary files that were generated last time
          --keep-tmpdir                Keeps all temporary files that were generated last time
          --make-args=ARGS             Passes extra arguments to make
          --vcbuild-args=ARGS          Passes extra arguments to vcbuild.bat
      -n, --npm=FILE                   Specifies the path of npm
          --skip-npm-install           Skips the npm install process
          --debug                      Enables debug mode
      -o, --dest-os=OS                 Specifies the destination operating system (enum: win mac solaris freebsd openbsd linux android aix)
      -a, --dest-arch=ARCH             Specifies the destination CPU architecture (enum: arm arm64 ia32 mips mipsel ppc ppc64 x32 x64 x86 s390 s390x)
          --quiet                      Enables quiet mode
      -v, --version                    Prints the version of nodec and exit
      -h, --help                       Prints this help and exit

Note: if ENTRANCE was not provided, a single raw Node.js interpreter executable would be produced.

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

## Learn More

### How it works

- [Node.js Packer: compiling your Node.js application into a single executable](https://speakerdeck.com/pmq20/node-dot-js-compiler-compiling-your-node-dot-js-application-into-a-single-executable).

### Comparing with Similar Projects

|            Project                       | Differences                                                                                                                                                                                                                                                                                                                                             |
|:----------------------------------------:|---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------|
| [pkg](https://github.com/zeit/pkg)       | Pkg hacked `fs.*` API's dynamically in order to access in-package files, whereas Node.js Packer leaves them alone and instead works on a deeper level via [libsquash](https://github.com/pmq20/libsquash). Pkg uses JSON to store in-package files while Node.js Packer uses the more sophisticated and widely used SquashFS as its data structure. |
| [EncloseJS](http://enclosejs.com/)       | EncloseJS restricts access to in-package files to only five `fs.*` API's, whereas Node.js Packer supports all `fs.*` API's. EncloseJS is proprietary licensed and charges money when used while Node.js Packer is MIT-licensed and users are both free to use it and free to modify it.                                                             |
| [Nexe](https://github.com/nexe/nexe)     | Nexe does not support dynamic `require` because of its use of `browserify`, whereas Node.js Packer supports all kinds of `require` including `require.resolve`.                                                                                                                                                                                       |
| [asar](https://github.com/electron/asar) | Asar keeps the code archive and the executable separate while Node.js Packer links all JavaScript source code together with the Node.js virtual machine and generates a single executable as the final product. Asar uses JSON to store files' information while Node.js Packer uses SquashFS.                                                      |
| [AppImage](http://appimage.org/)         | AppImage supports only Linux with a kernel that supports SquashFS, while Node.js Packer supports all three platforms of Linux, macOS and Windows, meanwhile without any special feature requirements from the kernel.                                                                                                                                 |

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

## To-do

- Eliminate dependending on an outside Node.js and npm when compiling
- Be able to use a custom icon and file description for the executable output. maybe an icon file in the package root directory.
  - https://github.com/pmq20/node-packer/issues/54
- Add options to select items to deliver
  - Ppt out zlib/openssl for system libraries
  - Incl and ICU
  - Debug facilities
- Add options to select compression-method
  - Optionally xz the final product
- Detect simultaneous runs of nodec
  - https://github.com/pmq20/node-packer/issues/31
- Warn the user that some symbolic link links to the outside of the project
  - Add a check procedure at compile time
  - https://github.com/pmq20/node-packer/issues/37
- Cross-compile
  - https://github.com/pmq20/node-packer/pull/36
  - Support ARM architecture
  - https://github.com/pmq20/node-packer/issues/26
- Make Docker images for compiler environments
- Generate Windows-less Cmd-less Windows applications via /subsystem=windows
- Drop the external dependency of mksquashfs
  - i.e. Give libsquash the ability to mksquashfs
- Support library only projects
  - https://github.com/pmq20/node-packer/issues/39

## Authors

[Minqi Pan et al.](https://raw.githubusercontent.com/pmq20/node-packer/master/AUTHORS)

## License

[MIT](https://raw.githubusercontent.com/pmq20/node-packer/master/LICENSE)

## See Also

- [Libsquash](https://github.com/pmq20/libsquash): portable, user-land SquashFS that can be easily linked and embedded within your application.
- [Squashfs Tools](https://github.com/plougher/squashfs-tools): tools to create and extract Squashfs filesystems.
