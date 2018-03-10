/**
 * rofi-file_browser
 *
 * MIT/X11 License
 * Copyright (c) 2017 Qball Cow <qball@gmpclient.org>
 * Copyright (c) 2018 Marvin Kreis <MarvinKreis@web.de>
 *
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
 * CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
 * TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
 * SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

// ================================================================================================================= //

/*
 * Displays a file browser.
 * Can display icons for individual file and folder types.
 *
 * MENU_NEXT and MENU_PREVIOUS can be used to toggle hidden files on or off.
 *
 * Type :<cmd> to set the command to open files with, cmd can be a plain program name or a string containing %s.
 * %s will then be replaced with the file name.
 *
 * Command line options:
 *      -fb_cmd     Sets the command used to open the file with.
 *      -fb_dir     Sets the starting directory.
 *      -fb_theme   Sets the icon theme, can be used multiple times to set fallback themes.
 */

// ================================================================================================================= //

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <gmodule.h>
#include <gio/gio.h>

#include <rofi/mode.h>
#include <rofi/helper.h>
#include <rofi/mode-private.h>

#include <stdbool.h>
#include <dirent.h>
#include <nkutils-xdg-theme.h>

// ================================================================================================================= //

/* The default starting directory. */
#define START_DIR g_get_home_dir()

/* The default icon themes. */
#define ICON_THEMES "Numix-Circle"

/* Fallback icon themes. */
#define FALLBACK_THEMES "Adwaita", "gnome"

/* Display a message showing the current mode (show hidden files or not) and the current path. */
#define USE_MESSAGE true

/* Format of the message. Arguments are: "current mode" - "current path" */
#define MESSAGE_FORMAT " %s%s"

/* Symbols for displaying the mode in the message. */
#define NO_HIDDEN_SYMBOL "-"
#define SHOW_HIDDEN_SYMBOL "+"

/* The name to display for the parent directory. */
#define PARENT_NAME ".."

/* The format use to display files and directories. */
#define FILE_FORMAT "%s"
#define DIRECTORY_FORMAT "%s"

/* The default command to use to open files. */
#define CMD "xdg-open '%s'"

// ================================================================================================================= //

G_MODULE_EXPORT Mode mode;

typedef enum FBFileType {
    UP,
    DIRECTORY,
    RFILE,
} FBFileType;

typedef struct {
    char* name;
    /* The absolute path of the file. */
    char* path;
    enum FBFileType type;
} FBFile;

typedef struct {
    /* Current directory */
    GFile* current_dir;

    /* List of displayed files */
    FBFile* files;
    /* Number of displayed files */
    unsigned int num_files;

    /* Loaded icons by their names */
    GHashTable* icons;
    /* Icon theme context */
    NkXdgThemeContext *xdg_context;
    /* Used icon themes with fallbacks */
    char** icon_themes;

    /* Show hidden files */
    bool show_hidden;
    /* Command to open files with */
    char* cmd;
} FileBrowserModePrivateData;

// ================================================================================================================= //

/**
 * @param Mode The current mode.
 *
 * Gets the files in the current directory and sets them in the private data.
 */
static void get_file_browser ( Mode* sw );

/**
 * @param pd The private data.
 *
 * Frees the current file list.
 */
static void free_list ( FileBrowserModePrivateData* pd );

/**
 * @param The path the get the absolute path of.
 * @param current_dir The current directory.
 *
 * Gets the absolute path of either an already absolute path or a relative path to the current directory.
 * If no file or directory with the path, exists, NULL is returned.
 *
 * @return The absolute path, or NULL if no file or directory with the path exists.
 *         If a path is returned, it should be free'd with g_free() afterwards.
 */
static char* get_absolute_path ( char* path, GFile* current_dir );

/**
 * @param path The path the get the absolute path of.
 * @param current_dir The current directory.
 *
 * Gets the absolute path of either an already absolute path or a relative path to the current directory.
 * If no file or directory with the path, exists, NULL is returned.
 *
 * @return The absolute path, or NULL if no file or directory with the path exists.
 */
