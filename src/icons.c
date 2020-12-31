#include <gmodule.h>
#include <cairo.h>
#include <gtk/gtk.h>
#include <rofi/rofi-icon-fetcher.h>

#include "defaults.h"
#include "types.h"
#include "icons.h"
#include "util.h"

void destroy_icon_data ( FileBrowserIconData *id ) {
    g_free ( id->up_icon );
    g_free ( id->inaccessible_icon );
    g_free ( id->fallback_icon );
}

void request_icons_for_file ( FBFile *fbfile, int icon_size, FileBrowserIconData *id )
{
    char *default_icon_names[] = { NULL, NULL };
    char **icon_names = NULL;
    GIcon *icon = NULL;

    if ( fbfile->type == UP ) {
        default_icon_names[0] = id->up_icon;
        icon_names = default_icon_names;

    } else if ( fbfile->type == INACCESSIBLE ) {
        default_icon_names[0] = id->inaccessible_icon;
        icon_names = default_icon_names;

    } else if ( fbfile->path == NULL ) {
        default_icon_names[0] = ERROR_ICON;
        icon_names = default_icon_names;

    } else {
        GFile *file = g_file_new_for_path ( fbfile->path );
        GFileInfo *file_info = g_file_query_info ( file, "standard::icon", G_FILE_QUERY_INFO_NONE, NULL, NULL );

        if ( file_info != NULL ) {
            icon = g_file_info_get_icon ( file_info );
            if ( G_IS_THEMED_ICON ( icon ) ) {
                g_themed_icon_append_name ( G_THEMED_ICON ( icon ), id->fallback_icon );
                icon_names = ( char ** ) g_themed_icon_get_names ( G_THEMED_ICON ( icon ) );
            }
        }

        g_object_unref ( file );

        if ( icon_names == NULL ) {
            default_icon_names[0] = ERROR_ICON;
            icon_names = default_icon_names;
        }
    }

    /* Create icon fetcher requests. */
    fbfile->num_icon_fetcher_requests = count_strv ( ( const char ** ) icon_names );
    fbfile->icon_fetcher_requests = malloc ( sizeof ( uint32_t ) * fbfile->num_icon_fetcher_requests );
    for (int i = 0; i < fbfile->num_icon_fetcher_requests; i++) {
        fbfile->icon_fetcher_requests[i] = rofi_icon_fetcher_query ( icon_names[i], icon_size );
    }

    if ( icon != NULL ) {
        g_object_unref ( icon );
    }
}

cairo_surface_t *fetch_icon_for_file ( FBFile *fbfile )
{
    for ( int i = 0; i < fbfile->num_icon_fetcher_requests; i++ ) {
        cairo_surface_t *icon = rofi_icon_fetcher_get ( fbfile->icon_fetcher_requests[i] );
        if ( icon != NULL ) {
            return icon;
        }
    }

    return NULL;
}
