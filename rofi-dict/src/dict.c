/**
 * rofi-dict
 *
 * MIT/X11 License
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
 * Reads all json-files in a directory as dictionaries and lets the user query values by keys.
 * Dictionaries must be saved as utf-8 and have the following structure:
 *      {
 *        "key1": "value1",
 *        "key2": "value2",
 *        "key3": "value3"
 *      }
 *
 * Supports three matching modes:
 *      exact matching
 *      substring matching
 *      Levenshtein distance matching
 *
 *
 * Key Bindings:
 * ------------
 * kb-mode-next     (default: Shift+Right)    | Switch to the next matching mode. Switch to the next rofi-mode if the last matching mode is already selected.
 * kb-mode-previous (default: Shift+Left)     | Switch to the previous matching mode. Switch to the previous rofi-mode if the first matching mode is already selected.
 * kb-accept-custom (default: Control+Return) | Switch to the next matching mode (only if the input is empty).
 *
 * Command line options:
 * ---------------------
 * -dict_path '/path/to/directory'     Sets the directory containing the dictionaries.
 * -dict_mode <0|1|2>                  Sets the matching mode.
 */

// ================================================================================================================= //

#define _GNU_SOURCE

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <dirent.h>

#include <rofi/mode.h>
#include <rofi/helper.h>
#include <rofi/mode-private.h>

#include <gmodule.h>
#include <json.h>

// ================================================================================================================= //

/* The dictionary path. The default path is XDG_DATA_HOME/dicts/json
   The path ist free'd using g_free() afterwards, therefore g_strdup(PATH) should be used for plain strings.
   g_get_user_data_dir() can be used to get the home directory.
   Alternatively, the path can be set with the -dict_path command line option. */
#define DICTIONARY_PATH g_build_filename(g_get_user_data_dir(), "dicts", "json", NULL)

/* The default matching mode.
   Possible modes are:
        match_exact:        match if the query is equal to the key (case insensitive)
        match_substring:    match if the query is a substring of the key (case insensitive)
        match_levenshtein:  match if the Levenshtein distance between query and key is <= LEVENSHTEIN_THRESHOLD
   The matching mode can be changed at runtime via MENU_PREVIOUS or MENU_NEXT. */
#define DEFAULT_MATCHING_MODE match_exact

/* The threshold for the Levenshtein matching mode. A query will match a key if the Levenshtein distance between
   the query and key is <= LEVENSHTEIN_THRESHOLD. */
#define LEVENSHTEIN_THRESHOLD 2

/* Keep all current results when typing a new search string. If set to false, rofi will filter the current results
   with the new search string. */
#define KEEP_ALL_RESULTS true

/* Display the dictionary name in front of the result. Useful when using multiple dictionaries. */
#define DISPLAY_DICT_NAME true

/* Spacing (number of whitespaces) to be used between the dictionary name, key and value of each result. */
#define SPACING 2

/* Use filling in the result strings, so the keys and values of the results align vertically, when multiple results
   are found. */
#define USE_FILLING true

/* Use the message line to display current mode and search string. */
#define USE_MESSAGE true

/* Format of the message. Arguments are: "current mode", "search string" */
#define MESSAGE_FORMAT "%s %s"

/* Format of the results. Arguments are: "dictionary name", "spacing", "key", "spacing", "value"
   If DISPLAY_DICT_NAME is false, "dictionary name" and the first "spacing" will be omitted. */
#define RESULT_FORMAT "<span alpha='50%%'>%s</span>%s<span background='#FFFFFF12'>%s</span>%s%s"

/* Symbols to use for displaying the matching mode in the message bar. */
#define EXACT_MATCH_SYMBOL "[-]"
#define SUBSTRING_MATCH_SYMBOL "[+]"
#define LEVENSHTEIN_MATCH_SYMBOL "[*]"

// ================================================================================================================= //

G_MODULE_EXPORT Mode mode;

