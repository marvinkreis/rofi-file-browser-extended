ACLOCAL_AMFLAGS=-I m4
plugindir=@rofi_PLUGIN_INSTALL_DIR@

plugin_LTLIBRARIES = dict.la

dict_la_SOURCES=\
		 src/dict.c

dict_la_CFLAGS= @glib_CFLAGS@ @rofi_CFLAGS@ @jsonc_CFLAGS@
dict_la_LIBADD= @glib_LIBS@ @rofi_LIBS@ @jsonc_LIBS@
dict_la_LDFLAGS= -module -avoid-version
