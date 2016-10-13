# Enclose::IO::Compiler

Compiler of Enclose.IO which packs your Node.js app into a single executable.

http://enclose.io

[![Build Status](https://travis-ci.org/enclose-io/compiler.svg)](https://travis-ci.org/enclose-io/compiler)
[![Code Climate](https://codeclimate.com/github/enclose-io/compiler/badges/gpa.svg)](https://codeclimate.com/github/enclose-io/compiler)
[![codecov.io](https://codecov.io/github/enclose-io/compiler/coverage.svg?branch=master)](https://codecov.io/github/enclose-io/compiler?branch=master)

## Usage

    enclose-io-compiler [node_version] [module_name]

## Optional Environment Variables

* `ENCLOSE_IO_CONFIGURE_ARGS`
* `ENCLOSE_IO_MAKE_ARGS`

## Examples

    enclose-io-compiler node-v6.7.0 coffee-script
    ENCLOSE_IO_MAKE_ARGS=-j6 enclose-io-compiler node-v6.7.0 coffee-script
