#!/usr/bin/env ruby
require 'rubygems'
require 'rspec'
require_relative 'helper'

describe "stepping" do
  def run_remake(test_name, input, opts, makefile_short=nil)
    makefile_short = test_name unless makefile_short
    makefile = File.join(EXAMPLE_DIR, makefile_short + '.Makefile')
    rightfile = File.join(DATA_DIR, test_name + '.right')
    expected_output = File.open(rightfile, 'r').readlines.map{|l| l.chomp}
    output = `#{input} | #{MAKE_PROG} #{opts} #{makefile}`.split("\n")[6..-1]
    output.each do |line|
      line.gsub!(/^.*(#{makefile_short}\.Makefile:\d+)/,'\1')
    end
    output.should == expected_output
  end

  it 'should be able to do handle simple debugger stepping' do
    opts = '-X -f'
    input = "echo 'step
step
step
'"
    run_remake('step3', input, opts, 'triple')
  end
end
