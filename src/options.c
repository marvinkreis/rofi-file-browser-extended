#include <stdbool.h>
#include <gmodule.h>
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
 */
static char *get_string_option ( char *name, char *default_val );

// ================================================================================================================= //

bool set_command_line_options ( FileBrowserModePrivateData *pd )
{
    FileBrowserFileData *fd = &pd->file_data;
    FileBrowserIconData *id = &pd->icon_data;
    FileBrowserKeyData *kd = &pd->key_data;

    fd->show_hidden         = ( find_arg ( "-file-browser-show-hidden"         ) != -1 ) ? true  : SHOW_HIDDEN;
    fd->only_dirs           = ( find_arg ( "-file-browser-only-dirs"           ) != -1 ) ? true  : ONLY_DIRS;
    fd->only_files          = ( find_arg ( "-file-browser-only-files"          ) != -1 ) ? true  : ONLY_FILES;
    fd->hide_parent         = ( find_arg ( "-file-browser-hide-parent"         ) != -1 ) ? true  : HIDE_PARENT;
    id->show_icons          = ( find_arg ( "-file-browser-disable-icons"       ) != -1 ) ? false : SHOW_ICONS;
    kd->use_mode_keys       = ( find_arg ( "-file-browser-disable-mode-keys"   ) != -1 ) ? false : USE_MODE_KEYS;
    pd->dmenu               = ( find_arg ( "-file-browser-dmenu"               ) != -1 ) ? true  : DMENU;
    pd->show_status         = ( find_arg ( "-file-browser-disable-status"      ) != -1 ) ? false : SHOW_STATUS;
    pd->no_descend          = ( find_arg ( "-file-browser-no-descend"          ) != -1 ) ? true  : NO_DESCEND;
    pd->stdin_mode          = ( find_arg ( "-file-browser-stdin"               ) != -1 ) ? true  : STDIN_MODE;
    pd->open_parent_as_self = ( find_arg ( "-file-browser-open-parent-as-self" ) != -1 ) ? true  : OPEN_PARENT_AS_SELF;

    fd->up_text             = get_string_option ( "-file-browser-up-text",            UP_TEXT );
    id->up_icon             = get_string_option ( "-file-browser-up-icon",            UP_ICON );
    id->inaccessible_icon   = get_string_option ( "-file-browser-inaccessible-icon",  INACCESSIBLE_ICON );
    id->fallback_icon       = get_string_option ( "-file-browser-fallback-icon",      FALLBACK_ICON );
    id->error_icon          = get_string_option ( "-file-browser-error-icon",         ERROR_ICON );
    pd->cmd                 = get_string_option ( "-file-browser-cmd",                CMD );
    pd->show_hidden_symbol  = get_string_option ( "-file-browser-show-hidden-symbol", SHOW_HIDDEN_SYMBOL );
    pd->hide_hidden_symbol  = get_string_option ( "-file-browser-hide-hidden-symbol", HIDE_HIDDEN_SYMBOL );
    pd->path_sep            = get_string_option ( "-file-browser-path-sep",           PATH_SEP );

    /* Depth. */
    if ( ! find_arg_int ( "-file-browser-depth", &fd->depth ) ) {
        fd->depth = DEPTH;
    }

    /* Sorting method. */
    int sort_by_type;
    if ( find_arg_int ( "-file-browser-sort-by-type", &sort_by_type ) ) {
        fd->sort_by_type = sort_by_type != 0;
    } else {
        fd->sort_by_type = SORT_BY_TYPE;
    }
    int sort_by_depth;
    if ( find_arg_int ( "-file-browser-sort-by-depth", &sort_by_depth ) ) {
        fd->sort_by_depth = sort_by_depth != 0;
    } else {
        fd->sort_by_depth = SORT_BY_DEPTH;
    }

    /* Start directory. */
    char *start_dir = NULL;
    if ( ! find_arg_str ( "-file-browser-dir", &start_dir ) ) {
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
    char **icon_themes = g_strdupv ( ( char ** ) find_arg_strv ( "-file-browser-theme" ) );
    if ( icon_themes == NULL ) {
        icon_themes = g_strdupv ( ( char ** ) find_arg_strv ( "-file-browser-icon-theme" ) );
    }
    if ( icon_themes != NULL ) {
        id->icon_themes = icon_themes;
    } else {
        id->icon_themes = get_default_icon_theme ();
    }

    /* Set glob patterns. */
    char **exclude_globs_strs = g_strdupv ( ( char ** ) find_arg_strv ( "-file-browser-exclude" ) );
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
    char ** cmds = g_strdupv ( ( char ** ) find_arg_strv ( "-file-browser-oc-cmd" ) );
    if ( cmds != NULL ) {
        set_open_custom_cmds(cmds, pd);
    }
    g_strfreev ( cmds );
    if ( find_arg ( "-file-browser-oc-find-cmds" ) != -1 ) {
        find_custom_cmds ( pd );
    }

    /* Set key bindings. */
    char *open_custom_key_str = NULL;
    char *open_multi_key_str = NULL;
    char *toggle_hidden_key_str = NULL;
    find_arg_str ( "-file-browser-open-custom-key", &open_custom_key_str );
    find_arg_str ( "-file-browser-open-multi-key", &open_multi_key_str );
    find_arg_str ( "-file-browser-toggle-hidden-key", &toggle_hidden_key_str );
    set_key_bindings ( open_custom_key_str, open_multi_key_str, toggle_hidden_key_str, &pd->key_data );

    return true;
}

static char* get_string_option ( char *name, char* default_val )
{
    char* val;
    if ( find_arg_str ( name, &val ) ) {
        return g_strdup ( val );
    } else {
        return g_strdup ( default_val );
    }
}

