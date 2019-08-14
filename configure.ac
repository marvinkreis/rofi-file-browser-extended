AC_INIT([rofi-file-browser-extended], [1.1.0], [https://github.com/marvinkreis/rofi-file-browser-extended/issues], [], [https://github.com/marvinkreis/rofi-file-browser-extended])

AC_CONFIG_MACRO_DIRS([m4])

dnl ---------------------------------------------------------------------
dnl Setup automake to be silent and in foreign mode.
dnl We want xz distribution
dnl ---------------------------------------------------------------------
AM_INIT_AUTOMAKE([-Wall -Werror foreign subdir-objects dist-xz])
AM_SILENT_RULES([yes])

dnl ---------------------------------------------------------------------
dnl  Check for compiler
dnl ---------------------------------------------------------------------
AC_PROG_CC([clang gcc cc])

dnl ---------------------------------------------------------------------
dnl  C99 standard
dnl ---------------------------------------------------------------------
AC_PROG_CC_C99

dnl ---------------------------------------------------------------------
dnl C to Object rules.
dnl ---------------------------------------------------------------------
AM_PROG_CC_C_O

dnl ---------------------------------------------------------------------
dnl System extensions
dnl ---------------------------------------------------------------------
AC_USE_SYSTEM_EXTENSIONS

dnl ---------------------------------------------------------------------
dnl Static libraries programs
dnl ---------------------------------------------------------------------
AM_PROG_AR

dnl ---------------------------------------------------------------------
dnl Base CFLAGS
dnl ---------------------------------------------------------------------
AM_CFLAGS="-Wall -Wextra -Wparentheses -Winline -pedantic  -Wunreachable-code"

dnl ---------------------------------------------------------------------
dnl Check dependencies
dnl ---------------------------------------------------------------------
PKG_PROG_PKG_CONFIG

dnl ---------------------------------------------------------------------
dnl PKG_CONFIG based dependencies
dnl ---------------------------------------------------------------------
NK_INIT([bindings xdg-theme])
PKG_CHECK_MODULES([glib], [glib-2.0 >= 2.40 gio-unix-2.0 gmodule-2.0 ])
PKG_CHECK_MODULES([rofi], [rofi])
PKG_CHECK_MODULES([gtk],  [gtk+-3.0])

[rofi_PLUGIN_INSTALL_DIR]="`$PKG_CONFIG --variable=pluginsdir rofi`"
AC_SUBST([rofi_PLUGIN_INSTALL_DIR])

fb_CFLAGS="-I./include"
AC_SUBST(fb_CFLAGS)

LT_INIT([disable-static])

dnl ---------------------------------------------------------------------
dnl Add extra compiler flags
dnl ---------------------------------------------------------------------
AC_SUBST([AM_CFLAGS])

AC_CONFIG_FILES([Makefile])
AC_OUTPUT
