#!/usr/bin/env ruby
require_relative 'helper'

describe "test-set" do
  include RemakeTestHelper
  it 'should be able to do run a "set" command' do
    opts = {
      :filter          => Filter_filename, 
      :flags           => '-X  -f',
      :input           => "echo 'set
set silent on
set silent off
set keep-going off
expand $(word 2)
quit
'"
    }
    test_name = File.basename(__FILE__, '.rb')[5..-1]
    run_remake(test_name, opts, 'implicit')
  end
end
