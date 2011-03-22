#!/usr/bin/env ruby
require_relative 'helper'

describe "breakpoints" do
  include RemakeTestHelper
  it 'should be able to do set a breakpoint on a target (no options)' do
    opts = {
      :filter => Filter_filename,
      :flags  =>'-X -f',
      :input  => "echo 'break third
continue
quit
'"
    }
    test_name = File.basename(__FILE__, '.rb')[5..-1]
    run_remake(test_name, opts, 'triple')
  end
end
