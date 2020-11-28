%global _version 2.0.4
%global _release 20201112.184125.gite8506076
%global _inner_name isula_libutils

Name:      lcr
Version:   %{_version}
Release:   %{_release}
URL:       https://gitee.com/openeuler/lcr
Source:    https://gitee.com/openeuler/lcr/repository/archive/v%{version}.tar.gz
Summary:   Lightweight Container Runtime
Group:     Applications/System
License:   LGPLv2.1+
BuildRoot: %{_tmppath}/lcr-%{version}

Patch9001: 0001-support-local-volume.patch
Patch9002: 0002-remove-extension-json-config-of-cni-net.patch
Patch9003: 0003-fix-ctest-error-when-use-make.patch

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


%prep
%autosetup -n lcr -Sgit -p1

%build
mkdir -p build
cd build
%cmake -DDEBUG=ON -DLIB_INSTALL_DIR=%{_libdir} ../
%make_build

%install
rm -rf %{buildroot}
cd build
mkdir -p %{buildroot}/{%{_libdir},%{_libdir}/pkgconfig,%{_includedir}/lcr,%{_bindir}}
install -m 0644 ./src/liblcr.so            %{buildroot}/%{_libdir}/liblcr.so
install -m 0644 ./conf/lcr.pc          %{buildroot}/%{_libdir}/pkgconfig/lcr.pc
install -m 0644 ../src/lcrcontainer.h  %{buildroot}/%{_includedir}/lcr/lcrcontainer.h
chmod +x %{buildroot}/%{_libdir}/liblcr.so

install -m 0644 ./src/libisula_libutils.so        %{buildroot}/%{_libdir}/libisula_libutils.so
install -d $RPM_BUILD_ROOT/%{_includedir}/%{_inner_name}
install -m 0644 ../build/json/*.h  %{buildroot}/%{_includedir}/%{_inner_name}/
install -m 0644 ../src/json/*.h  %{buildroot}/%{_includedir}/%{_inner_name}/
install -m 0644 ../third_party/log.h  %{buildroot}/%{_includedir}/%{_inner_name}/log.h
install -m 0644 ../third_party/go_crc64.h  %{buildroot}/%{_includedir}/%{_inner_name}/go_crc64.h
chmod +x %{buildroot}/%{_libdir}/libisula_libutils.so

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
* Sat Nov 28 2020 lifeng <lifeng68@huawei.com> - 2.0.4-20201112.184125.gite8506076
- Type:clean code
- ID:NA
- SUG:NA
- DESC:remove huge patch instead of update base version

* Thu Nov 12 2020 gaohuatao <gaohuatao@huawei.com> - 2.0.3-20201112.163615.git01787bcd
- Type:enhancement
- ID:NA
- SUG:NA
- DESC: update from master

* Fri Sep 04 2020 zhangxiaoyu <zhangxiaoyu58@huawei.com> - 2.0.3-20200904.101728.git8b4641a4
- Type:enhancement
- ID:NA
- SUG:NA
- DESC: upgrade from v2.0.2 to v2.0.3

* Fri Apr 14 2017 Hui Wang <hw.huiwang@huawei.com> - 0.0.1
- Initial RPM release
