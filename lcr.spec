%global _version 2.0.2
%global _release 20200526.162423.gitf1095eab
%global _inner_name isula_libutils

Name:      lcr
Version:   %{_version}
Release:   %{_release}
URL:       lcr
Source:    lcr-2.0.tar.gz
Summary:   Lightweight Container Runtime
Group:     Applications/System
License:   Mulan PSL v2
BuildRoot: %{_tmppath}/lcr-%{version}

BuildRequires: cmake
BuildRequires: lxc
BuildRequires: lxc-devel
BuildRequires: zlib-devel yajl-devel gtest-devel
Requires:      lxc yajl zlib
ExclusiveArch:  x86_64 aarch64

%ifarch x86_64
Provides:       liblcr.so()(64bit)
Provides:       libisula_libutils.so()(64bit)
%endif

%ifarch aarch64
Provides:       liblcr.so()(64bit)
Provides:       libisula_libutils.so()(64bit)
%endif

%description
Containers are insulated areas inside a system, which have their own namespace
for filesystem, network, PID, IPC, CPU and memory allocation and which can be
created using the Control Group and Namespace features included in the Linux
kernel.

This package provides the lightweight container tools and library to control
lxc-based containers.

%package devel
Summary: Huawei container runtime, json and log C Library
Group:   Libraries
ExclusiveArch:  x86_64 aarch64
Requires:       %{name} = %{version}-%{release}

%description devel
the %{name}-libs package contains libraries for running iSula applications.

%global debug_package %{nil}

%prep
%setup -c -n lcr-%{version}

%build
mkdir -p build
cd build
%cmake -DDEBUG=OFF -DLIB_INSTALL_DIR=%{_libdir} ../
%make_build

%install
rm -rf %{buildroot}
cd build
mkdir -p %{buildroot}/{%{_libdir},%{_libdir}/pkgconfig,%{_includedir}/lcr,%{_bindir}}
install -m 0644 ./src/liblcr.so            %{buildroot}/%{_libdir}/liblcr.so
install -m 0644 ./conf/lcr.pc          %{buildroot}/%{_libdir}/pkgconfig/lcr.pc
install -m 0644 ../src/lcrcontainer.h  %{buildroot}/%{_includedir}/lcr/lcrcontainer.h

install -m 0644 ./src/libisula_libutils.so        %{buildroot}/%{_libdir}/libisula_libutils.so
install -d $RPM_BUILD_ROOT/%{_includedir}/%{_inner_name}
install -m 0644 ../build/json/*.h  %{buildroot}/%{_includedir}/%{_inner_name}/
install -m 0644 ../src/json/*.h  %{buildroot}/%{_includedir}/%{_inner_name}/
install -m 0644 ../third_party/log.h  %{buildroot}/%{_includedir}/%{_inner_name}/log.h

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
%{_libdir}/pkgconfig/lcr.pc

%files devel
%defattr(-,root,root,-)
%{_includedir}/lcr/lcrcontainer.h
%{_includedir}/%{_inner_name}/*.h


%changelog
* Fri Apr 14 2017 Hui Wang <hw.huiwang@huawei.com> - 0.0.1
- Initial RPM release
