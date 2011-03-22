#!/usr/bin/env ruby
require 'rubygems'
require 'rspec'
require_relative 'helper'

describe "A simple trace" do
  def run_remake(test_name, opts, makefile_short=nil)
    makefile_short = test_name unless makefile_short
    makefile = File.join(EXAMPLE_DIR, makefile_short + '.Makefile')
    rightfile = File.join(DATA_DIR, test_name + '.right')
    expected_output = File.open(rightfile, 'r').readlines.map{|l| l.chomp}
    output = `#{MAKE_PROG} #{opts} #{makefile}`.split("\n")[6..-1]
    output.each do |line|
      line.gsub!(/^.*(#{makefile_short}\.Makefile:\d+)/,'\1')
    end
    output.should == expected_output
  end

  it 'should be able to do handle --trace (no options)' do
    ['--trace --file', '-x -f'].each do |opts|
      run_remake('opt-x-trace', opts, 'triple')
    end
  end
  it 'should be able to do handle -y (--trace=nohshell options)' do
    ['--trace=noshell -f', '-y -f'].each do |opts|
      run_remake('opt-y-trace', opts, 'triple')
    end
  end
end
