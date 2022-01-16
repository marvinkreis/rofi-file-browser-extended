#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <gmodule.h>
#include <glib/gstdio.h>
#include <rofi/helper.h>

#include "defaults.h"
#include "types.h"
#include "util.h"
#include "options.h"
#include "keys.h"
#include "cmds.h"

/**
 * Read the config file at the given path and store it into the private data.
 */
static void read_config_file ( char* path, FileBrowserModePrivateData *pd );

/**
 * Returns the int argument for the option if it is specified.
 * Otherwise, returns the default value.
 */
static int int_arg_or_default ( char *name, int default_val, FileBrowserModePrivateData *pd );

/**
 * Returns a newly allocated copy of a string argument for the option if it is specified.
 * Otherwise, returns a newly allocated copy of the default value.
 */
static char *str_arg_or_default ( char *name, char *default_val, FileBrowserModePrivateData *pd );

/**
 * Wrapper for find_arg that checks the config file if the option is not specified on the command line.
 */
static bool fb_find_arg ( char* option, FileBrowserModePrivateData *pd );

/**
 * Wrapper for find_arg_int that checks the config file if the option is not specified on the command line.
 */
static bool fb_find_arg_int ( char* option, int *arg, FileBrowserModePrivateData *pd );

/**
 * Wrapper for find_arg_str that checks the config file if the option is not specified on the command line.
 * Returns a newly allocated copy of the string argument.
 */
static bool fb_find_arg_str ( char* option, char **arg, FileBrowserModePrivateData *pd );

/**
 * Wrapper for find_arg_strv that prepends command line arguments to config-file arguments.
 * Returns a newly allocated copy of the string arguments.
 */
static char **fb_find_arg_strv ( char* option, FileBrowserModePrivateData *pd );

/**
 * Returns a newly allocated path to the directory to start the file browser in. This checks, in order,
 * the -file-browser-dir option, the path in the resume file (if resuming is enabled), and the default start path.
 */
static char *get_start_dir ( FileBrowserModePrivateData *pd );

/**
 * Expands a path and returns a newly allocated copy of the canonical absolute path.
 */
static char *expand_start_dir ( char *raw_start_dir );

/**
 * Length of "-file-browser", used to ignore the prefix in the config file.
 */
static const unsigned int FB_LEN = strlen ( "-file-browser-" );

// ================================================================================================================= //

