# Enclose::IO::Compiler

Compiler of Enclose.IO which packs your Node.js app into a single executable.

http://enclose.io

[![Build Status](https://travis-ci.org/enclose-io/compiler.svg)](https://travis-ci.org/enclose-io/compiler)
[![Code Climate](https://codeclimate.com/github/enclose-io/compiler/badges/gpa.svg)](https://codeclimate.com/github/enclose-io/compiler)
[![codecov.io](https://codecov.io/github/enclose-io/compiler/coverage.svg?branch=master)](https://codecov.io/github/enclose-io/compiler?branch=master)

## Usage

    enclose-io-compiler [node_version] [module_name] [module_version] [bin_name]

## Optional Environment Variables

* `ENCLOSE_IO_CONFIGURE_ARGS`
* `ENCLOSE_IO_MAKE_ARGS`

## Examples

    enclose-io-compiler node-v6.8.0 coffee-script 1.11.1 coffee

Then the compiled product will be located at `./vendor/node-v6.8.0/node`, ready to be used.

![enclose-io-compiler node-v6.8.0 coffee-script 1.11.1 coffee](https://github.com/enclose-io/compiler/blob/master/README.png?raw=true)
