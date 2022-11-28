Name: tarantool-memcached
Version: 0.1.0
Release: 1%{?dist}
Summary: Memcached protocol emulation for Tarantool
Group: Applications/Databases
License: BSD
URL: https://github.com/tarantool/memcached
Source0: https://github.com/tarantool/memcached/archive/%{version}/%{name}-%{version}.tar.gz
BuildRequires: cmake >= 2.8
BuildRequires: gcc >= 4.5
BuildRequires: tarantool-devel >= 1.10.0
BuildRequires: /usr/bin/prove
BuildRequires: cyrus-sasl-devel
Requires: tarantool >= 1.10.0

%description
This package provides a Memcached protocol emulation for Tarantool.

%prep
%setup -q -n %{name}-%{version}

%build
%cmake . -DCMAKE_BUILD_TYPE=RelWithDebInfo
%if 0%{?fedora} >= 33 || 0%{?rhel} >= 8
  %cmake_build
%else
  make %{?_smp_mflags}
%endif


%install
%if 0%{?fedora} >= 33 || 0%{?rhel} >= 8
  %cmake_install
%else
  %make_install
%endif

%files
%{_libdir}/tarantool/*/
%{_datarootdir}/tarantool/*/
%doc README.md
%{!?_licensedir:%global license %doc}
%license LICENSE

%changelog
* Thu Feb 24 2016 Eugene Blikh <bigbes@tarantool.org> 0.1.0-1
- Initial version of the RPM spec
