#include <stdbool.h>
#include <gmodule.h>
#include <cairo.h>
#include <nkutils-xdg-theme.h>
#include <gtk/gtk.h>
#include <rofi/helper.h>

#include "config.h"
#include "types.h"
#include "util.h"

/**
 * Returns the most specific (i.e. the first existing) icon for an array of icon names and caches it.
 * The cairo surface is destroyed when destroy_icons is called.
 */
static cairo_surface_t *get_icon_for_names ( FileBrowserIconData *id, char **icon_names, int icon_size );

// ================================================================================================================= //

void init_icons ( FileBrowserIconData *id ) {
    static const char * const fallback_icon_themes[] = {
        FALLBACK_ICON_THEMES,
        NULL
    };
    id->xdg_context = nk_xdg_theme_context_new ( fallback_icon_themes, NULL );
    nk_xdg_theme_preload_themes_icon ( id->xdg_context, ( const gchar * const * ) id->icon_themes );
    id->icons = g_hash_table_new_full ( g_str_hash, g_str_equal, g_free, ( void (*) (void *) ) cairo_surface_destroy );
}

void destroy_icons ( FileBrowserIconData *id ) {
    if ( id->icons != NULL ) {
        g_hash_table_destroy ( id->icons );
    }
    if ( id->xdg_context != NULL ) {
        nk_xdg_theme_context_free ( id->xdg_context );
    }
    g_strfreev ( id->icon_themes );
    g_free ( id->up_icon );
    g_free ( id->inaccessible_icon );
    g_free ( id->fallback_icon );
    g_free ( id->error_icon );
}

cairo_surface_t *get_icon_for_file ( FBFile *fbfile, int icon_size, FileBrowserIconData *id )
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
        default_icon_names[0] = id->error_icon;
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
            default_icon_names[0] = id->error_icon;
            icon_names = default_icon_names;
        }
    }

    cairo_surface_t *icon_surf = get_icon_for_names( id, icon_names, icon_size );
    if ( icon_surf == NULL ) {
        print_err ( "Could not find an icon for file \"%s\".", fbfile->name );
    }

    if ( icon != NULL ) {
        g_object_unref ( icon );
    }

    return icon_surf;
}

static cairo_surface_t *get_icon_for_names ( FileBrowserIconData *id, char **icon_names, int icon_size )
{
    for (int i = 0; icon_names[i] != NULL; i++) {
        cairo_surface_t *icon_surf = g_hash_table_lookup ( id->icons, icon_names[i] );

        if ( icon_surf != NULL ) {
            return icon_surf;
        }

        char *icon_path = nk_xdg_theme_get_icon ( id->xdg_context, ( const char ** ) id->icon_themes, NULL,
                                                  icon_names[i], icon_size, 1, true );

        if ( icon_path == NULL ) {
            continue;
        }

        if ( g_str_has_suffix ( icon_path, ".png" ) ) {
            icon_surf = cairo_image_surface_create_from_png ( icon_path );
        } else if ( g_str_has_suffix ( icon_path, ".svg" ) ) {
            icon_surf = cairo_image_surface_create_from_svg ( icon_path, icon_size );
        }

        g_free ( icon_path );

        if ( icon_surf != NULL ) {
            if ( cairo_surface_status ( icon_surf ) != CAIRO_STATUS_SUCCESS ) {
                cairo_surface_destroy ( icon_surf );
            } else {
                g_hash_table_insert ( id->icons, g_strdup ( icon_names[i] ), icon_surf );
                return icon_surf;
            }
        }
    }

    return NULL;
}

char **get_default_icon_theme ( void )
{
    char *default_theme = NULL;
    gtk_init ( NULL, NULL );
    g_object_get ( gtk_settings_get_default (), "gtk-icon-theme-name", &default_theme, NULL );

    if ( default_theme == NULL ) {
        print_err ( "Could not determine GTK icon theme. Maybe try setting a theme with \"-file-browser-theme\".\n" );
    }

    char *icon_themes[] = {
        default_theme,
        NULL
    };

    return g_strdupv ( icon_themes );
}
