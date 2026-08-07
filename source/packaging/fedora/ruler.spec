Name:           ruler
Version:        0.1.0
Release:        1%{?dist}
Summary:        Ekranski ravnalo (screen ruler) - GTK3 port originalnog Windows Forms alata

License:        MIT
URL:            https://example.com/ruler
Source0:        %{name}-%{version}.tar.gz

# --- Build dependencije: jednim "dnf builddep ruler.spec" instaliras sve ---
BuildRequires:  meson
BuildRequires:  ninja-build
BuildRequires:  gcc
BuildRequires:  pkgconf-pkg-config
BuildRequires:  gtk3-devel
BuildRequires:  libX11-devel

# --- Runtime dependencije (instaliraju se automatski uz sam paket) ---
Requires:       gtk3
Requires:       libX11

%description
Ekranski ravnalo za mjerenje elemenata na zaslonu, prijenozen s
Windows Forms/VB.NET verzije na GTK3 preko X11/XWayland-a.
Podrzava pomicanje, skaliranje preko rubova, kapaljku boje piksela
izravno iz frame buffera, hairline mod i prikaz dijagonale.

%prep
%autosetup

%build
%meson
%meson_build

%install
%meson_install

%files
%{_bindir}/ruler
%license LICENSE
%doc README.md

%changelog
* %(date "+%a %b %d %Y") Packaging Bot <noreply@example.com> - 0.1.0-1
- Inicijalni GTK3 port skeleton (glavni prozor + options dijalog)
