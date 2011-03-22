#!/usr/bin/env ruby
require_relative 'helper'

describe "breakpoints" do
  include RemakeTestHelper
  it 'should be able to do set a breakpoint on a target' do
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
end