bool set_options ( FileBrowserModePrivateData *pd )
{
    FileBrowserFileData *fd = &pd->file_data;
    FileBrowserIconData *id = &pd->icon_data;
    FileBrowserKeyData  *kd = &pd->key_data;

    pd->config_table = g_hash_table_new_full ( g_str_hash, g_str_equal, g_free, NULL );

    /* Get the config file path and read the config file. */
    const char **config_files = find_arg_strv ( "-file-browser-config" );
    if ( config_files == NULL ) {
        read_config_file ( CONFIG_FILE, pd );
    } else {
        for ( int i = 0; config_files[i] != NULL; i++ ) {
            char *expanded_path = rofi_expand_path ( config_files[i] );
            read_config_file ( expanded_path, pd );
            g_free ( expanded_path );
        }
    }

    fd->follow_symlinks      = fb_find_arg ( "-file-browser-follow-symlinks"     , pd ) ? true  : FOLLOW_SYMLINKS;
    fd->show_hidden          = fb_find_arg ( "-file-browser-show-hidden"         , pd ) ? true  : SHOW_HIDDEN;
    fd->only_dirs            = fb_find_arg ( "-file-browser-only-dirs"           , pd ) ? true  : ONLY_DIRS;
    fd->only_files           = fb_find_arg ( "-file-browser-only-files"          , pd ) ? true  : ONLY_FILES;
    fd->hide_parent          = fb_find_arg ( "-file-browser-hide-parent"         , pd ) ? true  : HIDE_PARENT;
    id->show_icons           = fb_find_arg ( "-file-browser-disable-icons"       , pd ) ? false : SHOW_ICONS;
    id->show_thumbnails      = fb_find_arg ( "-file-browser-disable-thumbnails"  , pd ) ? false : SHOW_THUMBNAILS;
    pd->stdout_mode          = fb_find_arg ( "-file-browser-stdout"              , pd ) ? true  : STDOUT_MODE;
    pd->stdin_mode           = fb_find_arg ( "-file-browser-stdin"               , pd ) ? true  : STDIN_MODE;
    pd->show_status          = fb_find_arg ( "-file-browser-disable-status"      , pd ) ? false : SHOW_STATUS;
    pd->no_descend           = fb_find_arg ( "-file-browser-no-descend"          , pd ) ? true  : NO_DESCEND;
    pd->open_parent_as_self  = fb_find_arg ( "-file-browser-open-parent-as-self" , pd ) ? true  : OPEN_PARENT_AS_SELF;
    pd->search_path_for_cmds = fb_find_arg ( "-file-browser-oc-search-path"      , pd ) ? true  : SEARCH_PATH_FOR_CMDS;
    pd->resume               = fb_find_arg ( "-file-browser-resume"              , pd ) ? true  : RESUME;

    fd->up_text             = str_arg_or_default ( "-file-browser-up-text",            UP_TEXT,            pd );
    id->up_icon             = str_arg_or_default ( "-file-browser-up-icon",            UP_ICON,            pd );
    id->inaccessible_icon   = str_arg_or_default ( "-file-browser-inaccessible-icon",  INACCESSIBLE_ICON,  pd );
    id->fallback_icon       = str_arg_or_default ( "-file-browser-fallback-icon",      FALLBACK_ICON,      pd );
    pd->cmd                 = str_arg_or_default ( "-file-browser-cmd",                CMD,                pd );
    pd->show_hidden_symbol  = str_arg_or_default ( "-file-browser-show-hidden-symbol", SHOW_HIDDEN_SYMBOL, pd );
    pd->hide_hidden_symbol  = str_arg_or_default ( "-file-browser-hide-hidden-symbol", HIDE_HIDDEN_SYMBOL, pd );
    pd->path_sep            = str_arg_or_default ( "-file-browser-path-sep",           PATH_SEP,           pd );
    pd->resume_file         = str_arg_or_default ( "-file-browser-resume-file",        RESUME_FILE,        pd );

    fd->depth = int_arg_or_default ( "-file-browser-depth", DEPTH, pd );

    /* Sort options. */
    /* TODO: make a helper function for "no-..." options and add a "no-..." option for all boolean options. */
    if ( fb_find_arg ( "-file-browser-sort-by-type", pd ) ) {
        fd->sort_by_type = true;
    } else if ( fb_find_arg ( "-file-browser-no-sort-by-type", pd ) ) {
        fd->sort_by_type = false;
    } else {
        fd->sort_by_type = SORT_BY_TYPE;
    }
    if ( fb_find_arg ( "-file-browser-sort-by-depth", pd ) ) {
        fd->sort_by_depth = true;
    } else if ( fb_find_arg ( "-file-browser-no-sort-by-depth", pd ) ) {
        fd->sort_by_depth = false;
    } else {
        fd->sort_by_depth = SORT_BY_DEPTH;
    }

    /* Start directory. */
    fd->current_dir = get_start_dir( pd );
    if ( fd->current_dir == NULL ) {
        return false;
    }

    /* Set glob patterns. */
    char **exclude_globs_strs = fb_find_arg_strv ( "-file-browser-exclude", pd );
    if ( exclude_globs_strs == NULL ) {
        fd->num_exclude_patterns = 0;
    } else {
        int num_globs = count_strv ( ( const char ** ) exclude_globs_strs );
        fd->num_exclude_patterns = num_globs;
        fd->exclude_patterns = g_malloc ( num_globs * sizeof ( GPatternSpec* ) );
        for ( int i = 0; i < num_globs; i++ ) {
            fd->exclude_patterns[i] = g_pattern_spec_new ( exclude_globs_strs[i] );
        }
    }

    /* Set commands for open-custom. */
    char ** cmds = fb_find_arg_strv ( "-file-browser-oc-cmd", pd );
    set_user_cmds(cmds, pd);
    g_strfreev ( cmds );

    /* Set key bindings. */
    char *open_custom_key_str =   str_arg_or_default ( "-file-browser-open-custom-key",   NULL, pd );
    char *open_multi_key_str =    str_arg_or_default ( "-file-browser-open-multi-key",    NULL, pd );
    char *toggle_hidden_key_str = str_arg_or_default ( "-file-browser-toggle-hidden-key", NULL, pd );
    set_key_bindings ( open_custom_key_str, open_multi_key_str, toggle_hidden_key_str, &pd->key_data );
    g_free ( open_custom_key_str );
    g_free ( open_multi_key_str );
    g_free ( toggle_hidden_key_str );

    return true;
}

