#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <gmodule.h>
#include <rofi/mode.h>
#include <rofi/helper.h>

#include "defaults.h"
#include "types.h"
#include "keys.h"
#include "util.h"

FBKey get_key_for_name ( char* key_str )
{
    if ( g_strcmp0 ( key_str, "kb-accept-alt" ) == 0 ) {
        return KB_ACCEPT_ALT;
    } else if ( g_str_has_prefix ( key_str, "kb-custom-" ) ) {
        key_str += strlen ( "kb-custom-" );
        int index = strtol ( key_str, &key_str, 10 );
        if ( *key_str == '\0' == index >= 1 && index <= 20 ) {
            return KB_CUSTOM_1 + index - 1;
        }
    }
    return KEY_NONE;
}

char *get_name_of_key ( FBKey key ) {
    switch ( key ) {
        case KB_ACCEPT_ALT:
            return g_strdup ( "kb-accept-alt" );
        case KEY_NONE:
            return g_strdup ( "none" );
        default: {
            char buf[23];
            sprintf ( buf, "kb-custom-%d", key + 1 );
            return g_strdup ( buf );
        }
    }
}

FBKey get_key_for_rofi_mretv ( int mretv ) {
    if ( mretv & MENU_CUSTOM_ACTION ) {
        return KB_ACCEPT_ALT;
    } else if ( mretv & MENU_QUICK_SWITCH ) {
        return KB_CUSTOM_1 + ( mretv & MENU_LOWER_MASK );
    }
    return KEY_UNSUPPORTED;
}

void set_key_bindings (
        char *open_custom_key_str,
        char* open_multi_key_str,
        char* toggle_hidden_key_str,
        FileBrowserKeyData *kd )
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
    char  *params[]  = { open_custom_key_str,
                         open_multi_key_str,
                         toggle_hidden_key_str };

    for ( int i = 0; i < 3; i++ ) {
        if ( params[i] != NULL ) {
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
