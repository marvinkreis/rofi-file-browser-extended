ACLOCAL_AMFLAGS=-I m4 -I ../submodules/libnkutils
plugindir=@rofi_PLUGIN_INSTALL_DIR@

noinst_LTLIBRARIES =
noinst_PROGRAMS=
EXTRA_DIST=
check_PROGRAMS=
TESTS=
include $(top_srcdir)/../submodules/libnkutils/libnkutils.mk

plugin_LTLIBRARIES = file_browser.la

file_browser_la_SOURCES=\
		 src/filebrowser.c

file_browser_la_CFLAGS= @glib_CFLAGS@ @rofi_CFLAGS@ $(NKUTILS_CFLAGS)
file_browser_la_LIBADD= @glib_LIBS@ @rofi_LIBS@ $(NKUTILS_LIBS)
file_browser_la_LDFLAGS= -module -avoid-version
