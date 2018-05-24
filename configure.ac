AC_INIT([Guile Gobject Introspection], [0.1], [steve@ayerh.art], [guile-gobject-introspection])
AC_CONFIG_SRCDIR([modules/pre-install-env.in])
AC_CONFIG_MACRO_DIRS([m4])
AC_CONFIG_AUX_DIR([build-aux])

AM_INIT_AUTOMAKE

AC_PROG_CC

dnl libtool
AC_LIBTOOL_DLOPEN
AC_DISABLE_STATIC
AC_PROG_LIBTOOL

dnl guile
GUILE_PKG([2.2])
GUILE_PROGS
GUILE_FLAGS
GUILE_SITE_DIR
AC_SUBST(GUILE_LIBS)
AC_SUBST(GUILE_CFLAGS)

dnl introspection
PKG_CHECK_MODULES(GIR, gobject-introspection-1.0 >= 0.6.5)
AC_SUBST(GIR_LIBS)
AC_SUBST(GIR_CFLAGS)

AC_CONFIG_FILES([Makefile modules/Makefile src/Makefile])
AC_CONFIG_FILES([modules/pre-install-env], [chmod +x modules/pre-install-env])

AC_OUTPUT