typedef struct {
    /* Name of the dictionary */
    const char* dict_name;
    /* Key */
    const char* key;
    /* Value belonging to the key */
    const char* value;
} Result;

typedef enum match_mode {
    match_exact,
    match_substring,
    match_levenshtein,
} match_mode;

typedef struct {
    /* Dictionaries */
    json_object** json_dicts;
    /* Dictionary names */
    char** dict_names;
    /* Number of dictionaries */
    unsigned int num_dicts;
    /* Maximum string length of dictionary names */
    unsigned int max_dict_len;

    /* Results */
    Result* results;
    /* Number of results */
    unsigned int num_results;
    /* Maximum string length of the results' keys. */
    unsigned int max_key_len;

    /* Current search string */
    char* search;
    /* Current matching mode. */
    match_mode cur_match_mode;
} DictModePrivateData;

// ================================================================================================================= //

/**
 * @param dir_name The directory to find the dictionaries in.
 * @param sw The current mode.
 *
 * Sets all json file names of dictionaries.
 * Initializes pd->dict_names and sets pd->num_dicts.
 *
 * @return true on success, false otherwise.
 */
bool set_dict_files ( const char* dir_name, Mode* sw );

/**
 * @param dir_name The directory of the dictionaries.
 * @param sw The current mode.
 *
 * Reads the json files and stores the json objects in the mode's private data.
 * Allocates pd->json_dicts.
 */
void set_json_dicts ( const char* dir_name, Mode* sw );

/**
 * @param search The query to search for.
 * @param sw The current mode.
 *
 * Searches through the json dictionaries for matching entries and stores them in the private data.
 * Clears old results and re-allocates and pd->results. Sets pd->num_results.
 * and pd->max_key_len.
 */
void set_results( const char* search, Mode* sw );

/**
 * @param a The first result.
 * @param b The second result.
 * @param search The search string.
 *
 * Comparison function for sorting the results when Levenshtein matching is used.
 * Results are ordered by their Levenshtein distance.
 */
static gint levenshtein_compare ( gconstpointer a, gconstpointer b, gpointer search );

/**
 * @param a The first result.
 * @param b The second result.
 * @param search The search string.
 *
 * Comparison function for sorting the results when substring matching is used.
 * Exact results should always be first, the rest are un-sorted.
 */
static gint substring_compare ( gconstpointer a, gconstpointer b, gpointer search );

// ================================================================================================================= //

static int dict_init ( Mode* sw )
{
    if ( mode_get_private_data ( sw ) == NULL ) {

        DictModePrivateData* pd = g_malloc0 ( sizeof ( DictModePrivateData ) );
        mode_set_private_data ( sw, ( void * ) pd );

        /* Get the matching mode. */
        unsigned int match_mode;
        if ( find_arg_uint ( "-dict_mode", &match_mode ) && match_mode <= 2 ) {
            pd->cur_match_mode = match_mode;
        } else {
            pd->cur_match_mode = DEFAULT_MATCHING_MODE;
        }

        /* Get the directory containing the dictionaries. */
        char* dir_name = NULL;
        if ( find_arg_str ( "-dict_path", &dir_name ) ) {
            dir_name = g_strdup ( dir_name );
        } else {
            dir_name = DICTIONARY_PATH;
        }

        /* Get dictionary names and number. */
        if ( !set_dict_files ( dir_name, sw ) ) {
            return false;
        }

        /* Parse the dictionary json files. */
        set_json_dicts ( dir_name, sw );

        /* Set the maximum string length of the dictionary names. */
        for ( int i = 0; i < pd->num_dicts; i++ ) {
            unsigned int dict_len = mbstowcs ( NULL, pd->dict_names[i], 0 );
            if ( dict_len > pd->max_dict_len ) {
                pd->max_dict_len = dict_len;
            }
        }

        g_free ( dir_name );

        char* search = NULL;
        if ( find_arg_str ( "-dict_search", &search ) ) {
            pd->search = g_strdup ( search );
            set_results ( search, sw );
        }
    }

    return true;
}

