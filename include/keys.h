#ifndef FILE_BROWSER_KEYS_H
#define FILE_BROWSER_KEYS_H

#include "types.h"

/**
 * Returns the FBKey for a key name.
 * If the name is invalid (or unsupported), returns KEY_NONE;
 */
FBKey get_key_for_name ( char* key_str );

/**
 * Returns the (newly allocated) name of a FBKey.
 */
char *get_name_of_key ( FBKey key );

/**
 * Returns the FBKey for the rofi menu return value.
 * Returns -1 if it does not match any FBKey.
 */
FBKey get_key_for_rofi_mretv ( int mretv );

/**
 * Sets the key bindings from the command line options.
 * Prints error messages in case of invalid or clashing key bindings.
 * Missing command line options are passed as NULL.
 */
void set_key_bindings (
        char *open_bookmarks_key_str,
        char *open_custom_key_str,
        char* open_multi_key_str,
        char* toggle_hidden_key_str,
        FileBrowserKeyData *kd );

#endif
