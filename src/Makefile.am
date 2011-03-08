## Process this file with automake to produce Makefile.in
AM_CFLAGS=$(HARM_CFLAGS)
AM_CPPFLAGS= -DLOCALEDIR=\"$(localedir)\" -DPACKAGE=\"$(PACKAGE)\"
bin_PROGRAMS=Harmonic
Harmonic_SOURCES=	\
	harmonic.c	\
	plotlinear0-1-0.c	\
	harmonic.h	\
	plotlinear0-1-0.h
Harmonic_LDADD=$(HARM_LIBS)
