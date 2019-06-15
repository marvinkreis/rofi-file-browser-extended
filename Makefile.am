ACLOCAL_AMFLAGS=-I m4 -I submodules/libnkutils
plugindir=@rofi_PLUGIN_INSTALL_DIR@

noinst_LTLIBRARIES =
noinst_PROGRAMS=
EXTRA_DIST=
check_PROGRAMS=
TESTS=
include submodules/libnkutils/libnkutils.mk

plugin_LTLIBRARIES = filebrowser.la

filebrowser_la_SOURCES=\
		 src/filebrowser.c

filebrowser_la_CFLAGS= @glib_CFLAGS@ @rofi_CFLAGS@ $(NKUTILS_CFLAGS) @gtk_CFLAGS@
filebrowser_la_LIBADD= @glib_LIBS@ @rofi_LIBS@ $(NKUTILS_LIBS) @gtk_LIBS@
filebrowser_la_LDFLAGS= -module -avoid-version
