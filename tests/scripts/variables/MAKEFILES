$description = "The following test creates a makefile to test ";

$makefile2 = &get_tmpfile;

open(MAKEFILE,"> $makefile");
# The Contents of the MAKEFILE ...
print MAKEFILE "MAKEFILES = work/MAKEFILES_variable.mk.2\n\n";
print MAKEFILE "all:\n";
print MAKEFILE "\t\@echo THIS IS THE DEFAULT RULE\n";
# END of Contents of MAKEFILE
close(MAKEFILE);


open(MAKEFILE,"> $makefile2");
print MAKEFILE "NDEF:\n";
print MAKEFILE "\t\@echo THIS IS THE RULE FROM MAKEFILE 2\n";
close(MAKEFILE);

&run_make_with_options($makefile,"",&get_logfile);


# Create the answer to what should be produced by this Makefile
$answer = "THIS IS THE DEFAULT RULE\n";

# COMPARE RESULTS

# In this call to compare output, you should use the call &get_logfile(1)
# to send the name of the last logfile created.

&compare_output($answer,&get_logfile(1));

# If you wish to stop if the compare fails, then add
# a "|| &error ("abort")" to the
# end of the previous line.

# This tells the test driver that the perl test script executed properly.
1;
