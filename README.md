# Node.js Compiler

the Node.js compiler of Enclose.IO which compiles your app into a single executable.

http://enclose.io

[![Linux and Mac OS X Build Status](https://travis-ci.org/enclose-io/node-compiler.svg?branch=master)](https://travis-ci.org/enclose-io/node-compiler)
[![Windows Build status](https://ci.appveyor.com/api/projects/status/f4x3bq5hub3uu3ys/branch/master?svg=true)](https://ci.appveyor.com/project/pmq20/node-compiler/branch/master)
[![Code Climate](https://codeclimate.com/github/enclose-io/node-compiler/badges/gpa.svg)](https://codeclimate.com/github/enclose-io/node-compiler)
[![codecov.io](https://codecov.io/github/enclose-io/node-compiler/coverage.svg?branch=master)](https://codecov.io/github/enclose-io/node-compiler?branch=master)
[![Gem Version](https://badge.fury.io/rb/node-compiler.svg)](https://badge.fury.io/rb/node-compiler)

## Supported Node.js Versions

| Release Line | Supported Version | Based on                                          |
|:------------:|:-----------------:|---------------------------------------------------|
|      LTS     |   `node-v6.9.1`   | https://nodejs.org/dist/v6.9.1/node-v6.9.1.tar.gz |
|    Current   |   `node-v7.2.0`   | https://nodejs.org/dist/v7.2.0/node-v7.2.0.tar.gz |

## Installation

Add this line to your application's Gemfile:

```ruby
gem 'node-compiler'
```

And then execute:

    $ bundle

Or install it yourself as:

    $ gem install node-compiler


## Usage

    nodec [node_version] [module_name] [module_version] [bin_name] [output_path]

## Example

    nodec node-v7.2.0 coffee-script 1.11.1 coffee coffee-1.11.1-node-v7.2.0-darwin-x64

Then the compiled product will be located at `./coffee-1.11.1-node-v7.2.0-darwin-x64`, ready to be distributed.

## Optional Environment Variables

* `ENCLOSE_IO_KEEP_WORK_DIR`
* `ENCLOSE_IO_CONFIGURE_ARGS`
* `ENCLOSE_IO_MAKE_ARGS`
* `ENCLOSE_IO_VCBUILD_ARGS`
* `ENCLOSE_IO_NPM`
* `ENCLOSE_IO_NPM_INSTALL_ARGS`
* `npm_config_registry`

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

The gem is available as open source under the terms of the [MIT License](http://opensource.org/licenses/MIT).
