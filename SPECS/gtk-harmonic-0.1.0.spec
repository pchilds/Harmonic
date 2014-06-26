Summary:	Provides a Harmonic spectrum analyser
Name:		gtk-harmonic
Version:	0.1.0
Release:	1%{?dist}
License:	GPLv2+
Group:		Applications/Engineering
Source:		%{name}-%{version}.tar.gz
BuildRequires:	automake, autoconf, fftw-devel >= 3.0, gettext, gnome-doc-utils, gtk2 >= 2.18, gtk2plot, pkgconfig
Requires:	fftw >= 3.0, gtk2 >= 2.18, gtk2plot

%description
%{name} provides a GUI for providing harmonic transformations
 (e.g. Fourier) of spectral regions and analysis of the resultant
 peaks for determining spectral shifts, interference visibility
 and chirp.

%prep
%setup -q

%build
%configure
make %{?_smp_mflags}

%install
%make_install
desktop-file-install --dir=${RPM_BUILD_ROOT}%{_datadir}/applications harmonic.desktop
%find_lang %{name}

%files -f %{name}.lang
%defattr(-,root,root)
%doc COPYING COPYING-DOCS README
%{_bindir}/Harmonic
%{_datadir}/applications/harmonic.desktop
%{_datadir}/gnome/help/Harmonic-help/C/Harmonic-help.xml
%{_datadir}/gnome/help/Harmonic-help/C/figures/ex4_1.png
%{_datadir}/gnome/help/Harmonic-help/C/figures/ex4_2.png
%{_datadir}/omf/Harmonic-help/Harmonic-help-C.omf
%{_datadir}/pixmaps/db8.png

%changelog
* Thu Jun 26 2014 Paul Childs <pchilds@physics.org> 0.1.0-1
- Initial build
