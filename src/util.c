#include <stdio.h>
#include <gmodule.h>
#include <gio/gio.h>

#include "util.h"

char *get_existing_abs_path ( char *path, char *current_dir )
{
    char *new_path;
    if ( g_file_test ( path, G_FILE_TEST_EXISTS ) ) {
        new_path = g_strdup ( path );
    } else {
        new_path = g_build_filename ( current_dir, path, NULL );
        if ( ! g_file_test ( new_path, G_FILE_TEST_EXISTS ) ) {
            print_err ( "Path does not exist: \"%s\"\n", path );
            g_free ( new_path );
            return NULL;
        }
    }

    /* Canonicalize the path. */
    GFile *file = g_file_new_for_path ( new_path );
    char *canonical_path = g_file_get_path ( file );

    g_object_unref ( file );
    g_free ( new_path );

    return canonical_path;
}

void print_err ( const char *format, ... )
{
    char *new_format = g_strconcat ( "[file-browser] ", format, NULL );
    va_list args;
    va_start ( args, format );
    vfprintf ( stderr, new_format, args );
    va_end ( args );
    g_free ( new_format );
}
