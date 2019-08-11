#include <stdbool.h>
#include <stdio.h>
#include <gmodule.h>
#include <glib/gstdio.h>
#include <cairo.h>
#include <nkutils-xdg-theme.h>
#include <rofi/helper.h>

#include "defaults.h"
#include "types.h"
#include "util.h"
#include "options.h"
#include "icons.h"
#include "keys.h"
#include "cmds.h"

/**
 * Returns a newly allocated copy of a string command line option if it is specified.
 * Otherwise, returns a newly allocated copy of the given default value.
 * // TODO: update
 */
static char *get_string_arg ( char *name, char *default_val, FileBrowserModePrivateData *pd );

static void read_config_file ( char* path, FileBrowserModePrivateData *pd );

static bool fb_find_arg_str ( char* option, char **arg, FileBrowserModePrivateData *pd );
static char **fb_find_arg_strv ( char* option, FileBrowserModePrivateData *pd );
static bool fb_find_arg_int ( char* option, int *arg, FileBrowserModePrivateData *pd );
static bool fb_find_arg ( char* option, FileBrowserModePrivateData *pd );

// ================================================================================================================= //

bool set_options ( FileBrowserModePrivateData *pd )
{
    FileBrowserFileData *fd = &pd->file_data;
    FileBrowserIconData *id = &pd->icon_data;
    FileBrowserKeyData *kd = &pd->key_data;

    char *config_file = NULL;
    if ( ! find_arg_str ( "-file-browser-config", &config_file ) ) {
        config_file = CONFIG_FILE;
    }
    read_config_file ( config_file, pd );

    fd->show_hidden         = fb_find_arg ( "-file-browser-show-hidden"         , pd ) ? true  : SHOW_HIDDEN;
    fd->only_dirs           = fb_find_arg ( "-file-browser-only-dirs"           , pd ) ? true  : ONLY_DIRS;
    fd->only_files          = fb_find_arg ( "-file-browser-only-files"          , pd ) ? true  : ONLY_FILES;
    fd->hide_parent         = fb_find_arg ( "-file-browser-hide-parent"         , pd ) ? true  : HIDE_PARENT;
    id->show_icons          = fb_find_arg ( "-file-browser-disable-icons"       , pd ) ? false : SHOW_ICONS;
    kd->use_mode_keys       = fb_find_arg ( "-file-browser-disable-mode-keys"   , pd ) ? false : USE_MODE_KEYS;
    pd->dmenu               = fb_find_arg ( "-file-browser-dmenu"               , pd ) ? true  : DMENU;
    pd->show_status         = fb_find_arg ( "-file-browser-disable-status"      , pd ) ? false : SHOW_STATUS;
    pd->no_descend          = fb_find_arg ( "-file-browser-no-descend"          , pd ) ? true  : NO_DESCEND;
    pd->stdin_mode          = fb_find_arg ( "-file-browser-stdin"               , pd ) ? true  : STDIN_MODE;
    pd->open_parent_as_self = fb_find_arg ( "-file-browser-open-parent-as-self" , pd ) ? true  : OPEN_PARENT_AS_SELF;

    fd->up_text             = get_string_arg ( "-file-browser-up-text",            UP_TEXT, pd);
    id->up_icon             = get_string_arg ( "-file-browser-up-icon",            UP_ICON, pd );
    id->inaccessible_icon   = get_string_arg ( "-file-browser-inaccessible-icon",  INACCESSIBLE_ICON, pd );
    id->fallback_icon       = get_string_arg ( "-file-browser-fallback-icon",      FALLBACK_ICON, pd );
    id->error_icon          = get_string_arg ( "-file-browser-error-icon",         ERROR_ICON, pd );
    pd->cmd                 = get_string_arg ( "-file-browser-cmd",                CMD, pd );
    pd->show_hidden_symbol  = get_string_arg ( "-file-browser-show-hidden-symbol", SHOW_HIDDEN_SYMBOL, pd );
    pd->hide_hidden_symbol  = get_string_arg ( "-file-browser-hide-hidden-symbol", HIDE_HIDDEN_SYMBOL, pd );
    pd->path_sep            = get_string_arg ( "-file-browser-path-sep",           PATH_SEP, pd );

    /* Depth. */
    if ( ! fb_find_arg_int ( "-file-browser-depth", &fd->depth, pd ) ) {
        fd->depth = DEPTH;
    }

    /* Sorting method. */
    int sort_by_type;
    if ( fb_find_arg_int ( "-file-browser-sort-by-type", &sort_by_type, pd ) ) {
        fd->sort_by_type = sort_by_type != 0;
    } else {
        fd->sort_by_type = SORT_BY_TYPE;
    }
    int sort_by_depth;
    if ( fb_find_arg_int ( "-file-browser-sort-by-depth", &sort_by_depth, pd ) ) {
        fd->sort_by_depth = sort_by_depth != 0;
    } else {
        fd->sort_by_depth = SORT_BY_DEPTH;
    }

    /* Start directory. */
    char *start_dir = NULL;
    if ( ! fb_find_arg_str ( "-file-browser-dir", &start_dir, pd ) ) {
        start_dir = START_DIR;
    }
    char *abs_path = get_existing_abs_path ( start_dir, g_get_current_dir() );
    if ( abs_path != NULL ) {
        fd->current_dir = abs_path;
    } else {
        print_err ( "Start directory does not exist: \"%s\".\n", start_dir );
        return false;
    }

    /* Icon theme. */
    char **icon_themes = g_strdupv ( ( char ** ) fb_find_arg_strv ( "-file-browser-theme", pd ) );
    if ( icon_themes == NULL ) {
        icon_themes = g_strdupv ( ( char ** ) fb_find_arg_strv ( "-file-browser-icon-theme", pd ) );
    }
    if ( icon_themes != NULL ) {
        id->icon_themes = icon_themes;
    } else {
        id->icon_themes = get_default_icon_theme ();
    }

    /* Set glob patterns. */
    char **exclude_globs_strs = g_strdupv ( ( char ** ) fb_find_arg_strv ( "-file-browser-exclude", pd ) );
    if ( exclude_globs_strs == NULL ) {
        fd->num_exclude_patterns = 0;
    } else {
        int num_globs;
        for ( num_globs = 0; exclude_globs_strs[num_globs] != NULL; num_globs++ ) { }

        fd->num_exclude_patterns = num_globs;
        fd->exclude_patterns = g_malloc ( num_globs * sizeof ( GPatternSpec* ) );
        for ( int i = 0; i < num_globs; i++ ) {
            fd->exclude_patterns[i] = g_pattern_spec_new ( exclude_globs_strs[i] );
        }
    }

    /* Set commands for open-custom. */
    char ** cmds = g_strdupv ( ( char ** ) fb_find_arg_strv ( "-file-browser-oc-cmd", pd ) );
    set_open_custom_cmds ( cmds, pd );
    g_strfreev ( cmds );
    if ( fb_find_arg ( "-file-browser-oc-find-cmds", pd ) != -1 ) {
        find_custom_cmds ( pd );
    }

    /* Set key bindings. */
    char *open_custom_key_str = NULL;
    char *open_multi_key_str = NULL;
    char *toggle_hidden_key_str = NULL;
    fb_find_arg_str ( "-file-browser-open-custom-key", &open_custom_key_str, pd );
    fb_find_arg_str ( "-file-browser-open-multi-key", &open_multi_key_str, pd );
    fb_find_arg_str ( "-file-browser-toggle-hidden-key", &toggle_hidden_key_str, pd );
    set_key_bindings ( open_custom_key_str, open_multi_key_str, toggle_hidden_key_str, &pd->key_data );

    return true;
}

