## Process this file with automake to produce Makefile.in
AM_CFLAGS=$(HARM_CFLAGS)
AM_CPPFLAGS= -DLOCALEDIR=\"$(localedir)\" -DPACKAGE=\"$(PACKAGE)\"
bin_PROGRAMS=Harmonic
Harmonic_SOURCES=		\
	main.c			\
	main.h			\
	util.c			\
	util.h			\
	data.c			\
	data.h			\
	proc.c			\
	proc.h			\
	disp.c			\
	disp.h			\
	batc.c			\
	batc.h
Harmonic_LDADD=$(HARM_LIBS)
MAINTAINERCLEANFILES=Makefile.in
