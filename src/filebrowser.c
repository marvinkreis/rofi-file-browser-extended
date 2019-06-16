#define _XOPEN_SOURCE 700
#define _GNU_SOURCE

#include <stdio.h>

#include <rofi/mode.h>
#include <rofi/helper.h>
#include <rofi/mode-private.h>

#include <ftw.h>
#include <gmodule.h>

#include <nkutils-xdg-theme.h>
#include <gtk/gtk.h>

#include "config.h"
#include "types.h"
#include "util.h"
#include "options.h"
#include "filebrowser.h"

G_MODULE_EXPORT Mode mode;

/**
 * Save private data globally for nftw.
 */
static FileBrowserModePrivateData* global_pd;

// ================================================================================================================= //

static int file_browser_init ( Mode *sw )
{
    if ( mode_get_private_data ( sw ) == NULL ) {
        FileBrowserModePrivateData* pd = g_malloc0 ( sizeof ( * pd ) );
        mode_set_private_data ( sw, ( void * ) pd );

        pd->open_custom = false;
        pd->open_custom_index = -1;
        pd->files = NULL;
        pd->num_files = 0;
        pd->icons = NULL;
        pd->xdg_context = NULL;

        if ( ! set_command_line_options ( pd ) ) {
            return false;
        }

        /* Set up icons if enabled. */
        if ( pd->show_icons ) {
            static const char * const fallback_icon_themes[] = {
                FALLBACK_ICON_THEMES, NULL
            };
            pd->xdg_context = nk_xdg_theme_context_new ( fallback_icon_themes, NULL );
            nk_xdg_theme_preload_themes_icon ( pd->xdg_context, ( const gchar * const * ) pd->icon_themes );
            pd->icons = g_hash_table_new_full ( g_str_hash, g_str_equal, g_free,
                    ( void ( * ) ( void * ) ) cairo_surface_destroy );
        }

        /* Load the files. */
        change_dir ( pd->current_dir, pd );
    }

    return true;
}

static void file_browser_destroy ( Mode *sw )
{
    FileBrowserModePrivateData *pd = ( FileBrowserModePrivateData * ) mode_get_private_data ( sw );
    mode_set_private_data ( sw, NULL );

    if ( pd != NULL ) {
        /* Free file list. */
        free_files ( pd );

        /* Free icon themes and icons. */
        if ( pd->icons != NULL ) {
            g_hash_table_destroy ( pd->icons );
        }
        if ( pd->xdg_context != NULL ) {
            nk_xdg_theme_context_free ( pd->xdg_context );
        }
        g_strfreev ( pd->icon_themes );

        /* Free the rest. */
        g_free ( pd->current_dir );
        g_free ( pd->cmd );
        g_free ( pd->up_text );
        g_free ( pd->show_hidden_symbol );
        g_free ( pd->hide_hidden_symbol );
        g_free ( pd->path_sep );
        g_free ( pd->up_icon );
        g_free ( pd->inaccessible_icon );
        g_free ( pd->fallback_icon );
        g_free ( pd->error_icon );

        /* Fill with zeros, just in case. */
        memset ( ( void * ) pd , 0, sizeof ( pd ) );

        g_free ( pd );
    }
}

static unsigned int file_browser_get_num_entries ( const Mode *sw )
{
    const FileBrowserModePrivateData *pd = ( const FileBrowserModePrivateData * ) mode_get_private_data ( sw );

    if ( pd != NULL ) {
        if ( pd->open_custom ) {
            return 1;
        } else {
            return pd->num_files;
        }
    } else {
        return 0;
    }
}

