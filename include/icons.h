#ifndef FILE_BROWSER_ICONS_H
#define FILE_BROWSER_ICONS_H

#include <cairo.h>

#include "types.h"

void destroy_icon_data ( FileBrowserIconData *id );
void request_icons_for_file ( FBFile *fbfile, int icon_size, FileBrowserIconData *id );
cairo_surface_t *fetch_icon_for_file ( FBFile *fbfile );

#endif