static void dict_destroy ( Mode* sw )
{
    DictModePrivateData* pd = ( DictModePrivateData * ) mode_get_private_data ( sw );
    mode_set_private_data ( sw, NULL );

    if ( pd != NULL ) {

        /* Free dictionaries and their names. */
        for ( int i = 0; i < pd->num_dicts; i++ ) {
            g_free ( pd->dict_names[i] );
            if ( pd->json_dicts[i] != NULL ) {
                json_object_put ( pd->json_dicts[i] );
            }
        }
        g_free ( pd->dict_names );
        g_free ( pd->json_dicts );

        /* Free results. */
        g_free ( pd->results );

        /* Free search. */
        g_free ( pd->search );

        /* Fill with zeros, just in case. */
        memset ( ( void * ) pd , 0, sizeof ( pd ) );

        g_free ( pd );
    }
}

static unsigned int dict_get_num_entries ( const Mode* sw )
{
    const DictModePrivateData* pd = ( const DictModePrivateData * ) mode_get_private_data ( sw );

    if ( pd != NULL ) {
        return pd->num_results;
    } else {
        return 0;
    }
}

static ModeMode dict_result ( Mode* sw, int mretv, char** input, unsigned int selected_line )
{
    DictModePrivateData* pd = ( DictModePrivateData * ) mode_get_private_data ( sw );

    ModeMode retv = RELOAD_DIALOG;

    /* Default actions */
    if ( mretv & MENU_CANCEL ) {
        retv = MODE_EXIT;
    } else if ( ( mretv & MENU_NEXT ) && pd->cur_match_mode >= 2 ) {
        retv = NEXT_DIALOG;
    } else if ( ( mretv & MENU_PREVIOUS ) && pd->cur_match_mode <= 0 ) {
        retv = PREVIOUS_DIALOG;
    } else if ( mretv & MENU_QUICK_SWITCH ) {
        retv = ( mretv & MENU_LOWER_MASK );

    /* Handle match mode switching Control+Return. */
    } else {

        /* Handle match mode switching Control+Return. */
        if ( ( mretv & MENU_CUSTOM_INPUT ) && strlen ( *input ) == 0 ) {
            pd->cur_match_mode = ( pd->cur_match_mode + 1 ) % 3;

        /* Handle match mode switching Shift+Left and Shift+Right. */
        } else if ( ( mretv & MENU_NEXT ) && pd->cur_match_mode <= 1 ) {
            pd->cur_match_mode = ( pd->cur_match_mode + 1 ) % 3;
        } else if ( ( mretv & MENU_PREVIOUS ) && pd->cur_match_mode >= 1 ) {
            pd->cur_match_mode = ( pd->cur_match_mode + 2 ) % 3;
        }

        char* search = NULL;

        /* Use the old search string if no input provided. */
        if ( strlen ( *input ) == 0 ) {
            search = pd->search;
        } else {
            search = *input;
            g_free ( pd->search );
            pd->search = g_strdup ( search );
        }

        if (search != NULL) {
            set_results ( search, sw );
        }

    }

    return retv;
}

static int dict_token_match ( const Mode* sw, rofi_int_matcher** tokens, unsigned int index )
{
    const DictModePrivateData* pd = ( const DictModePrivateData * ) mode_get_private_data ( sw );

#if KEEP_ALL_RESULTS
    return true;
#else
    return helper_token_match ( tokens, pd->results[index] );
#endif
}