static ModeMode file_browser_result ( Mode *sw, int mretv, char **input, unsigned int selected_line )
{
    FileBrowserModePrivateData *pd = ( FileBrowserModePrivateData * ) mode_get_private_data ( sw );

    ModeMode retv = RELOAD_DIALOG;

    /* Check if one of the custom key bindings pressed. */
    FBKey key = -1;
    if ( mretv & MENU_CUSTOM_ACTION ) {
        key = KB_ACCEPT_ALT;
    } else if ( mretv & MENU_QUICK_SWITCH ) {
        key = mretv & MENU_LOWER_MASK;
    }

    /* Handle prompt for program to open file with. */
    if ( pd->open_custom ) {
        if ( mretv & ( MENU_OK | MENU_CUSTOM_INPUT | MENU_CUSTOM_ACTION ) ) {
            if ( strlen ( *input ) == 0 ) {
                char *file_path = pd->files[pd->open_custom_index].path;
                open_file ( file_path, pd );
                retv = MODE_EXIT;
            } else {
                char* file_path = pd->files[pd->open_custom_index].path;
                g_free ( pd->cmd );
                pd->cmd = g_strdup ( *input );
                open_file ( file_path, pd );
                retv = MODE_EXIT;
            }
        } else if ( mretv & MENU_CANCEL ) {
            pd->open_custom = false;
            pd->open_custom_index = -1;
            retv = RESET_DIALOG;
        }

    /* Handle Shift+Return. */
    } else if ( key == pd->open_custom_key && selected_line != -1 ) {
        pd->open_custom = true;
        pd->open_custom_index = selected_line;
        retv = RESET_DIALOG;

    /* Handle Return. */
    } else if ( key == pd->open_multi_key || mretv & MENU_OK ) {
        FBFile* entry = &( pd->files[selected_line] );
        switch ( entry->type ) {
        case UP:
        case DIRECTORY:
            change_dir ( entry->path, pd );
            retv = RESET_DIALOG;
            break;
        case RFILE:
        case INACCESSIBLE:
            open_file ( entry->path, pd );
            if ( key != pd->open_multi_key ) {
                retv = MODE_EXIT;
            }
            break;
        }

    /* Handle custom input or Control+Return. */
    } else if ( mretv & MENU_CUSTOM_INPUT ) {
        if ( strlen ( *input ) > 0 ) {
            char *expanded_input = rofi_expand_path ( *input );
            char *file = g_filename_from_utf8 ( expanded_input, -1, NULL, NULL, NULL );
            g_free ( expanded_input );

            char *abs_path = get_existing_abs_path ( file, pd->current_dir );
            g_free ( file );

            if ( abs_path == NULL ) {
                retv = RELOAD_DIALOG;
            } else if ( g_file_test ( abs_path, G_FILE_TEST_IS_DIR ) ){
                change_dir ( abs_path, pd );
                retv = RESET_DIALOG;
            } else if ( g_file_test ( abs_path, G_FILE_TEST_IS_REGULAR ) ) {
                open_file ( abs_path, pd );
                retv = MODE_EXIT;
            }

            g_free ( abs_path );
        }

    /* Toggle hidden files with toggle_hidden_key. */
    } else if ( key == pd->toggle_hidden_key ) {
        pd->show_hidden = ! pd->show_hidden;
        load_files ( pd );
        retv = RELOAD_DIALOG;

    /* Enable hidden files with Shift+Right. */
    } else if ( pd->use_mode_keys && ( mretv & MENU_NEXT ) && !pd->show_hidden ) {
        pd->show_hidden = true;
        load_files ( pd );
        retv = RELOAD_DIALOG;

    /* Disable hidden files with Shift+Left. */
    } else if ( pd->use_mode_keys && ( mretv & MENU_PREVIOUS ) && pd->show_hidden ) {
        pd->show_hidden = false;
        load_files ( pd );
        retv = RELOAD_DIALOG;

    /* Default actions */
    } else if ( mretv & MENU_CANCEL ) {
        retv = MODE_EXIT;
    } else if ( mretv & MENU_NEXT ) {
        retv = NEXT_DIALOG;
    } else if ( mretv & MENU_PREVIOUS ) {
        retv = PREVIOUS_DIALOG;
    } else if ( mretv & MENU_QUICK_SWITCH ) {
        retv = ( mretv & MENU_LOWER_MASK );
    }

    return retv;
}

static int file_browser_token_match ( const Mode *sw, rofi_int_matcher **tokens, unsigned int index )
{
    FileBrowserModePrivateData *pd = ( FileBrowserModePrivateData * ) mode_get_private_data ( sw );

    if ( pd->open_custom ) {
        return true;
    } else {
        return helper_token_match ( tokens, pd->files[index].name );
    }
}

static char *file_browser_get_display_value ( const Mode *sw, unsigned int selected_line, G_GNUC_UNUSED int *state,
        G_GNUC_UNUSED GList **attr_list, int get_entry )
{
    FileBrowserModePrivateData *pd = ( FileBrowserModePrivateData * ) mode_get_private_data ( sw );

    if ( !get_entry ) return NULL;

    int index;
    if ( pd->open_custom ) {
        index = pd->open_custom_index;
    } else {
        index = selected_line;
    }

    return g_strdup ( pd->files[index].name );
}

