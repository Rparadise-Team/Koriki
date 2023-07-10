%define    name    bftpd
%define    version 2.3
%define    release 1
%define    prefix  /usr

Summary:   A small, fast and easy-to-configure FTP server.
Name:      %{name}
Version:   %{version}
Release:   %{release}
License:   GPL
Group:     System Environment/Daemons
Vendor:    Jesse Smith <jessefrgsmith@yahoo.ca>
URL:       http://bftpd.sourceforge.net/
Source0:   http://bftpd.sourceforge.net/downloads/rpm/%{name}-%{version}.tar.gz
BuildArch: i386
#BuildRoot: /var/tmp/%{name}-root
Provides:  bftpd

%description
A very configurable small FTP server
bftpd is a easy-to-configure and small FTP server that supports chroot
without special directory preparation or configuration. Most FTP commands
are supported.

%prep
%setup 
%build
make

%install
make install

%clean
rm -rf "$RPM_BUILD_ROOT"

%files
%defattr(-,root,root)
%config(noreplace) %verify(not mtime) /etc/bftpd.conf
%{prefix}/sbin/bftpd
%{prefix}/share/man/man8/bftpd.8

%changelog
* Mon Jan 1 2007 Joe Klemmer <joe@webtrek.com>
- updated the version number.
* Mon Jan 9 2006 Joe Klemmer <joe@webtrek.com>
- added defined variables to the top of the file.
- set the config file in the %files section so it won't
  be over written on upgrades.
- added a default attributes to the %files section.
- redid the summery section to bring it in line with "rpm
  spec file standards" (an oxymoron if ever there was one).
- changed the deprecated "Copyright" into "License".

