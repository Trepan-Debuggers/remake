@echo off
echo Configuring MAKE for DJGPP
rem This batch file assumes a unix-type "sed" program

update configh.dos config.h

rem Do they have Make?
redir -o junk.$$$ -eo make -n -f NUL
rem REDIR will return 1 if it cannot run Make.
rem If it can run Make, it will usually return 2,
rem but 0 is also OK with us.
if errorlevel 2 goto MakeOk
if not errorlevel 1 goto MakeOk
if exist junk.$$$ del junk.$$$
echo No Make program found--use DOSBUILD.BAT to build Make.
goto End

rem They do have Make.	Generate Makefiles.

:MakeOk
del junk.$$$
echo # Makefile generated for DJGPP by "configure.bat"> Makefile

if exist config.sed del config.sed

echo ": try_sl						">> config.sed
echo "/\\$/ {						">> config.sed
echo "	N						">> config.sed
echo "	s/[	]*\\\n[		]*/ /			">> config.sed
echo "	b try_sl					">> config.sed
echo "}							">> config.sed

echo "s/@srcdir@/./					">> config.sed
echo "s/@exec_prefix@/$(DJDIR)/				">> config.sed
echo "s/@prefix@/$(DJDIR)/				">> config.sed
echo "s/@CC@/gcc/					">> config.sed
echo "s/@CFLAGS@/-O2 -g/				">> config.sed
if "%1"=="no-float" goto nofloat
if "%1"=="NO-FLOAT" goto nofloat
if "%1"=="NO_FLOAT" goto nofloat
if "%1"=="no_float" goto nofloat
echo "s/@CPPFLAGS@/-DHAVE_CONFIG_H/			">> config.sed
goto floatdone
:nofloat
echo "s/@CPPFLAGS@/-DHAVE_CONFIG_H -DNO_FLOAT/		">> config.sed
:floatdone
echo "s/@LDFLAGS@//					">> config.sed
echo "s/@RANLIB@/ranlib/				">> config.sed
echo "s/@DEFS@//					">> config.sed
echo "s/@REMOTE@/stub/					">> config.sed
echo "s/@ALLOCA@//					">> config.sed
echo "s/@LIBS@//					">> config.sed
echo "s/@LIBOBJS@//					">> config.sed
echo "s/@SET_MAKE@//					">> config.sed
echo "s/@NEED_SETGID@/false/				">> config.sed
echo "s/@INSTALL_PROGRAM@/install/			">> config.sed
echo "s/@INSTALL_DATA@/install -m 644/			">> config.sed
echo "s/@INSTALL@/install/				">> config.sed
echo "s/^Makefile *:/_Makefile:/			">> config.sed
echo "s/^config.h *:/_config.h:/			">> config.sed
echo "s/^defines *=.*$/defines =/			">> config.sed
echo "/mv -f make.new make/d				">> config.sed

echo "s/cd glob; $(MAKE)/$(MAKE) -C glob/		">> config.sed

echo "/^tagsrcs *=/s/\$(srcs)/$(srcs:.h.in=.h)/		">> config.sed

echo "s/\*.o/*.o *.exe make.new/			">> config.sed
echo "s/\.info\*/.i*/g					">> config.sed

sed -e "s/^\"//" -e "s/\"$//" -e "s/[	]*$//" config.sed > config2.sed
if exist config2.sed goto SedOk
echo To configure Make you need a Unix-style Sed program!
goto End
:SedOk
sed -f config2.sed Makefile.in >> Makefile
del config.sed
del config2.sed

cd glob
call configure
cd ..
:End
