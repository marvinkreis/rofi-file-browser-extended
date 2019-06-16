#include <stdio.h>
#include <gmodule.h>

#include "util.h"

char *get_existing_abs_path ( char *path, char *current_dir )
{
    char *abs_path = g_canonicalize_filename ( path, current_dir );
    if ( abs_path == NULL ) {
        print_err ( "Invalid path: '%s'\n", path );
        return NULL;
    }
    if ( ! g_file_test ( abs_path, G_FILE_TEST_EXISTS ) ) {
        print_err ( "Path does not exist: '%s'\n", path );
        g_free ( abs_path );
        return NULL;
    }
    return abs_path;
}

void print_err ( const char *format, ... )
{
    format = g_strconcat ( "[file-browser] ", format, NULL );
    va_list args;
    va_start ( args, format );
    vfprintf ( stderr, format, args );
    va_end ( args );
}
