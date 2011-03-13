## Process this file with automake to produce Makefile.in
SUBDIRS= doc po src
Applicationsdir=$(datadir)/applications
Applications_DATA=harmonic.desktop
pixmapdir=$(datadir)/pixmaps
pixmap_DATA=pixmaps/db8.png
EXTRA_DIST=confsupp/gnome-doc-utils.make \
	ABOUT-NLS confsupp/config.rpath m4/ChangeLog \
	AUTHORS ChangeLog COPYING INSTALL NEWS README TODO \
	$(pixmap_DATA) \
	$(Applications_DATA) \
	configure.ac

ACLOCAL_AMFLAGS = -I m4