static cairo_surface_t *file_browser_get_icon ( const Mode *sw, unsigned int selected_line, int height )
{
    FileBrowserModePrivateData *pd = ( FileBrowserModePrivateData * ) mode_get_private_data ( sw );

    int index;
    if ( pd->open_custom ) {
        index = pd->open_custom_index;
    } else {
        index = selected_line;
    }

    if ( pd->show_icons ) {
        cairo_surface_t *icon =  get_icon_surf ( pd->files[index], height, pd );
        return icon;
    } else {
        return NULL;
    }
}

static char *file_browser_get_message ( const Mode *sw )
{
    FileBrowserModePrivateData *pd = ( FileBrowserModePrivateData * ) mode_get_private_data ( sw );

    if ( pd->open_custom ) {
        char* file_name = pd->files[pd->open_custom_index].name;
        char* message = g_strdup_printf ( OPEN_CUSTOM_MESSAGE_FORMAT, file_name );
        return message;

    } else if ( pd->show_status ) {
        char** split = g_strsplit ( pd->current_dir, G_DIR_SEPARATOR_S, -1 );
        char* join = g_strjoinv ( pd->path_sep, split );
        char* message = g_strconcat ( pd->show_hidden ? pd->show_hidden_symbol : pd->hide_hidden_symbol, join, NULL );

        g_strfreev ( split );
        g_free ( join );

        return message;

    } else {
        return NULL;
    }
}

// ================================================================================================================= //

void free_files ( FileBrowserModePrivateData *pd )
{
    for ( unsigned int i = 0; i < pd->num_files; i++ ) {
        FBFile *fb = & ( pd->files[i] );
        g_free ( fb->name );
        g_free ( fb->path );
    }
    g_free ( pd->files );
    pd->files  = NULL;
    pd->num_files = 0;
}

int add_file ( const char *fpath, const struct stat *sb, int typeflag, struct FTW *ftwbuf )
{
    FileBrowserModePrivateData *pd = global_pd;

    /* Skip hidden files. */
    if ( ! pd->show_hidden && fpath[ftwbuf->base] == '.' ) {
        return FTW_SKIP_SUBTREE;

    /* Skip the current dir itself. */
    } else if ( ftwbuf->level == 0 ) {
        return FTW_CONTINUE;
    }

    int pos = strlen ( fpath ) - 1;
    int level = ftwbuf->level;
    while ( level > 0 ) {
        pos--;
        if ( fpath[pos] == G_DIR_SEPARATOR ) {
            level--;
        }
    }
    pos++;

    FBFile fbfile;
    fbfile.path = g_strdup ( fpath );
    fbfile.name = g_filename_to_utf8 ( &fpath[pos], -1, NULL, NULL, NULL);

    switch ( typeflag ) {
    case FTW_F:
        fbfile.type = RFILE;
        break;
    case FTW_D:
        fbfile.type = DIRECTORY;
        break;
    case FTW_DNR:
        fbfile.type = INACCESSIBLE;
        break;
    }

    pd->files = g_realloc ( pd->files, ( pd->num_files + 1 ) * sizeof ( FBFile ) );
    pd->files[pd->num_files] = fbfile;
    pd->num_files++;

    if ( pd->depth == 0 || ( ftwbuf->level < global_pd->depth ) ) {
        return FTW_CONTINUE;
    } else {
        return FTW_SKIP_SUBTREE;
    }
}

void load_files ( FileBrowserModePrivateData *pd )
{
    global_pd = pd;

    free_files ( pd );
    pd->files = g_realloc ( pd->files, ( pd->num_files + 1 ) * sizeof ( FBFile ) );

    FBFile up;
    up.type = UP;
    up.name = g_strdup ( pd->up_text );
    up.path = g_build_filename ( pd->current_dir, "..", NULL );
    pd->files[pd->num_files] = up;
    pd->num_files++;

    nftw ( pd->current_dir, add_file, 16, FTW_ACTIONRETVAL );

    /* Sort all but the first entry ("go-up"). */
    if ( pd->sort_by_type ) {
        g_qsort_with_data ( &( pd->files[1] ), pd->num_files - 1, sizeof ( FBFile ), compare_files_type, NULL );
    } else {
        g_qsort_with_data ( &( pd->files[1] ), pd->num_files - 1, sizeof ( FBFile ), compare_files_no_type, NULL );
    }
}

