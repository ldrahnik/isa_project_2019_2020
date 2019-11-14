Name:                 dns
Version:              0.1
Release:              0
Summary:              Program zasílá DNS dotaz na DNS server od kterého očekává odpověd.
License:              OSS FIT VUT
URL:                  https://github.com/ldrahnik/isa_project_2019_2020
Source0:              https://github.com/ldrahnik/isa_project_2019_2020/archive/v%{version}.tar.gz
BuildArch:            x86_64
AutoReqProv:          yes
Requires:             glibc

%description
Program zasílá DNS dotaz na DNS server od kterého očekává odpověd. V DNS dotazu program umožnuje požadovat reverzní dotaz typu PTR pomocí parametru -x, IPv6 dotaz typu AAAA pomocí parametru -6 nebo bez uvedení zmíněných parametrů defaultní typ A. Dále lze nastavit jiný port než defaultní 53 pomocí parametru -p a lze požadovat rekurzivní typ dotazu.

%prep
%setup -q -n isa_project_2019_2020-%{version}

%build
make
make tex

%install
sudo rm -rf %{buildroot}
sudo make install BUILD_ROOT=%{buildroot} VERSION=%{version}

%__spec_install_post
/usr/lib/rpm/brp-compress
/usr/lib/rpm/brp-strip
/usr/lib/rpm/brp-strip-comment-note

%clean
sudo rm -rf %{buildroot}

%files
%dir /usr/lib/%{name}/
/usr/lib/%{name}/%{name}
/usr/bin/%{name}
/usr/share/man/man1/%{name}.1.gz
/usr/share/doc/%{name}/manual.pdf
/usr/share/licenses/%{name}/LICENSE

%changelog
* Wed Nov 13 2019 Lukáš Drahník <xdrahn00@stud.fit.vutbr.cz>
- 0.1
- First draft
