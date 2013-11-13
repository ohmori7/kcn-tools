# To build rpm: rpmbuild -bb elinks.spec
# Debug version: rpmbuild -bb elinks.spec --debug
# Console version without X11 dependency: rpmbuild -bb elinks.spec --without x
# by default all features are enabled
# Conditional builds:
# --without 256 # build without support for 256 colors on xterm
# --without bzlib
# --without cgi # build without local cgi support
# --without gpm # build without GPM support
# --without leds # disable leds support
# --without lua # build without lua scripting engine
# --without openssl
# --without smb # build without support for SMB protocol
# --without x # without X11 dependency (X11 libs are used only for restoring title
# in xterm)
# --without zlib #
# --with guile # build with guile scripting engine
# --with perl # build with Perl scripting engine
#
# This file was contributed by <zimon@niksula.hut.fi> and
# <yanek@yankuv.koniec.sk>.
#
# Just try to send the patch/new version to them and wait few days if they will
# do anything. If yes, they'll approve it or forward it to me (don't fear, the
# credit won't be lost) if it's ok by them. If you won't get any reply, send it
# to the elinks-dev mailing list. Thanks! --pasky
#
#
# Spec file format from < http://www.rpm.org/RPM-HOWTO/build.html >
#

Summary:	Enhanced version of Links (Lynx-like text WWW browser)
Name:		elinks
Version:	0.11.7
Release:	1
License:	GPL
Vendor:		ELinks project <elinks-users@linuxfromscratch.org>
Packager:	Petr Baudis <pasky@ucw.cz>
Group:		Applications/Internet
Source:		http://elinks.cz/download/%{name}-%{version}.tar.bz2
URL:		http://elinks.cz/
BuildRequires:	bzip2-devel
BuildRequires:	expat-devel
BuildRequires:	gpm-devel
BuildRequires:	openssl-devel
BuildRequires:	zlib-devel
%{!?_without_x:BuildRequires:	XFree86-devel}
BuildRoot:	%{_tmppath}/%{name}-%{version}-build

%description
Enhanced version of Links (Lynx-like text WWW browser), with more liberal
features policy and development style.

ELinks aims to provide feature-rich version of Links.  Its purpuose is to make
alternative to Links, and to test and tune various new features, but still
provide good rock-solid releases inside stable branches.

%prep
%setup -q

%build

#
# if you just checkout the source and make a tarball from it uncomment
# the following line
#
# ./autogen.sh

%configure \
%{?debug:	--enable-debug} \
%{!?debug:	--enable-fastmem} \
%{?_without_x: --without-x} \
%{?_without_lua: --without-lua} \
%{?_without_zlib: --without-zlib} \
%{?_without_bzlib: --without-bzlib} \
%{?_without_gpm: --without-gpm} \
%{?_without_openssl: --without-openssl} \
%{?_with_guile: --with-guile} \
%{?_with_perl: --with-perl} \
%{?_without_lua: --without-lua} \
%{!?_without_leds: --enable-leds} \
%{!?_without_256:	--enable-256-colors} \
%{?_without_smb: --disable-smb} \
%{!?_without_cgi: --enable-cgi} \
	--enable-exmode \
	--enable-html-highlight \
	--enable-gopher \
	--enable-finger \
	--enable-nntp \
%{?_without_spidermonkey: --without-spidermonkey}


%{__make}


%install
rm -rf $RPM_BUILD_ROOT
%{__make} install DESTDIR=$RPM_BUILD_ROOT
install -d $RPM_BUILD_ROOT/%{_defaultdocdir}/%{name}/{contrib,conv,lua,guile,perl}
install \
	AUTHORS* BUGS* ChangeLog* COPYING* README* SITES* TODO* THANKS* \
	INSTALL* NEWS* $RPM_BUILD_ROOT/%{_defaultdocdir}/%{name}/
install \
	contrib/elinks.vim contrib/elinks-vim.diff contrib/keybind*.conf \
	$RPM_BUILD_ROOT/%{_defaultdocdir}/%{name}/contrib/
install \
	contrib/conv/w3m2links.awk contrib/conv/conf-links2elinks.pl \
	contrib/conv/mailcap.pl contrib/conv/old_to_new_bookmarks.sh \
	$RPM_BUILD_ROOT/%{_defaultdocdir}/%{name}/conv/
install \
	contrib/lua/elinks-remote contrib/lua/*.lua \
	$RPM_BUILD_ROOT/%{_defaultdocdir}/%{name}/lua/
install \
	contrib/guile/*scm \
	$RPM_BUILD_ROOT/%{_defaultdocdir}/%{name}/guile/
install \
	contrib/perl/hooks.pl \
	$RPM_BUILD_ROOT/%{_defaultdocdir}/%{name}/perl
install \
	doc/*.txt \
	$RPM_BUILD_ROOT/%{_defaultdocdir}/%{name}/

%find_lang %{name}

%clean
rm -rf $RPM_BUILD_ROOT

%files -f %{name}.lang
%defattr(644,root,root,755)
%attr (755,root,root) %{_bindir}/%{name}
%doc %{_defaultdocdir}/%{name}
%doc %{_mandir}/man?/*

# date +"%a %b %d %Y"
%changelog
* Fri Feb 17 2006 Witold Filipczyk <witekfl@pld-linux.org>
- typo

* Thu Dec 29 2005 Miciah Dashiel Butler Masters <mdm0304@ecu.edu>
- elinks.or.cz -> elinks.cz

* Tue Jun 14 2005 Witold Filipczyk <witekfl@pld-linux.org>
- removed unused texi2html dependency
- removed unused libdir directory

* Mon Sep 27 2004 Witold Filipczyk <witekfl@pld-linux.org>
- by default build feature rich ELinks

* Thu Mar 25 2004 Witold Filipczyk <witekfl@pld-linux.org>
- a lot of bconds
- added hooks.pl

* Wed Dec 10 2003 Witold Filipczyk <witekfl@pld-linux.org>
- enabled leds and local-cgi

* Sun Oct 26 2003 Witold Filipczyk <witekfl@pld-linux.org>
- revert to version from 5 October

* Sat Oct 25 2003 Witold Filipczyk <witekfl@pld-linux.org>
- more BRs. gettext should be enough for most cases, but
  gettext-devel shouldn't hurt.

* Sun Oct 05 2003 Witold Filipczyk <witekfl@pld-linux.org>
- enabled 256 colors

* Thu Oct 02 2003 Witold Filipczyk <witekfl@pld-linux.org>
- polished
- no sanity checks

* Sat Sep 27 2003 darix@irssi.org
- added license
- changed BuildRoot to use tmppath variable.
- removed prefix
- added some more docs and the guile scripts

* Sat Sep 27 2003 Petr Baudis <pasky@ucw.cz>
- sanity checks of $RPM_BUILD_ROOT before rm -rf'ing it, based on darix's
  suggestion

* Sun Jan 26 2003 Petr Baudis <pasky@ucw.cz>
- elinks.pld.org.pl -> elinks.or.cz, based on Bennett's suggestion

* Thu Dec 19 2002 Bennett Todd <bet@rahul.net>
- wildcarded the above doc and manpage specifications, and tagged the man
  pages as docs

* Mon May 06 2002 yanek@yankuv.koniec.sk
- general update - cleanup of .spec file, massive simplifications

* Thu Apr 04 2002 pasky@ucw.cz
- Changed some stuff so that it's now ready for inclusion..

* Sat Mar 16 2002 zimon (�t) iki fi
- Made my own elinks.spec file as the one I found with Google didn't work how
  I wanted

