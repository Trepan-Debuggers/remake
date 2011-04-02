#!/usr/bin/env rake
# -*- Ruby -*-

ROOT_DIR = File.dirname(__FILE__)
SPEC_FILES = FileList["#{ROOT_DIR}/tests/spec/test-*.rb"]

task :default => [:test]
task :check => [:test]

desc 'Test everything - the default.'
task :test => [:'test:basic', :'test:dbg']

desc "The normal GNU make test suite"
task :'test:basic' do
  sh "make check"
end

desc "Install via make"
task :'install' do
  sh "make install"
end

desc "Debugger-specific tests"
task :'test:dbg' do
  sh "rspec #{SPEC_FILES.to_a.join(' ')}"
end
