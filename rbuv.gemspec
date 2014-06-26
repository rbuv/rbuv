# coding: utf-8
lib = File.expand_path('../lib', __FILE__)
$LOAD_PATH.unshift(lib) unless $LOAD_PATH.include?(lib)
require 'rbuv/version'

Gem::Specification.new do |spec|
  spec.name          = "rbuv"
  spec.version       = Rbuv::VERSION
  spec.authors       = ["Hanfei Shen"]
  spec.email         = ["qqshfox@gmail.com"]
  spec.description   = %q{libuv binding for Ruby}
  spec.summary       = %q{libuv binding for Ruby}
  spec.homepage      = "https://github.com/rbuv/rbuv"
  spec.license       = "MIT"

  spec.files         = `git ls-files`.split($/)
  spec.extensions    = ['ext/rbuv/extconf.rb']
  spec.executables   = spec.files.grep(%r{^bin/}) { |f| File.basename(f) }
  spec.test_files    = spec.files.grep(%r{^(test|spec|features)/})
  spec.require_paths = ["lib"]

  spec.add_development_dependency "bundler"
  spec.add_development_dependency "rake"
  spec.add_development_dependency "rake-compiler"
  spec.add_development_dependency "rspec", "~> 2.0"
  spec.add_development_dependency "coveralls"
end
