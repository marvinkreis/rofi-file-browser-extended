#include <gmodule.h>

#include "defaults.h"
#include "types.h"
#include "cmds.h"
#include "util.h"


/**
 * Add an array of cmds to the ones currently stored in the private data.
 */
static void add_custom_cmds ( FBCmd* cmds, int num_cmds, FileBrowserModePrivateData *pd );

/**
 * Function used to add cmds to an array, iterating over a hash table.
 */
static gboolean add_cmd ( gpointer cmd, G_GNUC_UNUSED gpointer unused, gpointer pd );

/**
 * Compares the command strings of two cmds in lexicographic order.
 */
static gint compare_cmds ( gconstpointer a, gconstpointer b, G_GNUC_UNUSED gpointer data );

// ================================================================================================================= //

static void add_custom_cmds ( FBCmd* cmds, int num_cmds, FileBrowserModePrivateData *pd )
{
    pd->cmds = g_realloc ( pd->cmds, ( pd->num_cmds + num_cmds ) * sizeof ( FBCmd ) );
    memcpy ( &pd->cmds[pd->num_cmds], cmds, num_cmds * sizeof ( FBCmd ) );
    pd->num_cmds += num_cmds;
    pd->show_cmds = pd->num_cmds > 0;
}

void set_open_custom_cmds ( char** cmd_strs, FileBrowserModePrivateData *pd )
{
    if ( cmd_strs == NULL ) {
        return;
    }

    static int icon_sep_len = strlen ( OPEN_CUSTOM_CMD_ICON_SEP );
    static int name_sep_len = strlen ( OPEN_CUSTOM_CMD_NAME_SEP );

    /* Custom commands for open-custom prompt. */
    int num_cmds;
    for ( num_cmds = 0; cmd_strs[num_cmds] != NULL; num_cmds++ ) { }
    FBCmd *cmds = g_malloc ( num_cmds * sizeof ( FBCmd ) );

    for ( int i = 0; i < num_cmds; i++ ) {
        char* cmd = cmd_strs[i];
        char* icon_name = g_strrstr ( cmd, OPEN_CUSTOM_CMD_ICON_SEP );
        char* name = g_strrstr ( cmd, OPEN_CUSTOM_CMD_NAME_SEP );

        if ( icon_name != NULL ) {
            *icon_name = '\0';
        }
        if ( name != NULL ) {
            *name = '\0';
        }

        FBCmd *fbcmd = &cmds[i];
        fbcmd->cmd = g_strdup ( cmd );
        fbcmd->icon_name = icon_name == NULL ? NULL : g_strdup ( &icon_name[icon_sep_len] );
        fbcmd->name = name == NULL ? NULL : g_strdup ( &name[name_sep_len] );
        fbcmd->icon = NULL;
    }

    add_custom_cmds ( cmds, num_cmds, pd );
    g_free ( cmds );
}

void destroy_cmds ( FileBrowserModePrivateData *pd ) {
    for ( int i = 0; i < pd->num_cmds; i++ ) {
        g_free( pd->cmds[i].cmd );
        g_free( pd->cmds[i].icon_name );
        g_free( pd->cmds[i].name );
    }
    g_free ( pd->cmds );
    pd->cmds = NULL;
    pd->num_cmds = 0;
    pd->show_cmds = false;
}

void find_custom_cmds ( FileBrowserModePrivateData *pd ) {
    char *path = g_strdup ( g_getenv ( "PATH" ) );
    if ( path == NULL ) {
        print_err ( "Could not get $PATH environment variable to search for executables." );
        return;
    }

    GHashTable *table = g_hash_table_new_full ( g_str_hash, g_str_equal, g_free, NULL );

    char* dirname = strtok ( path, G_SEARCHPATH_SEPARATOR_S );

    while ( dirname != NULL ) {
        GDir *dir = g_dir_open ( dirname, 0, NULL );

        if ( dir == NULL ) {
            print_err ( "Could not open directory \"%s\" in $PATH to search for executables.", dirname );

        } else {
            const char *filename;

            while ( ( filename = g_dir_read_name ( dir ) ) ) {
                char c0 = filename[0];
                if ( ( c0 >= '0' && c0 <= '9' ) || ( c0 >= 'a' && c0 <= 'z' ) || ( c0 >= 'A' && c0 <= 'Z' ) ) {
                    g_hash_table_insert ( table, g_strdup ( filename ), NULL );
                }
            }

            g_dir_close ( dir );
        }

        dirname = strtok ( NULL, G_SEARCHPATH_SEPARATOR_S );
    }

    g_free ( path );

    struct { FBCmd* data; int num; } arr;
    arr.data = malloc ( g_hash_table_size ( table ) * sizeof ( FBCmd ) );
    arr.num = 0;

    g_hash_table_foreach_steal ( table, add_cmd, &arr );
    g_hash_table_destroy ( table );

    g_qsort_with_data ( arr.data, arr.num, sizeof ( FBCmd ), compare_cmds, NULL );

    add_custom_cmds( arr.data, arr.num, pd );

    g_free ( arr.data );
}

static gboolean add_cmd ( gpointer key, G_GNUC_UNUSED gpointer value, gpointer data ) {
    char *cmdstr = key;
    struct { FBCmd* data; int num; } *arr = data;

    FBCmd *fbcmd = &arr->data[arr->num];
    fbcmd->cmd = cmdstr;
    fbcmd->name = NULL;
    fbcmd->icon_name = NULL;
    fbcmd->icon = NULL;

    arr->num++;
    return true;
}

static gint compare_cmds ( gconstpointer a, gconstpointer b, G_GNUC_UNUSED gpointer data ) {
    const FBCmd *ca = a;
    const FBCmd *cb = b;
    return g_strcmp0 ( ca->cmd, cb->cmd );
}
