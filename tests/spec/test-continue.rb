#!/usr/bin/env ruby
require_relative 'helper'

describe "stepping" do
  include RemakeTestHelper
  it 'should be able to do handle simple debugger stepping' do
    opts = {
      :filter => Filter_filename, 
      :exitstatus  => 0,
      :flags  => '-X -f',
      :input  => "echo 'continue third
finish 10
finish 2
'"
    }
    test_name = File.basename(__FILE__, '.rb')[5..-1]
    run_remake(test_name, opts, 'triple')
  end
end
