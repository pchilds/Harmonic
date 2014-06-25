AC_PREREQ([2.63])
AC_INIT([Harmonic],[0.1.0],[pchilds@physics.org],[gtk-harmonic],[https://github.com/pchilds/Harmonic])
AC_CONFIG_HEADERS([config.h])
AC_CONFIG_SRCDIR([src/main.h])
AC_CONFIG_AUX_DIR([confsupp])
AC_CONFIG_MACRO_DIR([m4])
AM_INIT_AUTOMAKE([-Wall -Wno-portability -Wno-obsolete -Werror foreign])
AM_GNU_GETTEXT([external])
AC_ARG_ENABLE([gnomedoc],AS_HELP_STRING([--disable-gnomedoc],[Disable gnomedoc -incompatible with some packaging  @<:@yes/no@:>@]),,[enable_gnomedoc=yes])
AM_CONDITIONAL([ALLOW_GDOC], [test "$enable_gnomedoc" = "yes"])
AC_PROG_CC([gcc cc])
case "$host_os" in
   mingw*)
      AC_DEFINE([DLMT],["\r\n"],[End Of Line delimiter])
      ;;
   *)
      AC_DEFINE([DLMT],["\n"],[End Of Line delimiter])
      ;;
esac
gtk_rqr="gtk+-2.0 >= 2.14 gthread-2.0 gtkplot-2.0"
PKG_CHECK_MODULES([HARM], [$gtk_rqr fftw3])
if test "x$enable_gnomedoc" = "xyes"; then
    AC_DEFINE([USE_GDOC],[1],[Use Gnome Documentation])
    GNOME_DOC_INIT
    AC_CONFIG_FILES([doc/Makefile])
fi
AC_CONFIG_FILES([Makefile src/Makefile po/Makefile.in])
AC_OUTPUT()
