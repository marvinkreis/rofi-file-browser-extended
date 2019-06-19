#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <gmodule.h>
#include <rofi/mode.h>

#include "types.h"
#include "keys.h"

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

FBKey get_key_for_rofi_mretv(int mretv) {
    if ( mretv & MENU_CUSTOM_ACTION ) {
        return KB_ACCEPT_ALT;
    } else if ( mretv & MENU_QUICK_SWITCH ) {
        return KB_CUSTOM_1 + ( mretv & MENU_LOWER_MASK );
    }
    return -1;
}
