%global _version 2.0.6
%global _release 6
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

Patch1: 0001-modified-ipconfig.json-to-adapt-to-newest-version-of.patch
Patch2: 0002-disable-lxc_keep-with-oci-image.patch
Patch3: 0003-add-self-def-runtime-for-shimv2.patch
Patch4: 0004-move-cri-runtimes-to-daemon.patch
Patch5: 0005-config-v2-and-inspect-were-modified-to-support-modif.patch
Patch6: 0006-support-null-value-in-json.patch

%define lxcver 4.0.3-2021012801

BuildRequires: cmake gcc gcc-c++ git
BuildRequires: lxc >= %{lxcver}
BuildRequires: lxc-devel >= %{lxcver}
BuildRequires: zlib-devel yajl-devel gtest-devel
Requires:      lxc >= %{lxcver} yajl zlib
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
* Wed Nov 24 2021 haozi007 <liuhao27@huawei.com> - 2.0.6-6
- Type:enhancement
- ID:NA
- SUG:NA
- DESC: support null value in json

* Tue Nov 23 2021 chengzeruizhi <chengzeruizhi@huawei.com> - 2.0.6-5
- Type:enhancement
- ID:NA
- SUG:NA
- DESC: add new fields in config v2 and inspect to support modifications on iSulad

* Fri Nov 19 2021 gaohuatao <gaohuatao@huawei.com> - 2.0.6-4
- Type:enhancement
- ID:NA
- SUG:NA
- DESC: update to 2.0.6

* Wed Nov 10 2021 gaohuatao <gaohuatao@huawei.com> - 2.0.6-2
- Type:enhancement
- ID:NA
- SUG:NA
- DESC: update to 2.0.6

* Mon Jun 28 2021 wujing <wujing50@huawei.com> - 2.0.5-20210628.165131.git738752d8
- Type:enhancement
- ID:NA
- SUG:NA
- DESC: add git and gcc to build require

* Thu Jun 24 2021 wujing <wujing50@huawei.com> - 2.0.5-20210624.185408.git4ce88a49
- Type:enhancement
- ID:NA
- SUG:NA
- DESC: support auto resize of isulad shim

* Tue May 18 2021 wagnfengtu <wagnfengtu@huawei.com> - 2.0.5-20210518.110611.git5225bddc
- Type:enhancement
- ID:NA
- SUG:NA
- DESC: sync patches from upstream

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