void destroy_options ( FileBrowserModePrivateData *pd ) {
    g_hash_table_destroy ( pd->config_table );
}

static void free_arg_list ( gpointer data )
{
    GSList *list = data;
    g_slist_free_full ( list, g_free );
}

static void read_config_file ( char *config_file, FileBrowserModePrivateData *pd )
{
    pd->config_table = g_hash_table_new_full ( g_str_hash, g_str_equal, g_free, NULL );

    if ( ! g_file_test ( config_file, G_FILE_TEST_IS_REGULAR ) ) {
        print_err ( "Could not open config file. \"%s\" does not exist or is not a regular file.\n", config_file );
        return;
    }

    FILE *file =  g_fopen ( config_file, "r" );
    if ( file == NULL ) {
        print_err ( "Could not open config file \"%s\".\n", config_file );
        return;
    }

    char *buffer = NULL;
    size_t len = 0;

    while ( getline ( &buffer, &len, file ) != -1 ) {
        g_strstrip ( buffer );

        /* Skip empty lines and commented out lines. */
        if ( buffer[0] == '\0' || buffer[0] == '#' ) {
            continue;
        }

        char *option = strtok ( buffer, " " );
        char *arg = strtok ( NULL, "" );

        /* Remove quotes. */
        if ( arg != NULL && ( arg[0] == '"' || arg[0] == '\'' ) ) {
            int last_char = strlen ( arg ) - 1;
            if ( arg[last_char] == arg[0] ) {
                arg[last_char] = '\0';
                arg++;
            }
        }

        if ( arg == NULL ) {
            g_hash_table_insert ( pd->config_table, g_strdup ( option ), NULL );
        } else {
            GSList* args = g_hash_table_lookup ( pd->config_table, option );
            args = g_slist_append ( args, g_strdup ( arg ) );
            g_hash_table_insert ( pd->config_table, g_strdup ( option ), args );
        }
    }

    g_free ( buffer );
}

