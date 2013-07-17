#------------------------------------------------------------------------------
#   scummvm.spec
#       This SPEC file controls the building of ScummVM RPM packages.
#------------------------------------------------------------------------------

#------------------------------------------------------------------------------
#   Prologue information
#------------------------------------------------------------------------------
Name		: scummvm
Version		: 1.7.0git
Release		: 1
Summary		: Graphic adventure game interpreter
Group		: Interpreters
License		: GPL
Url             : http://www.scummvm.org
Source		: %{name}-%{version}.tar.bz2
BuildRequires: desktop-file-utils
BuildRequires: libogg-devel
BuildRequires: libvorbis-devel
BuildRequires: flac-devel
BuildRequires: zlib-devel
BuildRequires: SDL2-devel
BuildRequires: freetype-devel

#------------------------------------------------------------------------------
#   Description
#------------------------------------------------------------------------------
%description
ScummVM is an interpreter that will play many graphic adventure games,
including LucasArts SCUMM games (such as Monkey Island 1-3, Day of the
Tentacle, Sam & Max, ...), many of Sierra's AGI and SCI games (such as King's
Quest 1-6, Space Quest 1-5, ...), Discworld 1 and 2, Simon the Sorcerer 1 and
2, Beneath A Steel Sky, Lure of the Temptress, Broken Sword 1 and 2, Flight of
the Amazon Queen, Gobliiins 1-3, The Legend of Kyrandia 1-3, many of Humongous
Entertainment's children's SCUMM games (including Freddi Fish and Putt Putt
games) and many more. See http://www.scummvm.org for a full compatibility list.

#------------------------------------------------------------------------------
#   install scripts
#------------------------------------------------------------------------------
%prep
%setup -q

%build
./configure --prefix=%{_prefix} --enable-release
make

%install
install -m755 -D scummvm %{buildroot}%{_bindir}/scummvm
install -m644 -D dists/scummvm.6 %{buildroot}%{_mandir}/man6/scummvm.6
install -m644 -D icons/scummvm.xpm %{buildroot}%{_datadir}/pixmaps/scummvm.xpm
install -m644 -D icons/scummvm.svg %{buildroot}%{_datadir}/icons/hicolor/scalable/apps/scummvm.svg
install -m644 -D dists/redhat/scummvm48.png %{buildroot}%{_datadir}/icons/hicolor/48x48/apps/scummvm.png
install -m644 -D gui/themes/scummclassic.zip %{buildroot}%{_datadir}/scummvm/scummclassic.zip
install -m644 -D gui/themes/scummmodern.zip %{buildroot}%{_datadir}/scummvm/scummmodern.zip
install -m644 -D gui/themes/translations.dat %{buildroot}%{_datadir}/scummvm/translations.dat
install -m644 -D dists/pred.dic %{buildroot}%{_datadir}/scummvm/pred.dic
install -m644 -D dists/engine-data/kyra.dat %{buildroot}%{_datadir}/scummvm/kyra.dat
install -m644 -D dists/engine-data/lure.dat %{buildroot}%{_datadir}/scummvm/lure.dat
install -m644 -D dists/engine-data/queen.tbl %{buildroot}%{_datadir}/scummvm/queen.tbl
install -m644 -D dists/engine-data/sky.cpt %{buildroot}%{_datadir}/scummvm/sky.cpt
install -m644 -D dists/engine-data/drascula.dat %{buildroot}%{_datadir}/scummvm/drascula.dat
install -m644 -D dists/engine-data/teenagent.dat %{buildroot}%{_datadir}/scummvm/teenagent.dat
install -m644 -D dists/engine-data/hugo.dat %{buildroot}%{_datadir}/scummvm/hugo.dat
install -m644 -D dists/engine-data/tony.dat %{buildroot}%{_datadir}/scummvm/tony.dat
install -m644 -D dists/engine-data/toon.dat %{buildroot}%{_datadir}/scummvm/toon.dat
desktop-file-install --vendor scummvm --dir=%{buildroot}/%{_datadir}/applications dists/scummvm.desktop

%clean
rm -Rf ${RPM_BUILD_ROOT}

%post
#touch --no-create %{_datadir}/icons/hicolor || :
#if [ -x %{_bindir}/gtk-update-icon-cache ]; then
#        %{_bindir}/gtk-update-icon-cache --quiet %{_datadir}/icons/hicolor || :
#fi

%postun
#touch --no-create %{_datadir}/icons/hicolor || :
#if [ -x %{_bindir}/gtk-update-icon-cache ]; then
#        %{_bindir}/gtk-update-icon-cache --quiet %{_datadir}/icons/hicolor || :
#fi

#------------------------------------------------------------------------------
#   Files listing.
#------------------------------------------------------------------------------
%files
%defattr(0644,root,root,0755)
%doc AUTHORS README NEWS COPYING COPYING.LGPL COPYING.FREEFONT COPYING.BSD COPYRIGHT
%attr(0755,root,root)%{_bindir}/scummvm
%{_datadir}/applications/*
%{_datadir}/pixmaps/scummvm.xpm
%{_datadir}/icons/hicolor/48x48/apps/scummvm.png
%{_datadir}/icons/hicolor/scalable/apps/scummvm.svg
%{_datadir}/scummvm/scumm*.zip
%{_datadir}/scummvm/translations.dat
%{_datadir}/scummvm/pred.dic
%{_datadir}/scummvm/kyra.dat
%{_datadir}/scummvm/queen.tbl
%{_datadir}/scummvm/sky.cpt
%{_datadir}/scummvm/lure.dat
%{_datadir}/scummvm/drascula.dat
%{_datadir}/scummvm/teenagent.dat
%{_datadir}/scummvm/hugo.dat
%{_datadir}/scummvm/tony.dat
%{_datadir}/scummvm/toon.dat
%{_mandir}/man6/scummvm.6*