void change_dir ( char *path, FileBrowserModePrivateData *pd )
{
    char* new_dir = get_existing_abs_path ( path, pd->current_dir );
    if ( new_dir == NULL ) {
        return;
    }
    g_free ( pd->current_dir );
    pd->current_dir = new_dir;
    load_files ( pd );
}

gint compare_files_type ( gconstpointer a, gconstpointer b, gpointer data )
{
    FBFile *fa = ( FBFile * ) a;
    FBFile *fb = ( FBFile * ) b;
    if ( fa->type != fb->type ){
        return fa->type - fb->type;
    }
    return g_strcmp0 ( fa->name, fb->name );
}

gint compare_files_no_type ( gconstpointer a, gconstpointer b, gpointer data )
{
    FBFile *fa = ( FBFile * ) a;
    FBFile *fb = ( FBFile * ) b;
    return g_strcmp0 ( fa->name, fb->name );
}

cairo_surface_t *get_icon_surf ( FBFile fbfile, int icon_size, FileBrowserModePrivateData *pd )
{
    char *default_icon_names[] = { NULL, NULL };
    char **icon_names = NULL;
    GIcon *icon = NULL;
    cairo_surface_t *icon_surf = NULL;

    /* Get icon names for the file. */
    if ( fbfile.type == UP ) {
        default_icon_names[0] = pd->up_icon;
        icon_names = default_icon_names;
    } else if ( fbfile.type == INACCESSIBLE ) {
        default_icon_names[0] = pd->inaccessible_icon;
        icon_names = default_icon_names;
    } else if ( fbfile.path == NULL ) {
        default_icon_names[0] = pd->error_icon;
        icon_names = default_icon_names;
    } else {
        GFile *file = g_file_new_for_path ( fbfile.path );
        GFileInfo *file_info = g_file_query_info ( file, "standard::icon", G_FILE_QUERY_INFO_NONE, NULL, NULL );

        if ( file_info != NULL ) {
            icon = g_file_info_get_icon ( file_info );
            if ( G_IS_THEMED_ICON ( icon ) ) {
                g_themed_icon_append_name ( G_THEMED_ICON ( icon ), pd->fallback_icon );
                icon_names = ( char ** ) g_themed_icon_get_names ( G_THEMED_ICON ( icon ) );
            }
        }

        g_object_unref ( file );

        if ( icon_names == NULL ) {
            default_icon_names[0] = pd->error_icon;
            icon_names = default_icon_names;
        }
    }

    /* Get icon for the icon names. */
    for (int i = 0; icon_names[i] != NULL; i++) {
        icon_surf = g_hash_table_lookup ( pd->icons, icon_names[i] );

        if ( icon_surf != NULL ) {
            break;
        }

        char *icon_path = nk_xdg_theme_get_icon ( pd->xdg_context, ( const char ** ) pd->icon_themes, NULL,
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
                icon_surf = NULL;
            } else {
                g_hash_table_insert ( pd->icons, g_strdup ( icon_names[i] ), icon_surf );
                break;
            }
        }
    }

    if ( icon != NULL ) {
        g_object_unref ( icon );
    }

    return icon_surf;
}

void open_file ( char *path, FileBrowserModePrivateData *pd )
{
    if ( pd->dmenu ) {
        printf("%s\n", path);

    } else {
        /* Escape the file path. */
        char **split = g_strsplit ( path, "\"", -1 );
        path = g_strjoinv ( "\\\"", split );
        g_strfreev ( split );

        /* Construct the command. */
        char* complete_cmd = NULL;
        if ( g_strrstr ( pd->cmd, "%s" ) != NULL ) {
            complete_cmd = g_strdup_printf ( pd->cmd, path );
        } else {
            complete_cmd = g_strconcat ( pd->cmd, " \"", path, "\"", NULL );
        }

        helper_execute_command ( pd->current_dir, complete_cmd, false, NULL );

        g_free ( complete_cmd );
    }
}

// ================================================================================================================= //

Mode mode =
{
    .abi_version        = ABI_VERSION,
    .name               = "file-browser",
    .cfg_name_key       = "display-file-browser",
    ._init              = file_browser_init,
    ._get_num_entries   = file_browser_get_num_entries,
    ._result            = file_browser_result,
    ._destroy           = file_browser_destroy,
    ._token_match       = file_browser_token_match,
    ._get_display_value = file_browser_get_display_value,
    ._get_icon          = file_browser_get_icon,
    ._get_message       = file_browser_get_message,

    ._get_completion    = NULL,
    ._preprocess_input  = NULL,
    .private_data       = NULL,
    .free               = NULL,
};
