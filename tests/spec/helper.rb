require 'rubygems'
require 'rspec'
require 'tmpdir'
module RemakeTestHelper
  unless defined?(SPEC_DIR)
    SPEC_DIR = File.dirname(__FILE__)
    EXAMPLE_DIR = File.join(SPEC_DIR, 'example')
    DATA_DIR = File.join(SPEC_DIR, 'data')
    MAKE_PROG = File.join(SPEC_DIR, '..', '..', 'remake')
    Filter_filename = Proc.new{|makefile_short, lines|
      lines.each do |line|
        line.gsub!(/^.*(#{makefile_short}\.Makefile:\d+)/,'\1')
        line.gsub!(/ file .*(#{makefile_short})\.Makefile, line/,' file \1.Makefile, line')
        line.gsub!(/^#  commands to execute \(from `.*#{makefile_short}/,
                   "#  commands to execute (from `#{makefile_short}")
      end
    }
  end

  module_function
  def run_remake(test_name, opts, makefile_short=nil)
    makefile_short = test_name unless makefile_short
    makefile = File.join(EXAMPLE_DIR, makefile_short + '.Makefile')
    rightfile = File.join(DATA_DIR, test_name + '.right')
    expected_output = File.open(rightfile, 'r').readlines.map{|l| l.chomp}
    cmd = "#{MAKE_PROG} #{opts[:flags]} #{makefile}"
    cmd = "#{opts[:input]} | #{cmd}" if opts[:input]
    lines = `#{cmd} 2>&1`.split("\n")
    lines = lines[6..-1] unless opts[:full]
    opts[:filter].call(makefile_short, lines) if opts[:filter]
    exit_status = opts[:exitstatus] || 77
    if $?.exitstatus != exit_status
      puts expected_output
    end
    $?.exitstatus.should == exit_status
    unless opts[:no_check_output]
      if lines != expected_output
        File.open(Dir::tmpdir + "/#{test_name}.check", 'w') do |f|
          lines.map {|l| f.puts(l) }
        end
      end
      lines.should == expected_output
    end
  end


end
