#include <gmodule.h>
#include <gio/gio.h>
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
    GArray *icon_names = g_array_new ( false, false, sizeof ( char * ) );

    GFile *file = NULL;
    GIcon *icon = NULL;
    char *icon_path = NULL;

    if ( fbfile->type == UP ) {
        g_array_append_val( icon_names, id->up_icon );

    } else if ( fbfile->type == INACCESSIBLE ) {
        g_array_append_val( icon_names, id->inaccessible_icon );

    } else if ( fbfile->path == NULL ) {
        g_array_append_val( icon_names, ERROR_ICON );

    } else {
        file = g_file_new_for_path ( fbfile->path );
        GFileInfo *file_info = g_file_query_info ( file, "standard::icon", G_FILE_QUERY_INFO_NONE, NULL, NULL );

        if ( file_info != NULL ) {
            icon = g_file_info_get_icon ( file_info );
            if ( G_IS_THEMED_ICON ( icon ) ) {
                g_themed_icon_append_name ( G_THEMED_ICON ( icon ), id->fallback_icon );
                char** local_icon_names = ( char ** ) g_themed_icon_get_names( G_THEMED_ICON ( icon ) );
                g_array_append_vals ( icon_names, local_icon_names, count_strv ( ( const char ** ) local_icon_names ) );
            }
        }

        if ( rofi_icon_fetcher_file_is_image( fbfile->path ) ) {
            g_array_prepend_val ( icon_names, fbfile->path );
        }
    }

    unsigned long num_icon_names;
    char** icon_names_raw = g_array_steal ( icon_names, &num_icon_names );

    /* Create icon fetcher requests. */
    fbfile->num_icon_fetcher_requests = num_icon_names;
    fbfile->icon_fetcher_requests = malloc ( sizeof ( uint32_t ) * num_icon_names );
    for ( int i = 0; i < num_icon_names; i++ ) {
        fbfile->icon_fetcher_requests[i] = rofi_icon_fetcher_query ( icon_names_raw[i], icon_size );
    }

    if ( file != NULL ) {
        g_object_unref ( file );
    }
    if ( icon != NULL ) {
        g_object_unref ( icon );
    }
    if ( icon_path != NULL ) {
        g_free ( icon_path );
    }
    g_array_unref ( icon_names );
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
