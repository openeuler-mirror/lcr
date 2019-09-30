%global _version 1.0.15
%global _release 20190612.105034.gitefd3b7ac
Name:      lcr
Version:   %{_version}
Release:   %{_release}%{?dist}
URL:       http://code.huawei.com/containers/lcr
Source:    %{name}-1.0.tar.gz
Summary:   Lightweight Container Runtime
Group:     Applications/System
License:   Mulan PSL v1
BuildRoot: %{_tmppath}/%{name}-%{version}

BuildRequires: cmake
BuildRequires: lxc
BuildRequires: lxc-devel
BuildRequires: yajl yajl-devel
BuildRequires: libsecurec libsecurec-devel
Requires:      rsync bridge-utils lxc
ExclusiveArch:  x86_64 aarch64

%ifarch x86_64
Provides:       liblcr.so()(64bit)
%endif

%ifarch aarch64
Provides:       liblcr.so()(64bit)
%endif

%description
Containers are insulated areas inside a system, which have their own namespace
for filesystem, network, PID, IPC, CPU and memory allocation and which can be
created using the Control Group and Namespace features included in the Linux
kernel.

This package provides the lightweight container tools and library to control
lxc-based containers.

%global debug_package %{nil}

%prep
%setup -c -n %{name}-%{version}

%build
mkdir -p build
cd build
%cmake -DDEBUG=OFF -DLIB_INSTALL_DIR=%{_libdir} ../
%make_build

%install
rm -rf %{buildroot}
cd build
mkdir -p %{buildroot}/{%{_libdir},%{_libdir}/pkgconfig,%{_includedir}/%{name},%{_bindir}}
install -m 0644 ./src/liblcr.so            %{buildroot}/%{_libdir}/liblcr.so
install -m 0644 ./conf/lcr.pc          %{buildroot}/%{_libdir}/pkgconfig/lcr.pc
install -m 0644 ../src/lcrcontainer.h  %{buildroot}/%{_includedir}/%{name}/lcrcontainer.h

find %{buildroot} -type f -name '*.la' -exec rm -f {} ';'
find %{buildroot} -name '*.a' -exec rm -f {} ';'
find %{buildroot} -name '*.cmake' -exec rm -f {} ';'

%clean
rm -rf %{buildroot}

%pre

%post  -p /sbin/ldconfig

%postun  -p /sbin/ldconfig

%files
%defattr(-,root,root,-)
%{_libdir}/*
%{_libdir}/pkgconfig/%{name}.pc
%{_includedir}/%{name}/lcrcontainer.h

%changelog
* Fri Apr 14 2017 Hui Wang <hw.huiwang@huawei.com> - 0.0.1
- Initial RPM release