void destroy_options ( FileBrowserModePrivateData *pd )
{
    GList *list = g_hash_table_get_values ( pd->config_table );
    while ( list != NULL ) {
        g_slist_free_full ( list->data, g_free );
        list = list->next;
    }
    g_hash_table_destroy ( pd->config_table );
}

static void read_config_file ( char *path, FileBrowserModePrivateData *pd )
{
    if ( ! g_file_test ( path, G_FILE_TEST_IS_REGULAR ) ) {
        print_err ( "Could not open config file. \"%s\" does not exist or is not a regular file.\n", path );
        return;
    }

    FILE *file =  g_fopen ( path, "r" );
    if ( file == NULL ) {
        print_err ( "Could not open config file \"%s\".\n", path );
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

        if ( arg != NULL ) {
            g_strchug ( arg );

            /* Remove quotes. */
            if ( arg[0] == '"' || arg[0] == '\'' ) {
                int last_char = strlen ( arg ) - 1;
                if ( arg[last_char] == arg[0] ) {
                    arg[last_char] = '\0';
                    arg++;
                }
            }
        }

        GSList* args = g_hash_table_lookup ( pd->config_table, option );
        args = g_slist_prepend ( args, g_strdup ( arg ) );
        g_hash_table_insert ( pd->config_table, g_strdup ( option ), args );
    }

    g_free ( buffer );
}

static bool fb_find_arg ( char* option, FileBrowserModePrivateData *pd )
{
    if ( find_arg ( option ) != -1 ) {
        return true;
    }

    GSList *list = g_hash_table_lookup ( pd->config_table, &option[FB_LEN] );

    if ( list == NULL ) {
        return false;
    }

    if ( list->next != NULL ) {
        print_err ( "Duplicate option \"%s\" (in config file).\n", option );
    }

    if ( list->data != NULL ) {
        print_err ( "Option \"%s\" (in config file) does not take an argument, got \"%s\".\n", option, list->data );
    }

    return true;
}

static bool fb_find_arg_int ( char* option, int *arg, FileBrowserModePrivateData *pd )
{
    if ( find_arg_int ( option, arg ) ) {
        return true;
    }

    GSList *list = g_hash_table_lookup ( pd->config_table, &option[FB_LEN] );

    if ( list == NULL ) {
        return false;
    }

    if ( list->next != NULL ) {
        print_err ( "Duplicate option \"%s\" (in config file), using last instance.\n", option );
    }

    if ( list->data == NULL ) {
        print_err ( "Missing argument for option \"%s\" (in config file).\n", option );
        return false;
    }

    char* str_arg = list->data, *res;
    *arg = strtol ( str_arg, &res, 10 );

    if ( *res != '\0' ) {
        print_err ( "Argument for option \"%s\" (in config file) must be a number, got: \"%s\".\n", option, str_arg );
        return false;
    }

    return true;
}

static bool fb_find_arg_str ( char* option, char **arg, FileBrowserModePrivateData *pd )
{
    if ( find_arg_str ( option, arg ) ) {
        *arg = g_strdup ( *arg );
        return true;
    }

    GSList *list = g_hash_table_lookup ( pd->config_table, &option[FB_LEN] );

    if ( list == NULL ) {
        return false;
    }

    if ( list->next != NULL ) {
        print_err ( "Duplicate option \"%s\" (in config file), using last instance.\n", option );
    }

    if ( list->data == NULL ) {
        print_err ( "Missing argument for option \"%s\" (in config file).\n", option );
        return false;
    }

    *arg = g_strdup ( list->data );
    return true;
}