static void change_dir ( char* path, Mode* sw );

/**
 * @param path The absolute path of the file to open.
 * @param current_dir The current directory.
 * @param cmd The command to execute, as a formatted string with %s to be replaced with the file name or the plain
 *            command name.
 *
 * Opens the file at the given path in the current directory with the given command.
 */
static void open_file ( char* path, GFile* current_dir, const char* cmd );

/**
 * @param fbfile The file to get the icon names of.
 *
 * Gets a list of icon names for the file. The first icon name is the most specific, the following are fallback
 * icons.
 *
 * @return The list of icon names. The list should be free'd with g_strfreev() afterwards.
 */
static char** get_icon_names ( FBFile file );

/**
 * @param icon_names The icon names to get the icon of.
 * @param icon_size The size of the icon to get.
 * @param sw The current mode.
 *
 * Gets the most specific icon possible for a list of icon names with fallbacks.
 *
 * @return An icon for the given list of icon names, or NULL if the icon doesn't exist.
 */
static cairo_surface_t* get_icon_surf ( char** icon_names, int icon_size, const Mode* sw );

/**
 * @param a The first file.
 * @param b The first file.
 * @param data unused
 *
 * Comparison function for sorting the file list.
 * Compares two files by the order in which they should appear in the file browser.
 * Directories should appear before regular files, directories and files should be sorted alphabetically.
 */
static gint compare ( gconstpointer a, gconstpointer b, gpointer data );

// ================================================================================================================= //

static int file_browser_init ( Mode* sw )
{
    if ( mode_get_private_data ( sw ) == NULL ) {
        FileBrowserModePrivateData* pd = g_malloc0 ( sizeof ( *pd ) );
        mode_set_private_data ( sw, ( void * ) pd );

        pd->show_hidden = false;

        /* Set the command used to open the file with. */
        char* cmd = NULL;
        if ( find_arg_str ( "-fb_cmd", &cmd ) ) {
            pd->cmd = g_strdup ( cmd );
        } else {
            pd->cmd = g_strdup ( CMD );
        }

        /* Set the start directory. */
        char* start_dir = NULL;
        if ( find_arg_str ( "-fb_dir", &start_dir ) ) {
            pd->current_dir = g_file_new_for_path ( start_dir );
            if ( !g_file_query_exists ( pd->current_dir, NULL ) ) {
                g_object_unref ( pd->current_dir );
                fprintf ( stderr, "[file_browser] Start directory does not exist: %s\n", start_dir );
                pd->current_dir = g_file_new_for_path ( START_DIR );
            }
        } else {
            pd->current_dir = g_file_new_for_path ( START_DIR );
        }

        const gchar * const fallback_themes[] = {
            FALLBACK_THEMES,
            NULL
        };

        const gchar *default_themes[] = {
            ICON_THEMES,
            NULL
        };

        pd->icon_themes = g_strdupv ( ( char ** ) find_arg_strv ( "-fb_theme" ) );
        if ( pd->icon_themes == NULL ) {
            pd->icon_themes = g_strdupv ( ( char ** ) default_themes );
        }

        pd->xdg_context = nk_xdg_theme_context_new ( ( const gchar* const* ) fallback_themes, NULL );
        nk_xdg_theme_preload_themes_icon ( pd->xdg_context, ( const char ** ) pd->icon_themes );
        pd->icons = g_hash_table_new_full ( g_str_hash, g_str_equal, g_free, ( void (*) ( void * ) ) cairo_surface_destroy );

        /* Load content. */
        get_file_browser ( sw );
    }

    return true;
}

