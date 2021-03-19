%global _version 2.0.5
%global _release 20210319.090408.git6ac27845
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

Patch1:	0001-support-quiet-of-log-config.patch
Patch2:	0002-lcr-add-inactive-file-total-metric.patch

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
%cmake -DDEBUG=ON -DCMAKE_SKIP_RPATH=TRUE -DLIB_INSTALL_DIR=%{_libdir} ../
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
* Fri Mar 19 2021 wujing <wujing50@huawei.com> - 2.0.5-20210319.090408.git6ac27845
- Type:enhancement
- ID:NA
- SUG:NA
- DESC: add secure compile options

* Tue Feb 2 2021 lifeng <lifeng68@huawei.com> - 2.0.5-20210202.093043.git36603cff
- Type:sync with upstream
- ID:NA
- SUG:NA

* Wed Dec 30 2020 lifeng <lifeng68@huawei.com> - 2.0.5-20201230.150203.git5e91f13f
- Type:update to v2.0.5
- ID:NA
- SUG:NA

* Thu Dec 3 2020 haozi007 <liuhao27@huawei.com> - 2.0.4-20201203.185548.gitcc470dc1
- Type:update from master
- ID:NA
- SUG:NA
- DESC: update from master

* Thu Nov 12 2020 gaohuatao <gaohuatao@huawei.com> - 2.0.4-20201112.184125.gite8506076
- Type:update from master
- ID:NA
- SUG:NA
- DESC: update from master

* Wed Oct 14 2020 lifeng <lifeng68@huawei.com> - 2.0.4-20201014.151549.gita811a32f
- Type:upgrade to v2.0.4
- ID:NA
- SUG:NA
- DESC: upgrade to v2.0.4

* Fri Sep 04 2020 zhangxiaoyu <zhangxiaoyu58@huawei.com> - 2.0.3-20200904.101728.git8b4641a4
- Type:enhancement
- ID:NA
- SUG:NA
- DESC: modify spec file

* Thu Sep 03 2020 zhangxiaoyu <zhangxiaoyu58@huawei.com> - 2.0.3-20200903.182540.git62664adf
- Type:enhancement
- ID:NA
- SUG:NA
- DESC: upgrade from v2.0.2 to v2.0.3

* Wed Sep 02 2020 YoungJQ <yangjiaqi11@huawei.com> - 2.0.2-20200902.112545.git24f07933
- Type:enhancement
- ID:NA
- SUG:NA
- DESC: modify source0 address

* Fri Apr 14 2017 Hui Wang <hw.huiwang@huawei.com> - 0.0.1
- Initial RPM release
