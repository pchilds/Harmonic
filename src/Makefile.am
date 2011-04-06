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
	open.c			\
	open.h			\
	plotlinear0-1-0.c	\
	plotlinear0-1-0.h	\
	plotpolar0-1-0.c	\
	plotpolar0-1-0.h
Harmonic_LDADD=$(HARM_LIBS)
