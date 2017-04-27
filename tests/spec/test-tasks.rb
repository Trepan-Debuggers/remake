#!/usr/bin/env ruby
require_relative 'helper'

describe "A --tasks options" do
  include RemakeTestHelper
  before(:each) do
    @opts = {
      :exitstatus => 0,
      :full => true,
    }
  end
    
  it 'should be able to give trace descriptions)' do
    @opts[:flags] = '--tasks -f'
    run_remake('description-trace', @opts, 'simple')
  end

end
