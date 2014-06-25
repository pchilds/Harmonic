## Process this file with automake to produce Makefile.in
SUBDIRS= doc po src
Applicationsdir=$(datadir)/applications
Applications_DATA=harmonic.desktop
pixmapdir=$(datadir)/pixmaps
pixmap_DATA=pixmaps/db8.png
EXTRA_DIST= \
	ABOUT-NLS confsupp/config.rpath m4/ChangeLog \
	AUTHORS ChangeLog COPYING COPYING-DOCS INSTALL NEWS README TODO \
	$(pixmap_DATA) $(Applications_DATA) \
	configure.ac
if ALLOW_GDOC
EXTRA_DIST+=m4 confsupp/gnome-doc-utils.make
DISTCLEANFILES=confsupp/gnome-doc-utils.make
endif
ACLOCAL_AMFLAGS=-I m4
MAINTAINERCLEANFILES=Makefile.in configure
