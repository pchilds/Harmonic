## Process this file with automake to produce Makefile.in
SUBDIRS= doc po src
Applicationsdir=$(datadir)/applications
Applications_DATA=harmonic.desktop
pixmapdir=$(datadir)/pixmaps
pixmap_DATA=pixmaps/db8.png
EXTRA_DIST=confsupp/gnome-doc-utils.make \
	ABOUT-NLS confsupp/config.rpath m4/ChangeLog \
	AUTHORS ChangeLog COPYING COPYING-DOCS INSTALL NEWS README TODO \
	$(pixmap_DATA) $(Applications_DATA) \
	configure.ac
DISTCHECK_CONFIGURE_FLAGS=--disable-scrollkeeper
ACLOCAL_AMFLAGS=-I m4
MAINTAINERCLEANFILES=Makefile.in configure