static bool fb_find_arg ( char* option, FileBrowserModePrivateData *pd )
{
    if ( find_arg ( option ) != -1 ) {
        return true;
    }

    if ( g_hash_table_contains ( pd->config_table, option ) ) {
        return true;
    }

    return false;
}

static bool fb_find_arg_int ( char* option, int *arg, FileBrowserModePrivateData *pd )
{
    if ( find_arg_int ( option, arg ) ) {
        return true;
    }

    GSList *list = g_hash_table_lookup ( pd->config_table, option );

    if ( list == NULL ) {
        return false;
    }

    char* str_arg = list->data, *res;
    *arg = strtol ( str_arg, &res, 10 );
    if ( *res == '\0' ) {
        return true;
    } else {
        print_err ( "Invalid argument for option \"%s\" in config file: \"%s\".\n", option, str_arg );
        return false;
    }
}

static bool fb_find_arg_str ( char* option, char **arg, FileBrowserModePrivateData *pd )
{
    if ( find_arg_str ( option, arg ) ) {
        *arg = g_strdup ( *arg );
        return true;
    }

    GSList *list = g_hash_table_lookup ( pd->config_table, option );

    if ( list == NULL ) {
        return false;
    } else {
        *arg = g_strdup ( list->data );
        return true;
    }
}

static char **fb_find_arg_strv ( char* option, FileBrowserModePrivateData *pd )
{
    const char** cli_args = find_arg_strv ( option );
    GSList *list = g_hash_table_lookup ( pd->config_table, option );

    if ( list == NULL ) {
        char ** retv = g_strdupv ( ( char ** ) cli_args );
        g_free ( cli_args );
        return retv;
    }

    int num_cli = count_strv ( cli_args );
    int num_file = g_slist_length ( list );

    char **args = g_malloc ( ( num_cli + num_file + 1 ) * sizeof ( char * ) );
    args[num_cli + num_file] = NULL;

    int i;
    for ( i = 0; i < num_cli; i++ ) {
        args[i] = g_strdup ( cli_args[i] );
    }
    for ( ; list != NULL; list = list->next ) {
        args[i++] = g_strdup ( list->data );
    }

    return args;
}

static char* get_string_arg ( char *name, char *default_val, FileBrowserModePrivateData *pd )
{
    char* val;
    if ( fb_find_arg_str ( name, &val, pd ) ) {
        return val;
    } else {
        return g_strdup ( default_val );
    }
}