static char **fb_find_arg_strv ( char* option, FileBrowserModePrivateData *pd )
{
    const char** cli_args = find_arg_strv ( option );
    GSList *list = g_hash_table_lookup ( pd->config_table, &option[FB_LEN] );

    if ( list == NULL ) {
        char ** retv = g_strdupv ( ( char ** ) cli_args );
        g_free ( cli_args );
        return retv;
    }

    list = g_slist_reverse ( list );

    int num_cli = count_strv ( cli_args );
    int num_file = g_slist_length ( list );

    char **args = g_malloc ( ( num_cli + num_file + 1 ) * sizeof ( char * ) );

    int i = 0;
    while ( i < num_cli ) {
        args[i] = g_strdup ( cli_args[i] );
        i++;
    }
    while ( list != NULL ) {
        if ( list->data == NULL ) {
            print_err ( "Missing argument for option \"%s\" in config file.\n", option );
        } else {
            args[i] = g_strdup ( list->data );
            i++;
        }
        list = list->next;
    }
    args[i] = NULL;

    return args;
}

static char* str_arg_or_default ( char *name, char *default_val, FileBrowserModePrivateData *pd )
{
    char* val;
    if ( fb_find_arg_str ( name, &val, pd ) ) {
        return val;
    } else {
        return g_strdup ( default_val );
    }
}

static int int_arg_or_default ( char *name, int default_val, FileBrowserModePrivateData *pd )
{
    int val;
    if ( fb_find_arg_int ( name, &val, pd ) ) {
        return val;
    } else {
        return default_val;
    }
}

static char *get_start_dir ( FileBrowserModePrivateData *pd )
{
    /* Get start dir from command line option. */
    char *raw_start_dir = NULL;
    if ( fb_find_arg_str ( "-file-browser-dir", &raw_start_dir, pd ) ) {
        char *start_dir = expand_start_dir ( raw_start_dir );
        if ( start_dir != NULL ) {
            return start_dir;
        }
    }

    /* Get start dir from resume file. */
     if ( pd->resume ) {
         char *resume_file_contents = NULL;
         if ( ! g_file_get_contents ( (const char *) pd->resume_file, &resume_file_contents, NULL, NULL ) ) {
             print_err ( "Could not open resume file: \"%s\"\n", pd->resume_file );
         } else {
             /* Skip initial newlines. */
             resume_file_contents += strspn ( resume_file_contents, "\r\n" );
             /* Remove anything after the first actual line. */
             resume_file_contents[ strcspn ( resume_file_contents, "\r\n" ) ] = '\0';

             char *start_dir = expand_start_dir ( resume_file_contents );
             g_free ( resume_file_contents );
             if ( start_dir != NULL ) {
                 return start_dir;
             }
         }
     }

    /* Get the default start dir. */
    return expand_start_dir ( START_DIR );
}

static char *expand_start_dir ( char *raw_start_dir )
{
    char *expanded_path = rofi_expand_path ( raw_start_dir );
    char *abs_path = get_canonical_abs_path ( expanded_path, g_get_current_dir () );
    g_free ( expanded_path );

    if ( ! g_file_test ( abs_path, G_FILE_TEST_EXISTS ) ) {
        print_err ( "Start directory does not exist: \"%s\".\n", raw_start_dir );
        return NULL;
    }
    if ( ! g_file_test ( abs_path, G_FILE_TEST_IS_DIR ) ) {
        print_err ( "Start directory is not a directory: \"%s\".\n", raw_start_dir );
        return NULL;
    }

    return abs_path;
}

bool write_resume_file ( FileBrowserModePrivateData *pd )
{
    if ( ! pd->resume ) {
        return true;
    }

    char* file_content = g_strconcat ( pd->file_data.current_dir, "\n", NULL );
    if ( ! g_file_set_contents ( pd->resume_file, file_content, -1, NULL ) ) {
        print_err ( "Could not write new path to the resume file: \"%s\"", pd-> resume_file );
        return false;
    }
    g_free ( file_content );
    return true;
}
