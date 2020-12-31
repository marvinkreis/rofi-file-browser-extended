#ifndef FILE_BROWSER_ICONS_H
#define FILE_BROWSER_ICONS_H

#include <cairo.h>

#include "types.h"

/**
 * Destroys the icon data.
 */
void destroy_icon_data ( FileBrowserIconData *id );

/**
 * Requests icons for the file from rofi's icon fetcher.
 */
void request_icons_for_file ( FBFile *fbfile, int icon_size, FileBrowserIconData *id );

/**
 * Fetches requested icons for the file from rofi's icon fetcher.
 */
cairo_surface_t *fetch_icon_for_file ( FBFile *fbfile );

#endif
