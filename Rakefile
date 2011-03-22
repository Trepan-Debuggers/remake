#!/usr/bin/env rake
# -*- Ruby -*-

ROOT_DIR = File.dirname(__FILE__)
SPEC_FILES = FileList["#{ROOT_DIR}/tests/spec/test-*.rb"]

task :default => [:test]
desc 'Test everything - unit tests for now.'
task :test do
  sh "rspec #{SPEC_FILES.to_a.join(' ')}"
end