static char* dict_get_display_value ( const Mode* sw, unsigned int selected_line, int* state,
        GList** attr_list, int get_entry )
{
    if ( !get_entry ) return NULL;

    // MARKUP flag, not defined in accessible headers
    *state |= 8;

    const DictModePrivateData* pd = ( const DictModePrivateData * ) mode_get_private_data ( sw );

    Result* result = NULL;
    result = &( pd->results[selected_line] );

    unsigned int dict_len = mbstowcs ( NULL, result->dict_name, 0 );
#if USE_FILLING
    char* dict_fill = g_strnfill ( SPACING + pd->max_dict_len - dict_len , ' ' );
#else
    char* dict_fill = g_strnfill ( SPACING );
#endif

    //TODO make macros prettier with if-else

    unsigned int key_len = mbstowcs ( NULL, result->key, 0 );
#if USE_FILLING
    char* key_fill = g_strnfill ( SPACING + pd->max_key_len - key_len , ' ' );
#else
    char* key_fill = g_strnfill ( SPACING );
#endif

    char* format_str = g_markup_printf_escaped ( RESULT_FORMAT,
#if DISPLAY_DICT_NAME
                                             result->dict_name,
                                             dict_fill,
#endif
                                             result->key,
                                             key_fill,
                                             result->value );

    g_free ( dict_fill );
    g_free ( key_fill );

    return format_str;
}

static char* dict_get_message ( const Mode *sw )
{
    const DictModePrivateData* pd = ( const DictModePrivateData * ) mode_get_private_data ( sw );

    static char* mode_symbols[3] = { EXACT_MATCH_SYMBOL, SUBSTRING_MATCH_SYMBOL, LEVENSHTEIN_MATCH_SYMBOL };

    char* search = ( pd->search != NULL ) ? pd->search : "";
    return g_markup_printf_escaped ( MESSAGE_FORMAT, mode_symbols[pd->cur_match_mode], search );
}

// ================================================================================================================= //

bool set_dict_files ( const char* dir_name, Mode* sw )
{
    DictModePrivateData* pd = ( DictModePrivateData * ) mode_get_private_data ( sw );

    DIR* dir;
    struct dirent *entry;

    dir = opendir ( dir_name );
    if ( dir == NULL ) {
        fprintf ( stderr, "[dict] Could not open directory: %s\n", dir_name );
        return false;
    }

    /* Gather all regular .json-files. */
    while ( ( entry = readdir( dir ) ) != NULL ) {
        if ( entry->d_type == DT_REG && g_str_has_suffix ( entry->d_name, ".json") ) {
            pd->dict_names = g_realloc ( pd->dict_names, (pd->num_dicts + 1) * sizeof ( char * ) );
            pd->dict_names[pd->num_dicts] = g_filename_to_utf8 ( entry->d_name, -1, NULL, NULL, NULL );
            pd->num_dicts++;
        }
    }

    closedir ( dir );

    return true;
}

void set_json_dicts ( const char* dir_name, Mode* sw )
{
    DictModePrivateData* pd = ( DictModePrivateData * ) mode_get_private_data ( sw );

    pd->json_dicts = g_malloc0 ( pd->num_dicts * sizeof ( json_object* ) );

    for (int i = 0; i < pd->num_dicts; i++) {
        char* file_name = g_build_filename ( dir_name, pd->dict_names[i], NULL );
        char* file_content = NULL;

        if ( !g_file_get_contents ( file_name, &file_content, NULL, NULL ) ) {
            fprintf ( stderr, "[dict] Could not read file: %s\n", pd->dict_names[i] );
            continue;
        }

        enum json_tokener_error error;
        json_object *dict = json_tokener_parse_verbose ( file_content, &error );
        if ( error != json_tokener_success ) {
            fprintf ( stderr, "[dict] Could parse json file: %s\n", pd->dict_names[i] );
            continue;
        }
        pd->json_dicts[i] = dict;

        /* Strip the file extension off the dictionary name, so it can be used for displaying later. */
        char *dot = strrchr ( pd->dict_names[i], '.' );
        if ( dot != NULL ) *dot = '\0';

        g_free ( file_name );
        g_free ( file_content );
    }
}

