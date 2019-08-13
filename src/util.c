#include <stdio.h>
#include <stdarg.h>
#include <gmodule.h>
#include <gio/gio.h>

#include "util.h"

/**
 * Returns the canonical version of the given path.
 */
static char *canonicalize_path ( char* path );

// ================================================================================================================= //

char *get_canonical_abs_path ( char *path, char *current_dir )
{
    if ( g_path_is_absolute ( path ) ) {
        return canonicalize_path ( path );
    } else {
        char* abs_path = g_build_filename ( current_dir, path, NULL );
        char *canonical_path = canonicalize_path ( abs_path );
        g_free ( abs_path );
        return canonical_path;
    }
}

static char *canonicalize_path ( char* path ) {
    GFile *file = g_file_new_for_path ( path );
    char *canonical_path = g_file_get_path ( file );
    g_object_unref ( file );
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

unsigned int count_strv ( const char **array ) {
    if ( array == NULL ) {
        return 0;
    }
    int num = 0;
    while ( array[num] != NULL ) {
        num++;
    }
    return num;
}