static void file_browser_destroy ( Mode* sw )
{
    FileBrowserModePrivateData* pd = ( FileBrowserModePrivateData * ) mode_get_private_data ( sw );
    mode_set_private_data ( sw, NULL );

    if ( pd != NULL ) {
        /* Free file list. */
        g_object_unref ( pd->current_dir );
        free_list ( pd );

        /* Free icon themes and icons. */
        g_hash_table_destroy ( pd->icons );
        nk_xdg_theme_context_free ( pd->xdg_context );
        g_strfreev ( pd->icon_themes );

        g_free ( pd->cmd );

        /* Fill with zeros, just in case. */
        memset ( ( void* ) pd , 0, sizeof ( pd ) );

        g_free ( pd );
    }
}

static unsigned int file_browser_get_num_entries ( const Mode* sw )
{
    const FileBrowserModePrivateData* pd = ( const FileBrowserModePrivateData * ) mode_get_private_data ( sw );

    if ( pd != NULL ) {
        return pd->num_files;
    } else {
        return 0;
    }
}

static ModeMode file_browser_result ( Mode* sw, int mretv, char **input, unsigned int selected_line )
{
    FileBrowserModePrivateData* pd = ( FileBrowserModePrivateData * ) mode_get_private_data ( sw );

    ModeMode retv = RESET_DIALOG;

    if ( mretv & MENU_CANCEL ) {
        return MODE_EXIT;
    } else if ( ( mretv & MENU_NEXT ) || ( mretv & MENU_PREVIOUS ) ) {
        pd->show_hidden = !pd->show_hidden;
        get_file_browser ( sw );
        return RELOAD_DIALOG;
    } else if ( ( mretv & MENU_OK ) ) {

        FBFile* entry = &( pd->files[selected_line] );
        if ( entry->type == UP || entry->type == DIRECTORY ) {
            change_dir ( entry->path, sw );
            return RESET_DIALOG;
        } else {
            open_file ( entry->path, pd->current_dir, pd->cmd );
            return MODE_EXIT;
        }

    } else if ( ( mretv & MENU_CUSTOM_INPUT ) && *input != NULL ) {

        char *expanded_input = rofi_expand_path ( *input );
        char *file = g_filename_from_utf8 ( expanded_input, -1, NULL, NULL, NULL );
        g_free ( expanded_input );

        char* abs_path = get_absolute_path ( file, pd->current_dir );
        g_free ( file );

        if ( abs_path == NULL ) {
            if ( ( *input )[0] == ':' ) {
                pd->cmd = g_strdup ( ( *input ) + 1 );
                retv = RESET_DIALOG;
            } else {
                retv = RELOAD_DIALOG;
            }
        } else if ( g_file_test ( abs_path, G_FILE_TEST_IS_DIR ) ){
            change_dir ( abs_path, sw );
            retv = RESET_DIALOG;
        } else if ( g_file_test ( abs_path, G_FILE_TEST_IS_REGULAR ) ) {
            open_file ( abs_path, pd->current_dir, pd->cmd );
            retv = MODE_EXIT;
        }

        g_free ( abs_path );

    }

    return retv;
}

static int file_browser_token_match ( const Mode* sw, rofi_int_matcher **tokens, unsigned int index )
{
    FileBrowserModePrivateData* pd = ( FileBrowserModePrivateData * ) mode_get_private_data ( sw );

    return helper_token_match ( tokens, pd->files[index].name);
}

static char* file_browser_get_display_value ( const Mode* sw, unsigned int selected_line, G_GNUC_UNUSED int *state, G_GNUC_UNUSED GList **attr_list, int get_entry )
{
    FileBrowserModePrivateData* pd = ( FileBrowserModePrivateData * ) mode_get_private_data ( sw );

    if ( !get_entry ) return NULL;

    // MARKUP flag, not defined in accessible headers
    *state |= 8;

    switch ( pd->files[selected_line].type ) {
    case UP:
        return g_strdup ( PARENT_NAME );
    case RFILE:
        return g_strdup_printf ( FILE_FORMAT, pd->files[selected_line].name );
    case DIRECTORY:
        return g_strdup_printf ( DIRECTORY_FORMAT, pd->files[selected_line].name );
    default:
        return g_strdup ( "error" );
    }
}

