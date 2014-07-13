require "bundler/gem_tasks"
require 'rake/extensiontask'
require 'rspec/core/rake_task'
require 'yard'

task :clean => ['deps:clean']

RSpec::Core::RakeTask.new(:spec)
task :spec => [:compile]

desc 'Run tests'
task :default => [:spec]

Rake::ExtensionTask.new('rbuv') do |ext|
  ext.lib_dir = File.join('lib', 'rbuv')
  ext.source_pattern = "*.{c,h}"
end

YARD::Rake::YardocTask.new do |t|
  t.files   = ['lib/**/*.rb', 'ext/**/*.c']
end

namespace :deps do
  deps_dir = 'deps'

  desc "Update all the deps"
  task :update, :libuv_version do |t, args|
    task('deps:libuv:update').invoke(args[:libuv_version])
  end

  desc "Clean all the deps"
  task :clean => ['deps:libuv:clean']

  namespace :libuv do
    libuv_dir = File.join(deps_dir, 'libuv')

    desc "Update libuv"
    task :update, :version do |t, args|
      version = args[:version] || 'v0.10.27'
      `git submodule update --init --recursive "#{libuv_dir}" && cd "#{libuv_dir}" && git checkout -qf #{version}`
    end

    desc 'Clean libuv'
    task :clean do
      `cd "#{libuv_dir}" && git clean -fdx`
    end
  end
end
