#include <stdio.h>
#include <stdbool.h>

#include <rofi/helper.h>
#include <nkutils-xdg-theme.h>
#include <gtk/gtk.h>

#include "config.h"
#include "types.h"
#include "util.h"
#include "options.h"

/**
 * Returns a newly allocated copy of a string command line option if it is specified.
 * Otherwise, returns a newly allocated copy of the given default value.
 */
static char* get_string_option ( char *name, char* default_val );

/**
 * Sets the key bindings from the command line options.
 * Prints error messages in case of invalid or clashing key bindings.
 */
static void set_key_bindings ( FileBrowserModePrivateData *pd );

/*
 * Returns the FBKey of a key name. If the name is invalid, returns KEY_NONE;
 */
static FBKey get_key_for_name ( char *key_str );

/**
 * Sets the default GTK icon theme as the theme, if possible.
 * Prints an error message if the default icon theme could not be determined.
 */
static void set_default_icon_theme ( FileBrowserModePrivateData *pd );

// ================================================================================================================= //

bool set_command_line_options ( FileBrowserModePrivateData *pd )
{
    pd->show_hidden        = ( find_arg ( "-file-browser-show-hidden"          ) != -1 ) ? true  : SHOW_HIDDEN;
    pd->show_icons         = ( find_arg ( "-file-browser-disable-icons"        ) != -1 ) ? false : SHOW_ICONS;
    pd->dmenu              = ( find_arg ( "-file-browser-dmenu"                ) != -1 ) ? true  : DMENU;
    pd->use_mode_keys      = ( find_arg ( "-file-browser-disable-mode-keys"    ) != -1 ) ? false : USE_MODE_KEYS;
    pd->show_status        = ( find_arg ( "-file-browser-disable-status"       ) != -1 ) ? false : SHOW_STATUS;
    pd->sort_by_type       = ( find_arg ( "-file-browser-disable-sort-by-type" ) != -1 ) ? false : SORT_BY_TYPE;

    pd->cmd                = get_string_option ( "-file-browser-cmd",                CMD );
    pd->show_hidden_symbol = get_string_option ( "-file-browser-show-hidden-symbol", SHOW_HIDDEN_SYMBOL );
    pd->hide_hidden_symbol = get_string_option ( "-file-browser-hide-hidden-symbol", HIDE_HIDDEN_SYMBOL );
    pd->path_sep           = get_string_option ( "-file-browser-path-sep",           PATH_SEP );
    pd->up_icon            = get_string_option ( "-file-browser-up-icon",            UP_ICON );
    pd->inaccessible_icon  = get_string_option ( "-file-browser-inaccessible-icon",  INACCESSIBLE_ICON );
    pd->fallback_icon      = get_string_option ( "-file-browser-fallback-icon",      FALLBACK_ICON );
    pd->error_icon         = get_string_option ( "-file-browser-error-icon",         ERROR_ICON );
    pd->up_text            = get_string_option ( "-file-browser-up-text",            UP_TEXT );

    /* Depth. */
    if ( ! find_arg_int ( "-file-browser-depth", &pd->depth ) ) {
        pd->depth = DEPTH;
    }

    /* Start directory. */
    char *start_dir = NULL;
    if ( ! find_arg_str ( "-file-browser-dir", &start_dir ) ) {
        start_dir = START_DIR;
    }
    char *abs_path = get_existing_abs_path ( start_dir, g_get_current_dir() );
    if ( abs_path != NULL ) {
        pd->current_dir = abs_path;
    } else {
        print_err ( "Start directory does not exist: %s\n", start_dir );
        return false;
    }

    /* Icon theme. */
    char **icon_themes = g_strdupv ( ( char ** ) find_arg_strv ( "-file-browser-theme" ) );
    if ( pd->icon_themes != NULL ) {
        pd->icon_themes = icon_themes;
    } else {
        set_default_icon_theme ( pd );
    }

    set_key_bindings ( pd );

    return true;
}

static void set_key_bindings ( FileBrowserModePrivateData *pd )
{
    pd->open_custom_key   = OPEN_CUSTOM_KEY;
    pd->open_multi_key    = OPEN_MULTI_KEY;
    pd->toggle_hidden_key = TOGGLE_HIDDEN_KEY;

    FBKey *keys[] = { &pd->open_custom_key,
                      &pd->open_multi_key,
                      &pd->toggle_hidden_key };
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
                    print_err ( "Detected key binding clash. Both \"%s\" and \"%s\" use \"%s\". "
                            "Disabling \"%s\".\n", names[i], names[j], params[i], names[j]);
                }
            }
        }
    }
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

static FBKey get_key_for_name ( char* key_str )
{
    if ( g_strcmp0 ( key_str, "kb-accept-alt" ) == 0 ) {
        return KB_ACCEPT_ALT;
    } else if ( g_str_has_prefix ( key_str, "kb-custom-" ) ) {
        int index = atoi ( key_str + strlen ( "kb-custom-" ) );
        if ( index >= 1 && index <= 20 ) {
            return index - 1;
        }
    }
    return KEY_NONE;
}

static void set_default_icon_theme ( FileBrowserModePrivateData *pd )
{
    char *default_theme = NULL;
    gtk_init(NULL, NULL);
    g_object_get(gtk_settings_get_default(), "gtk-icon-theme-name", &default_theme, NULL);

    if ( default_theme == NULL ) {
        fprintf ( stderr, "[file-browser] Could not determine GTK icon theme. "
                "Maybe try setting a theme with -file-browser-theme\n" );
    }

    char *icon_themes[] = {
        default_theme,
        NULL
    };

    pd->icon_themes = g_strdupv ( icon_themes );
}