static cairo_surface_t* file_browser_get_icon ( const Mode* sw, unsigned int selected_line, int height )
{
    FileBrowserModePrivateData* pd = ( FileBrowserModePrivateData * ) mode_get_private_data ( sw );

    char** icon_names = get_icon_names ( pd->files[selected_line] );
    cairo_surface_t* icon =  get_icon_surf ( icon_names, height, sw );
    g_strfreev ( icon_names );

    return icon;
}

static char* file_browser_get_message (const Mode* sw)
{
    FileBrowserModePrivateData* pd = ( FileBrowserModePrivateData * ) mode_get_private_data ( sw );

    static char* mode_symbols[2] = { NO_HIDDEN_SYMBOL, SHOW_HIDDEN_SYMBOL };

    char* path = g_file_get_path ( pd->current_dir );
    char** split = g_strsplit ( g_file_get_path(pd->current_dir), "/", -1 );
    char* join = g_strjoinv ( " / ", split );
    char* result = g_strdup_printf ( MESSAGE_FORMAT, mode_symbols[pd->show_hidden], join );

    g_free ( path );
    g_strfreev ( split );
    g_free ( join );

    return result;
}

// ================================================================================================================= //

static void get_file_browser ( Mode* sw )
{
    FileBrowserModePrivateData* pd = ( FileBrowserModePrivateData * ) mode_get_private_data ( sw );

    free_list ( pd );

    char *cdir = g_file_get_path ( pd->current_dir );
    DIR *dir = opendir ( cdir );

    if ( dir != NULL ) {
        struct dirent *rd = NULL;
        while ( ( rd = readdir ( dir ) ) != NULL ) {
            /* Ignore rd if rd is the current directory or rd is a hidden file and show_hidden is false. */
            if ( ( g_strcmp0 ( rd->d_name, "." ) == 0 )
              || ( !pd->show_hidden
                    && rd->d_name[0] == '.'
                    && g_strcmp0 ( rd->d_name, ".." ) != 0 ) ) {
                continue;
            }

            if ( rd->d_type == DT_REG || rd->d_type == DT_DIR || rd->d_type == DT_LNK ) {
                pd->files = g_realloc ( pd->files, ( pd->num_files + 1 ) * sizeof ( FBFile ) );

                FBFile* entry = &( pd->files[pd->num_files] );

                entry->name = g_filename_to_utf8 ( rd->d_name, -1, NULL, NULL, NULL);
                entry->path = g_build_filename ( cdir, rd->d_name, NULL );

                if ( g_strcmp0 ( rd->d_name, ".." ) == 0 ) {
                    entry->type = UP;
                } else {
                    switch ( rd->d_type ) {
                        case DT_REG:
                            entry->type = RFILE;
                            break;
                        case DT_DIR:
                            entry->type = DIRECTORY;
                            break;
                        case DT_LNK:
                            entry->type = g_file_test ( entry->path, G_FILE_TEST_IS_DIR ) ? DIRECTORY : RFILE;
                    }
                }

                pd->num_files++;
            }
        }

        closedir ( dir );
    }

    g_qsort_with_data ( pd->files, pd->num_files, sizeof (FBFile ), compare, NULL );
}

static void free_list ( FileBrowserModePrivateData* pd )
{
    for ( unsigned int i = 0; i < pd->num_files; i++ ) {
        FBFile *fb = & ( pd->files[i] );
        g_free ( fb->name );
        g_free ( fb->path );
    }
    g_free (pd->files);
    pd->files  = NULL;
    pd->num_files = 0;
}

static char* get_absolute_path ( char* path, GFile* current_dir )
{
    if ( g_file_test ( path, G_FILE_TEST_EXISTS ) ) {
        return g_strdup ( path );
    } else {
        char* current_path = g_file_get_path ( current_dir );
        char* new_path = g_build_filename ( current_path, path, NULL );
        g_free ( current_path );

        if ( g_file_test ( new_path, G_FILE_TEST_EXISTS ) ) {
            return new_path;
        } else {
            g_free ( new_path );
            return NULL;
        }
    }
}

