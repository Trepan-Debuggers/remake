#!/usr/local/bin/perl
# -*-perl-*-

# Test driver for the Make test suite

# Usage:  run_make_tests  [testname]
#                         [-debug]
#                         [-help]
#                         [-verbose]
#                         [-keep]
#                         [-make <make prog>]
#                         [-work <work dir>]
#                        (and others)

require "test_driver.pl";

sub valid_option
{
   local($option) = @_;
   if ($option =~ /^-make([-_]?path)?$/)
   {
      $make_path = shift @argv;
      if (!-f $make_path)
      {
	 print "$option $make_path: Not found.\n";
	 exit 0;
      }
      return 1;
   }
   elsif ($option =~ /^-work([-_]?dir)?$/)
   {
      $workdir = shift @argv;
      return 1;
   }

   return 0;
}

sub run_make_with_options
{
   local ($filename,$options,$logname,$expected_code) = @_;
   local($code);
   local($command) = $make_path;

   $expected_code = 0 unless defined($expected_code);

   if ($filename)
   {
      $command .= " -f $filename";
   }

   if ($options)
   {
      $command .= " $options";
   }

   $code = &run_command_with_output($logname,$command);

   # Check to see if we have Purify errors.  If so, keep the logfile.
   # For this to work you need to build with the Purify flag -exit-status=yes

   if ($pure_log && -f $pure_log) {
     if ($code & 0x7000) {
       $code &= ~0x7000;

       # If we have a purify log, save it
       $tn = $pure_testname . ($num_of_logfiles ? ".$num_of_logfiles" : "");
       print("Renaming purify log file to $tn\n") if $debug;
       rename($pure_log, "$tn")
	 || die "Can't rename $log to $tn: $!\n";
       ++$purify_errors;
     }
     else {
       unlink($pure_log);
     }
   }

   if ($code != $expected_code)
   {
      print "Error running $make_path ($code): $command\n";
      $test_passed = 0;
      return 0;
   }

   if ($profile & $vos)
   {
      system "add_profile $make_path";
   }
1;
}

sub print_usage
{
   &print_standard_usage ("run_make_tests", "[-make_path make_pathname]");
}

sub print_help
{
   &print_standard_help ("-make_path",
          "\tYou may specify the pathname of the copy of make to run.");
}

sub get_this_pwd {
   if ($vos)
   {
      $delete_command = "delete_file";
      $__pwd = `++(current_dir)`;
   }
   else
   {
      $delete_command = "rm";
      chop ($__pwd = `pwd`);
   }

   return $__pwd;
}

sub set_defaults
{
   # $profile = 1;
   $testee = "GNU make";
   $make_path = "make";
   $tmpfilesuffix = "mk";
   $pwd = &get_this_pwd;

   # Find the full pathname of Make.  For DOS systems this is more
   # complicated, so we ask make itself.

   open(MAKEFILE,"> makefile.tmp");
   print MAKEFILE 'all: ; @echo $(MAKE)' . "\n";
   close(MAKEFILE);
   $make_path = `$make_path -f makefile.tmp`;
   chop $make_path;
   unlink "makefile.tmp";
}

sub set_more_defaults
{
   local($string);
   local($index);

   $string = `$make_path -v -f /dev/null 2> /dev/null`;

   $string =~ s/[,\n].*/\n/s;
   $testee_version = $string;

   $string = `sh -c "$make_path -f /dev/null 2>&1"`;
   if ($string =~ /(.*): \*\*\* No targets\.  Stop\./) {
     $make_name = $1;
   }
   else {
     if ($make_path =~ /$pathsep([^\n$pathsep]*)$/) {
       $make_name = $1;
     }
     else {
       $make_name = $make_path;
     }
   }

   # prepend pwd if this is a relative path (ie, does not
   # start with a slash, but contains one).  Thanks for the
   # clue, Roland.

   if (index ($make_path, ":") != 1 && index ($make_path, "/") > 0)
   {
      $mkpath = "$pwd$pathsep$make_path";
   }
   else
   {
      $mkpath = $make_path;
   }

   # Get Purify log info--if any.

   ($pure_log = $ENV{PURIFYOPTIONS}) =~ s,.*-logfile=([^ ]+) .*,\1,;
   $pure_log =~ s/%v/$make_name/;
   $purify_errors = 0;

   $string = `sh -c "$make_path -j 2 -f /dev/null 2>&1"`;
   if ($string =~ /not supported/) {
     $parallel_jobs = 0;
   }
   else {
     $parallel_jobs = 1;
   }
}

sub setup_for_test
{
   $makefile = &get_tmpfile;
   if (-f $makefile)
   {
      unlink $makefile;
   }

   # Get rid of any Purify logs.
   ($pure_testname = $testname) =~ tr,/,_,;
   $pure_testname = "$pure_log.$pure_testname";
   system("rm -f $pure_testname*");
   print("Purify testfiles are: $pure_testname*\n") if $debug;
}

exit !&toplevel;
