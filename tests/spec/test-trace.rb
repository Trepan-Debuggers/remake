#!/usr/bin/env ruby
require_relative 'helper'

describe "A simple trace" do
  include RemakeTestHelper
  before(:each) do
    @opts = {
      :filter => Filter_filename,
      :exitstatus => 0,
    }
  end
    
  it 'should be able to do handle --trace (no options)' do
    ['--trace --file', '-x -f'].each do |flags|
      p flags if $DEBUG
      @opts[:flags] = flags
      run_remake('opt-x-trace', @opts, 'triple')
    end
  end
  it 'should be able to do handle -y (--trace=nohshell options)' do
    ['--trace=noshell -f', '-y -f'].each do |flags|
      p flags if $DEBUG
      @opts[:flags] = flags
      run_remake('opt-y-trace', @opts, 'triple')
    end
  end
  it 'should be give a warning for -x and -y' do
    flags = ['-x -y', '-y -x'].each do |flags|
      p flags if $DEBUG
      @opts[:flags] = flags
      run_remake('opt-x-y-trace', @opts, 'simple')
    end
  end
end
