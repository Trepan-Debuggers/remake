#!/usr/bin/env ruby
dir = File.dirname(__FILE__)
system("rspec #{dir}/test-*.rb")
