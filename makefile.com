$!
$! Makefile.com - builds GNU Make for VMS
$!
$! P1 is non-empty if you want to link with the VAXCRTL library instead
$!    of the shareable executable
$!
$ def/nolog sys sys$library:
$ filelist = "alloca commands default dir expand file function implicit job main misc read remake remote-stub rule signame variable version vmsfunctions vmsify vpath [.glob]glob [.glob]fnmatch getopt getopt1"
$ copy config.h-vms config.h
$ n=0
$ loop:
$ cfile = f$elem(n," ",filelist)
$ if cfile .eqs. " " then goto linkit
$ write sys$output "Compiling ''cfile'..."
$ call compileit 'cfile' 'p1'
$ n = n + 1
$ goto loop
$ linkit:
$ if p1 .nes. "" then goto link_using_library
$ link/exe=make alloca,commands,default,dir,expand,file,function,-
		implicit,job,main,misc,read,remake,remote-stub,rule,-
		signame,variable,version,vmsfunctions,vmsify,vpath,-
		glob,fnmatch,getopt,getopt1
$ exit
$ link_using_library:
$ link/exe=make alloca,commands,default,dir,expand,file,function,-
		implicit,job,main,misc,read,remake,remote-stub,rule,-
		signame,variable,version,vmsfunctions,vmsify,vpath,-
		glob,fnmatch,getopt,getopt1,sys$library:vaxcrtl/lib
$!
$ compileit : subroutine
$ cc/include=([],[.glob])/define=("allocated_variable_expand_for_file=alloc_var_expand_for_file","unlink=remove","HAVE_CONFIG_H","VMS","NO_ARCHIVES") 'p1'
$ exit
$ endsubroutine : compileit
