$!
$! Makefile.com - builds GNU Make for VMS
$!
$! P1 is non-empty if you want to link with the VAXCRTL library instead
$!    of the shareable executable
$! P2 = DEBUG will build an image with debug information
$!
$! In case of problems with the install you might contact me at
$! zinser@decus.decus.de (preferred) or martin_zinser@exchange.de
$!
$! Look for the compiler used
$!
$ lval = ""
$ if f$search("SYS$SYSTEM:DECC$COMPILER.EXE").eqs.""
$  then
$   if f$trnlnm("SYS").eqs."" then def/nolog sys sys$library:
$   ccopt = ""
$  else
$   ccopt = "/decc/prefix=all"
$   if f$trnlnm("SYS").eqs.""
$    then
$     if f$trnlnm("DECC$LIBRARY_INCLUDE").nes.""
$      then
$       define sys decc$library_include:
$      else
$       if f$search("SYS$COMMON:[DECC$LIB.REFERENCE]DECC$RTLDEF.DIR").nes."" -
           then lval = "SYS$COMMON:[DECC$LIB.REFERENCE.DECC$RTLDEF],"
$       if f$search("SYS$COMMON:[DECC$LIB.REFERENCE]SYS$STARLET_C.DIR").nes."" -
           then lval = lval+"SYS$COMMON:[DECC$LIB.REFERENCE.SYS$STARLET_C],"
$       lval=lval+"SYS$LIBRARY:"
$       define sys 'lval
$      endif
$   endif
$ endif
$!
$! Should we build a debug image
$!
$ if (p2.eqs."DEBUG")
$  then
$   ccopt = ccopt + "/noopt/debug"
$   lopt = "/debug"
$ else
$   lopt = ""
$ endif
$ filelist = "alloca ar arscan commands default dir expand file function implicit job main misc read remake remote-stub rule signame variable version vmsfunctions vmsify vpath [.glob]glob [.glob]fnmatch getopt1 getopt"
$ copy config.h-vms config.h
$ n=0
$ open/write optf make.opt
$ loop:
$ cfile = f$elem(n," ",filelist)
$ if cfile .eqs. " " then goto linkit
$ write sys$output "Compiling ''cfile'..."
$ call compileit 'cfile' 'p1'
$ n = n + 1
$ goto loop
$ linkit:
$ close optf
$ if p1 .nes. "" then goto link_using_library
$ link/exe=make make.opt/opt'lopt
$ exit
$ link_using_library:
$ link/exe=make make.opt/opt,sys$library:vaxcrtl/lib'lopt
$!
$ compileit : subroutine
$ ploc = f$locate("]",p1)
$ filnam = p1
$ if ploc .lt. f$length(p1) then filnam=f$extract(ploc+1,100,p1)
$ write optf "''filnam'"
$ cc'ccopt'/include=([],[.glob])/define=("allocated_variable_expand_for_file=alloc_var_expand_for_file","unlink=remove","HAVE_CONFIG_H","VMS") 'p1'
$ exit
$ endsubroutine : compileit