static void change_dir ( char* path, Mode* sw )
{
    FileBrowserModePrivateData* pd = ( FileBrowserModePrivateData * ) mode_get_private_data ( sw );

    GFile *new_dir = g_file_new_for_path ( path );
    g_object_unref ( pd->current_dir );
    pd->current_dir = new_dir;
    get_file_browser ( sw );
}

static void open_file ( char* path, GFile* current_dir, const char* cmd )
{
    char* current_path = g_file_get_path ( current_dir );
    char* complete_cmd = NULL;

    if ( g_strrstr ( cmd, "%s" ) != NULL ) {
        complete_cmd = g_strdup_printf ( cmd, path );
    } else {
        complete_cmd = g_strconcat ( cmd, " ", path, NULL );
    }

    helper_execute_command ( current_path, complete_cmd, FALSE, NULL );

    g_free ( current_path );
    g_free ( complete_cmd );
}

static char** get_icon_names ( FBFile fbfile )
{
    static char* error_icon[] = { "error", NULL };
    static char* up_icon[] = { "go-up", NULL };
    static char* folder_icon[] = { "folder", NULL };

    /* Special cases: directory up and NULL. */
    if ( fbfile.path == NULL ) {
        return g_strdupv ( error_icon );
    } else if ( fbfile.type == UP ) {
        return g_strdupv ( up_icon );
    }

    char** names = NULL;
    GError *error = NULL;

    GFile *file = g_file_new_for_path ( fbfile.path );
    GFileInfo *file_info = g_file_query_info ( file, "standard::icon", G_FILE_QUERY_INFO_NONE, NULL, &error );

    if ( error == NULL ) {

        GIcon *icon = g_file_info_get_icon ( file_info );

        if ( G_IS_THEMED_ICON ( icon ) ) {
            names = g_strdupv ( ( char ** ) g_themed_icon_get_names ( G_THEMED_ICON ( icon ) ) );
        }

        g_object_unref ( icon );
    }

    g_object_unref ( file );

    if ( names != NULL ) {
        return names;
    } else {
        return g_strdupv ( error_icon );
    }
}

static cairo_surface_t* get_icon_surf ( char** icon_names, int icon_size, const Mode* sw ) {
    FileBrowserModePrivateData* pd = ( FileBrowserModePrivateData * ) mode_get_private_data ( sw );

    for (int i = 0; icon_names[i] != NULL; i++) {

        cairo_surface_t *icon_surf = g_hash_table_lookup ( pd->icons, icon_names[i] );

        if ( icon_surf != NULL ) {
            return icon_surf;
        }

        gchar* icon_path = nk_xdg_theme_get_icon ( pd->xdg_context, ( const char ** ) pd->icon_themes,
                                        NULL, icon_names[i], icon_size, 1, true );

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
                return icon_surf;
            }
        }
    }

    return NULL;
}

static gint compare ( gconstpointer a, gconstpointer b, gpointer data )
{
    FBFile *fa = (FBFile*)a;
    FBFile *fb = (FBFile*)b;
    if ( fa->type != fb->type ){
        return (fa->type - fb->type);
    }

    return g_strcmp0 ( fa->name, fb->name );
}

// ================================================================================================================= //

Mode mode =
{
    .abi_version        = ABI_VERSION,
    .name               = "file_browser",
    .cfg_name_key       = "display-file_browser",
    ._init              = file_browser_init,
    ._get_num_entries   = file_browser_get_num_entries,
    ._result            = file_browser_result,
    ._destroy           = file_browser_destroy,
    ._token_match       = file_browser_token_match,
    ._get_display_value = file_browser_get_display_value,
    ._get_icon          = file_browser_get_icon,

#if USE_MESSAGE
    ._get_message       = file_browser_get_message,
#else
    ._get_message       = NULL,
#endif

    ._get_completion    = NULL,
    ._preprocess_input  = NULL,
    .private_data       = NULL,
    .free               = NULL,
};
