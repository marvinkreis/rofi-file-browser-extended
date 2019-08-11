#include <gmodule.h>

#include "defaults.h"
#include "types.h"
#include "cmds.h"
#include "util.h"

static gboolean add_cmd ( gpointer cmd, G_GNUC_UNUSED gpointer unused, gpointer pd );

static void add_custom_cmds ( OpenCustomCMD* cmds, int num_cmds, FileBrowserModePrivateData *pd );

typedef struct {
    unsigned int num;
    unsigned int size;
    OpenCustomCMD *data;
} CMDArray;

static gint compare_cmds ( gconstpointer a, gconstpointer b, G_GNUC_UNUSED gpointer data );

// ================================================================================================================= //

static void add_custom_cmds ( OpenCustomCMD* cmds, int num_cmds, FileBrowserModePrivateData *pd ) {
    if ( num_cmds <= 0) {
        return;
    }

    if ( pd->cmds == NULL ) {
        pd->cmds = g_malloc ( num_cmds * sizeof ( OpenCustomCMD ) );
    } else {
        pd->cmds = g_realloc ( pd->cmds, ( pd->num_cmds + num_cmds ) * sizeof ( OpenCustomCMD ) );
    }

    memcpy ( &pd->cmds[pd->num_cmds], cmds, num_cmds * sizeof ( OpenCustomCMD ) );
    pd->num_cmds += num_cmds;
}

void set_open_custom_cmds ( char** cmd_strs, FileBrowserModePrivateData *pd )
{
    static int icon_sep_len = strlen ( OPEN_CUSTOM_CMD_ICON_SEP );
    static int name_sep_len = strlen ( OPEN_CUSTOM_CMD_NAME_SEP );

    /* Custom commands for open-custom prompt. */
    int num_cmds;
    for ( num_cmds = 0; cmd_strs[num_cmds] != NULL; num_cmds++ ) { }
    OpenCustomCMD *cmds = g_malloc ( num_cmds * sizeof ( OpenCustomCMD ) );

    for ( int i = 0; i < num_cmds; i++ ) {
        char* cmdstr = cmd_strs[i];
        OpenCustomCMD *cmd = &cmds[i];

        char* icon_name = g_strrstr ( cmdstr, OPEN_CUSTOM_CMD_ICON_SEP );
        char* name = g_strrstr ( cmdstr, OPEN_CUSTOM_CMD_NAME_SEP );

        if ( icon_name != NULL ) {
            *icon_name = '\0';
        }
        if ( name != NULL ) {
            *name = '\0';
        }

        cmd->cmd = g_strdup ( cmdstr );
        cmd->icon_name = icon_name == NULL ? NULL : g_strdup ( &icon_name[icon_sep_len] );
        cmd->name = name == NULL ? NULL : g_strdup ( &name[name_sep_len] );
        cmd->icon = NULL;
    }

    add_custom_cmds( cmds, num_cmds, pd );
}

void destroy_open_custom_cmds ( FileBrowserModePrivateData *pd ) {
    if ( pd->num_cmds > 0 ) {
        for ( int i = 0; i < pd->num_cmds; i++ ) {
            g_free( pd->cmds[i].cmd );
            g_free( pd->cmds[i].icon_name );
            g_free( pd->cmds[i].name );
        }
        pd->num_cmds = 0;
    }
    g_free ( pd->cmds );
    pd->cmds = NULL;
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

    CMDArray arr;
    arr.size = g_hash_table_size ( table );
    arr.data = malloc ( arr.size * sizeof ( OpenCustomCMD ) );
    arr.num = 0;

    g_hash_table_foreach_steal ( table, add_cmd, &arr );
    g_hash_table_destroy ( table );

    g_qsort_with_data ( arr.data, arr.num, sizeof ( OpenCustomCMD ), compare_cmds, NULL );

    add_custom_cmds( arr.data, arr.num, pd );

    g_free ( arr.data );
}

static gboolean add_cmd ( gpointer key, G_GNUC_UNUSED gpointer value, gpointer data ) {
    char *cmdstr = ( char * ) key;
    CMDArray *arr = ( CMDArray * ) data;

    OpenCustomCMD *cmd = &arr->data[arr->num];
    cmd->cmd = cmdstr;
    cmd->name = NULL;
    cmd->icon_name = NULL;
    cmd->icon = NULL;

    arr->num++;
    return true;
}

static gint compare_cmds ( gconstpointer a, gconstpointer b, G_GNUC_UNUSED gpointer data ) {
    OpenCustomCMD *ca = ( OpenCustomCMD * ) a;
    OpenCustomCMD *cb = ( OpenCustomCMD * ) b;
    return g_strcmp0 ( ca->cmd, cb->cmd );
}
