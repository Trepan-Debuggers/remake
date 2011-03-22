#!/usr/bin/env ruby
require 'rubygems'
require 'rspec'

SPEC_DIR = File.dirname(__FILE__)
EXAMPLE_DIR = File.join(SPEC_DIR, 'example')
DATA_DIR = File.join(SPEC_DIR, 'data')
MAKE_PROG = File.join(SPEC_DIR, '..', '..', 'remake')

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

  it 'should be able to do a simple trace' do
    run_remake('simple-trace', '--trace -f', 'triple')
  end
end
