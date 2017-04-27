How to use
----------

* Copy the approproate tarball to `rmbuild/SOURCES`.
* Run `rpmbuild`. `rpmbuild` is in found in package `rpm-build`.

For example  to build just a binary only on CentOS 7:

```
$ sudo yum -y install rpm-build
$ cp remake-3.82+dbg0.9.tar.bz2 ~/rpmbuild/SOURCES
$ cp remake-centos6.spec remake.spec
$ rpmbuild -bb remake-centos7.spec
```
