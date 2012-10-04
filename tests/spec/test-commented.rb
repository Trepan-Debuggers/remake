#!/usr/bin/env ruby
require_relative 'helper'

describe "remake --tasks" do
  include RemakeTestHelper
  it 'should show commented targets on tasks' do
    opts = {
      :exitstatus      => 0,
      :flags           => '--tasks  -f',
      :full            => true
    }
    test_name = File.basename(__FILE__, '.rb')[5..-1]
    run_remake(test_name, opts, 'commented')
  end
end
