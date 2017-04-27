#!/usr/bin/env ruby
require_relative 'helper'

describe "list" do
  include RemakeTestHelper
  it 'should be able to run plain list, list with line number, and target' do
    opts = {
      :filter => Filter_filename,
      :exitstatus  => 77,
      :flags       =>'-X -f',
      :input       => "echo 'list
list third
list 3
list 6
quit
'"
    }
    test_name = File.basename(__FILE__, '.rb')[5..-1]
    run_remake("#{test_name}", opts, 'triple')
  end
end
