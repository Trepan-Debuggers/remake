#!/usr/bin/env ruby
require_relative 'helper'

describe "stepping" do
  include RemakeTestHelper
  it 'should be able to do run an "info rule" command' do
    opts = {
      :filter => Filter_filename, 
      :flags  => '-X  -f',
      :input  => "echo 'info rule
quit
'"
    }
    test_name = File.basename(__FILE__, '.rb')[5..-1]
    run_remake(test_name, opts, 'implicit')
  end
end