void set_results ( const char* search, Mode* sw )
{
    DictModePrivateData* pd = ( DictModePrivateData * ) mode_get_private_data ( sw );

    /* Clear previous results. */
    pd->num_results = 0;
    pd->max_key_len = 0;
    g_free ( pd->results );
    pd->results = NULL;

    /* Iterate over all json_objects in all dictionaries to find results. */
    for ( int i = 0; i < pd->num_dicts; i++ ) {
        if ( pd->json_dicts[i] == NULL ) {
            continue;
        }

        json_object_object_foreach ( pd->json_dicts[i], key, value ) {

            bool match = false;

            switch ( pd->cur_match_mode ) {
                case match_exact:
                    match = ( strcasecmp ( key, search ) == 0 );
                    break;
                case match_substring:
                    match = ( strcasestr ( key, search ) != NULL );
                    break;
                case match_levenshtein:
                    match = ( levenshtein ( search, mbstowcs( NULL, search, 0 ), key, mbstowcs( NULL, key, 0 ) ) <= LEVENSHTEIN_THRESHOLD );
                    break;
            }

            if ( match ) {
                Result* result = NULL;

                pd->results = g_realloc ( pd->results, (pd->num_results + 1) * sizeof ( Result ) );
                result = &( pd->results[pd->num_results] );
                pd->num_results++;

                result->dict_name = pd->dict_names[i];
                result->key = key;
                result->value = ( char * ) json_object_get_string ( value );

                unsigned int key_len = mbstowcs ( NULL, key, 0 );
                if ( key_len > pd->max_key_len ) {
                    pd->max_key_len = key_len;
                }
            }
        }
    }

    switch ( pd->cur_match_mode ) {
        case match_substring:
            g_qsort_with_data ( pd->results, pd->num_results, sizeof ( Result ), substring_compare, pd->search );
            break;
        case match_levenshtein:
            g_qsort_with_data ( pd->results, pd->num_results, sizeof ( Result ), levenshtein_compare, pd->search );
            break;
        default:
            break;
    }
}

static gint substring_compare ( gconstpointer a, gconstpointer b, gpointer search )
{
    Result *trans_a = ( Result * ) a;
    Result *trans_b = ( Result * ) b;

    bool exact_match_a = ( strcasecmp ( trans_a->key, search ) == 0 );
    bool exact_match_b = ( strcasecmp ( trans_b->key, search ) == 0 );

    if ( exact_match_a ^ exact_match_b ) {
        return exact_match_a ? -1 : 1;
    } else {
        return 0;
    }
}

static gint levenshtein_compare ( gconstpointer a, gconstpointer b, gpointer search )
{
    Result *trans_a = ( Result * ) a;
    Result *trans_b = ( Result * ) b;

    int levenshtein_a = levenshtein ( search,   mbstowcs ( NULL, search, 0 ),
                                      trans_a->key, mbstowcs ( NULL, trans_a->key, 0 ) );
    int levenshtein_b = levenshtein ( search,   mbstowcs ( NULL, search, 0 ),
                                      trans_b->key, mbstowcs ( NULL, trans_b->key, 0 ) );

    return (levenshtein_a - levenshtein_b);
}

// ================================================================================================================= //

Mode mode = {
    .abi_version        = ABI_VERSION,
    .name               = "dict",
    .cfg_name_key       = "display-dict",
    ._init              = dict_init,
    ._destroy           = dict_destroy,
    ._get_num_entries   = dict_get_num_entries,
    ._result            = dict_result,
    ._token_match       = dict_token_match,
    ._get_display_value = dict_get_display_value,
    ._get_icon          = NULL,
    ._get_completion    = NULL,
    ._preprocess_input  = NULL,
#if USE_MESSAGE
    ._get_message       = dict_get_message,
#else
    ._get_message       = NULL,
#endif
    .private_data       = NULL,
    .free               = NULL,
};
