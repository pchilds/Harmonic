AC_PREREQ([2.65])
AC_INIT([Harmonic],[0.1.0],[pchilds@physics.org],[gtk-harmonic],[https://github.com/pchilds/Harmonic])
AC_CONFIG_SRCDIR([src/harmonic.h])
AC_CONFIG_AUX_DIR([confsupp])
AC_CONFIG_MACRO_DIR([m4])
AM_INIT_AUTOMAKE([-Wall -Werror foreign])
AM_GNU_GETTEXT([external])
GNOME_DOC_INIT
AC_PROG_CC([gcc cc])
AC_CONFIG_HEADERS([config.h])
gtk_rqr="gtk+-2.0 >= 2.14"
PKG_CHECK_MODULES([HARM], [$gtk_rqr fftw3])
AC_CONFIG_FILES([Makefile src/Makefile doc/Makefile po/Makefile.in])
AC_OUTPUT()
