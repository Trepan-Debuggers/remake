# -*- coding: utf-8 -*-
Summary: GNU Make with comprehensible tracing and a debugger
Name: remake
Epoch: 1
Version: 3.82+dbg0.7
Release: 19%{?dist}
License: GPLv3
Group: Development/Tools
URL: http://bashdb.sf.net/remake/
Source: ftp://ftp.gnu.org/gnu/make/remake-%{version}.tar.bz2
BuildRoot: %{_tmppath}/%{name}-%{version}-%{release}-root-%(%{__id_u} -n)
BuildRequires: readline-devel
Requires(preun): /sbin/install-info

%description
remake is a patched version of GNU Make that adds improved error
reporting, the ability to trace execution in a comprehensible way, and
a debugger.

%prep
%setup -q

%build
%configure
make %{?_smp_mflags}

%install
rm -rf ${RPM_BUILD_ROOT}
make DESTDIR=$RPM_BUILD_ROOT install
rm -f ${RPM_BUILD_ROOT}/%{_infodir}/dir

%find_lang %name

%check
echo ============TESTING===============
/usr/bin/env LANG=C make check
echo ============END TESTING===========

%clean
rm -rf ${RPM_BUILD_ROOT}

%post
if [ -f %{_infodir}/remake.info.gz ]; then # for --excludedocs
   /sbin/install-info %{_infodir}/remake.info.gz %{_infodir}/dir --entry="* Make: (remake).                 The GNU remake utility." || :
fi

%preun
if [ $1 = 0 ]; then
   if [ -f %{_infodir}/make.info.gz ]; then # for --excludedocs
      /sbin/install-info --delete %{_infodir}/make.info.gz %{_infodir}/dir --entry="* Make: (remake).                 The GNU remake utility." || :
   fi
fi

%files  -f %{name}.lang
%defattr(-,root,root)
%doc NEWS README README.remake COPYING AUTHORS
%{_bindir}/*
%{_mandir}/man*/*
%{_infodir}/*.info*

%changelog
* Fri Sep 23 2011 Rocky Bernstein <rocky@gnu.org> - 1:3.81-19
- First cut via CentOS6 spec
