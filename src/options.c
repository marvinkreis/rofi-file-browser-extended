#include <stdbool.h>
#include <gmodule.h>
#include <cairo.h>
#include <nkutils-xdg-theme.h>
#include <rofi/helper.h>

#include "config.h"
#include "types.h"
#include "util.h"
#include "options.h"
#include "icons.h"
#include "keys.h"

/**
 * Returns a newly allocated copy of a string command line option if it is specified.
 * Otherwise, returns a newly allocated copy of the given default value.
 */
static char *get_string_option ( char *name, char *default_val );

/**
 * Sets the key bindings from the command line options.
 * Prints error messages in case of invalid or clashing key bindings.
 */
static void set_key_bindings ( FileBrowserKeyData *kd );

// ================================================================================================================= //

bool set_command_line_options ( FileBrowserModePrivateData *pd )
{
    FileBrowserFileData *fd = &pd->file_data;
    FileBrowserIconData *id = &pd->icon_data;
    FileBrowserKeyData *kd = &pd->key_data;

    fd->show_hidden        = ( find_arg ( "-file-browser-show-hidden"          ) != -1 ) ? true  : SHOW_HIDDEN;
    fd->sort_by_type       = ( find_arg ( "-file-browser-disable-sort-by-type" ) != -1 ) ? false : SORT_BY_TYPE;
    id->show_icons         = ( find_arg ( "-file-browser-disable-icons"        ) != -1 ) ? false : SHOW_ICONS;
    kd->use_mode_keys      = ( find_arg ( "-file-browser-disable-mode-keys"    ) != -1 ) ? false : USE_MODE_KEYS;
    pd->dmenu              = ( find_arg ( "-file-browser-dmenu"                ) != -1 ) ? true  : DMENU;
    pd->show_status        = ( find_arg ( "-file-browser-disable-status"       ) != -1 ) ? false : SHOW_STATUS;

    fd->up_text            = get_string_option ( "-file-browser-up-text",            UP_TEXT );
    id->up_icon            = get_string_option ( "-file-browser-up-icon",            UP_ICON );
    id->inaccessible_icon  = get_string_option ( "-file-browser-inaccessible-icon",  INACCESSIBLE_ICON );
    id->fallback_icon      = get_string_option ( "-file-browser-fallback-icon",      FALLBACK_ICON );
    id->error_icon         = get_string_option ( "-file-browser-error-icon",         ERROR_ICON );
    pd->cmd                = get_string_option ( "-file-browser-cmd",                CMD );
    pd->show_hidden_symbol = get_string_option ( "-file-browser-show-hidden-symbol", SHOW_HIDDEN_SYMBOL );
    pd->hide_hidden_symbol = get_string_option ( "-file-browser-hide-hidden-symbol", HIDE_HIDDEN_SYMBOL );
    pd->path_sep           = get_string_option ( "-file-browser-path-sep",           PATH_SEP );

    /* Depth. */
    if ( ! find_arg_int ( "-file-browser-depth", &fd->depth ) ) {
        fd->depth = DEPTH;
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
    if ( icon_themes != NULL ) {
        id->icon_themes = icon_themes;
    } else {
        id->icon_themes = get_default_icon_theme ();
    }

    set_key_bindings ( &pd->key_data );

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

static void set_key_bindings ( FileBrowserKeyData *kd )
{
    kd->open_custom_key   = OPEN_CUSTOM_KEY;
    kd->open_multi_key    = OPEN_MULTI_KEY;
    kd->toggle_hidden_key = TOGGLE_HIDDEN_KEY;

    FBKey *keys[] = { &kd->open_custom_key,
                      &kd->open_multi_key,
                      &kd->toggle_hidden_key };
    char  *names[] = { "open-custom",
                       "open-multi",
                       "toggle-hidden" };
    char  *options[] = { "-file-browser-open-custom-key",
                         "-file-browser-open-multi-key",
                         "-file-browser-toggle-hidden-key" };
    char  *params[]  = { NULL, NULL, NULL };

    for ( int i = 0; i < 3; i++ ) {
        if ( find_arg_str ( options[i], &params[i] ) ) {
            *keys[i] = get_key_for_name ( params[i] );
            if ( *keys[i] == KEY_NONE ) {
                print_err ( "Could not match key \"%s\". Disabling key binding for \"%s\". "
                        "Supported keys are \"kb-accept-alt\" and \"kb-custom-*\".\n", params[i], names[i] );
            }
        }
    }

    for ( int i = 0; i < 3; i++ ) {
        if ( params[i] != NULL && *keys[i] != KEY_NONE ) {
            for ( int j = 0; j < 3; j++ ) {
                if ( i != j && *keys[i] == *keys[j] ) {
                    *keys[j] = KEY_NONE;
                    char *key_name = get_name_of_key ( *keys[i] );
                    print_err ( "Detected key binding clash. Both \"%s\" and \"%s\" use \"%s\". "
                            "Disabling \"%s\".\n", names[i], names[j], key_name, names[j]);
                    g_free ( key_name );
                }
            }
        }
    }
}
