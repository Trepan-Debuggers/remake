#!/usr/bin/env ruby
require_relative 'helper'

describe "info target" do
  include RemakeTestHelper
  it 'should run info target order, nonorder, and depend command' do
    opts = {
      :filter          => Filter_filename, 
      :flags           => '-X  -f',
      :input           => "echo 'target @ order
target @ nonorder
target @ depend
quit
'"
    }
    test_name = File.basename(__FILE__, '.rb')[5..-1]
    run_remake(test_name, opts, 'order-only')
  end
end
