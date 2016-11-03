# Enclose.IO Compiler

Compiler of Enclose.IO which packs your Node.js app into a single executable.

http://enclose.io

[![Travis CI Build Status](https://travis-ci.org/enclose-io/compiler.svg)](https://travis-ci.org/enclose-io/compiler)
[![AppVeyor Build status](https://ci.appveyor.com/api/projects/status/h0j58lnfg32srtcu?svg=true)](https://ci.appveyor.com/project/pmq20/compiler)
[![Code Climate](https://codeclimate.com/github/enclose-io/compiler/badges/gpa.svg)](https://codeclimate.com/github/enclose-io/compiler)
[![codecov.io](https://codecov.io/github/enclose-io/compiler/coverage.svg?branch=master)](https://codecov.io/github/enclose-io/compiler?branch=master)
[![Gem Version](https://badge.fury.io/rb/enclose-io-compiler.svg)](https://badge.fury.io/rb/enclose-io-compiler)

## Installation

Add this line to your application's Gemfile:

```ruby
gem 'enclose-io-compiler'
```

And then execute:

    $ bundle

Or install it yourself as:

    $ gem install enclose-io-compiler


## Usage

    enclose-io-compiler [node_version] [module_name] [module_version] [bin_name] [output_path]

## Example

    enclose-io-compiler node-v6.8.0 coffee-script 1.11.1 coffee /tmp/coffee-1.11.1-node-v6.8.0-darwin-x64

Then the compiled product will be located at `/tmp/coffee-1.11.1-node-v6.8.0-darwin-x64`, ready to be distributed.

## Optional Environment Variables

* `ENCLOSE_IO_CONFIGURE_ARGS`
* `ENCLOSE_IO_MAKE_ARGS`
* `ENCLOSE_IO_KEEP_WORK_DIR`
* `npm_config_registry`

## Development

After checking out the repo, run `bin/setup` to install dependencies. Then, run `rake` to run the tests. You can also run `bin/console` for an interactive prompt that will allow you to experiment.

To install this gem onto your local machine, run `bundle exec rake install`. Or without installing, run `bundle exec enclose-io-compiler` from the root of your project directory.

To release a new version, update the version number in `version.rb`, and then run `bundle exec rake release`, which will create a git tag for the version, push git commits and tags, and push the `.gem` file to [rubygems.org](https://rubygems.org).

## Contributing

Bug reports and pull requests are welcome on GitHub at https://github.com/enclose-io/compiler.

## License

The gem is available as open source under the terms of the [MIT License](http://opensource.org/licenses/MIT).
