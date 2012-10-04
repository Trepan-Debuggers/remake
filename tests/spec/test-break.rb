#!/usr/bin/env ruby
require_relative 'helper'

describe "breakpoints" do
  include RemakeTestHelper
  it 'should be able to do set a breakpoint on a target with subopt' do
    ['end', 'prereq', 'run'].each do |break_opt|
      opts = {
        :filter => Filter_filename,
        :flags  =>'-X -f',
        :input  => "echo 'break third #{break_opt}
continue
quit
'"
      }
      test_name = File.basename(__FILE__, '.rb')[5..-1]
      run_remake("#{test_name}-#{break_opt}", opts, 'triple')
    end
  end
  it 'should be able to do set a breakpoint on a target' do
    opts = {
      :filter => Filter_filename,
      :flags  =>'-X -f',
      :input  => "echo 'break third
continue
continue
continue
quit
'"
    }
    test_name = File.basename(__FILE__, '.rb')[5..-1]
    run_remake("#{test_name}-all", opts, 'triple')
  end

  it 'should be able to do set a breakpoint on a target with expansion' do
    opts = {
      :filter => Filter_filename,
      :flags  =>'-X -f',
      :input  => "echo 'break $(PACKAGE).txt
quit
'"
    }
    test_name = File.basename(__FILE__, '.rb')[5..-1]
    run_remake("#{test_name}-expand", opts, 'test2')
  end
end
