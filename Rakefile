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
      version = args[:version] || '0.10.3'
      `cd #{deps_dir} && ((curl -L https://github.com/joyent/libuv/archive/v#{version}.tar.gz | tar zx) && (mv libuv libuv.old; mv libuv-#{version} libuv) || (rm -rf libuv-#{version}))`
    end

    desc 'Clean libuv'
    task :clean do
      `make -C #{libuv_dir} clean`
    end
  end
end
