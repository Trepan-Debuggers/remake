#!/usr/bin/env ruby
require_relative 'helper'

describe "info target" do
  include RemakeTestHelper
  it 'should run "info targets", and with options "name" and "position"' do
    opts = {
      :filter          => Filter_filename, 
      :flags           => '-X  -f',
      :input           => "echo 'info targets
info targets names
info targets positions
quit
'"
    }
    test_name = File.basename(__FILE__, '.rb')[5..-1]
    run_remake(test_name, opts, 'triple')
  end
end
