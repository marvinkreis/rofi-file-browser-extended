ACLOCAL_AMFLAGS=-I m4 -I ../submodules/libnkutils
plugindir=@rofi_PLUGIN_INSTALL_DIR@

noinst_LTLIBRARIES =
noinst_PROGRAMS=
EXTRA_DIST=
check_PROGRAMS=
TESTS=
include $(top_srcdir)/../submodules/libnkutils/libnkutils.mk

plugin_LTLIBRARIES = prompt.la

prompt_la_SOURCES=\
		 src/prompt.c

prompt_la_CFLAGS= @glib_CFLAGS@ @rofi_CFLAGS@ @jsonc_CFLAGS@ $(NKUTILS_CFLAGS)
prompt_la_LIBADD= @glib_LIBS@ @rofi_LIBS@ @jsonc_LIBS@ $(NKUTILS_LIBS)
prompt_la_LDFLAGS= -module -avoid-version
