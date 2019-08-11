#ifndef FILE_BROWSER_ICONS_H
#define FILE_BROWSER_ICONS_H

#include <cairo.h>

#include "types.h"

/**
 * Initializes the icons (i.e. the theme context and icon hashtable) from the set theme names.
 */
void init_icons ( FileBrowserIconData *id );

/**
 * Frees the icons (i.e. the theme context and icon hashtable).
 */
void destroy_icons ( FileBrowserIconData *id );

/**
 * Gets the most specific icon for a file, and caches it.
 * The cairo surface is destroyed when destroy_icons is called.
 */
cairo_surface_t *get_icon_for_file ( FBFile *fbfile, int icon_size, FileBrowserIconData *id );

/**
 * Gets the most specific (i.e. the first existing) icon for an array of icon names and caches it.
 * The cairo surface is destroyed when destroy_icons is called.
 */
cairo_surface_t *get_icon_for_names ( char **icon_names, int icon_size, FileBrowserIconData *id );

/**
 * Returns the default GTK icon theme name in a newly allocated array.
 * Prints an error message and returns an array of NULL if the default icon theme could not be determined.
 */
char **get_default_icon_theme ( void );

#endif
