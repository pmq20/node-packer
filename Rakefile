# frozen_string_literal: true

require 'rubygems'
require 'bundler/setup'
Bundler.require(:default)

require 'rake/testtask'
require 'rake/clean'

task default: [(Gem.win_platform? ? 'nodec.exe' : 'nodec')]
nodec_deps = FileList[File.expand_path('**/*', __dir__)] - [File.expand_path(((Gem.win_platform? ? 'nodec.exe' : 'nodec')), __dir__)]

desc "build #{(Gem.win_platform? ? 'nodec.exe' : 'nodec')}"
file((Gem.win_platform? ? 'nodec.exe' : 'nodec') => nodec_deps) do
  warn "Rebuilding #{(Gem.win_platform? ? 'nodec.exe' : 'nodec')}..."

  # don't include nodec in nodec
  rm_f(Gem.win_platform? ? 'nodec.exe' : 'nodec')

  sh_args = ['rubyc', 'bin/nodec', '-o', 'nodec']
  if ENV['ENCLOSE_IO_RUBYC_ADDTIONAL_ARGS'].present?
    ENV['ENCLOSE_IO_RUBYC_ADDTIONAL_ARGS'].split(/\s+/).each do |arg|
      sh_args << arg.strip
    end
  end
  warn "Will call sh with #{sh_args}"
  sh(*sh_args)
end

CLEAN << (Gem.win_platform? ? 'nodec.exe' : 'nodec')

namespace 'nodec' do
  nodec_original_node_env = {
    'ENCLOSE_IO_USE_ORIGINAL_NODE' => 'true'
  }
  desc 'run node -e from inside nodec'
  task :node, [:e] => (Gem.win_platform? ? 'nodec.exe' : 'nodec') do |_, args|
    sh nodec_original_node_env, (Gem.win_platform? ? '.\\nodec.exe' : './nodec'), '-e', args[:e]
  end
end
