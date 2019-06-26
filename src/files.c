#define _XOPEN_SOURCE 700
#define _GNU_SOURCE

#include <gmodule.h>

#include "types.h"
#include "util.h"
#include "files.h"

/**
 * Save file browser data globally so nftw's callback can access it.
 */
static FileBrowserFileData* global_pd;

/**
 * Frees the current files (but keeps the actual file array intact).
 */
static void free_files ( FileBrowserFileData *fd );

/**
 * Function used by nftw to add files to the list recursively.
 */
static inline int add_file ( const char *fpath, G_GNUC_UNUSED const struct stat *sb, int typeflag, struct FTW *ftwbuf );

/**
 * Compares files alphabetically.
 */
static inline gint compare_files ( gconstpointer a, gconstpointer b, gpointer data );

/**
 * Compares files to sort by type (directories first, inaccessible files last).
 * Then compares files alphabetically.
 */
static inline gint compare_files_type ( gconstpointer a, gconstpointer b, gpointer data );

/**
 * Compares files to sort by depth.
 * Then compares files alphabetically.
 */
static inline gint compare_files_depth ( gconstpointer a, gconstpointer b, gpointer data );

/**
 * Compares files to sort by depth.
 * Then compares files to sort by type (directories first, inaccessible files last).
 * Then compares files alphabetically.
 */
static inline gint compare_files_depth_type ( gconstpointer a, gconstpointer b, gpointer data );

/**
 * Directories appear before regular files, inaccessible directories and files appear last.
 * Files of the same type are sorted alphabetically.
 */

// ================================================================================================================= //

void free_files ( FileBrowserFileData *fd )
{
    FBFile *files = fd->files;
    for ( unsigned int i = 0; i < fd->num_files; i++ ) {
        g_free ( files[i].path );
    }
    fd->num_files = 0;
    fd->size_files = 0;
}

void destroy_files ( FileBrowserFileData *fd )
{
    free_files( fd );
    g_free ( fd->current_dir );
    g_free ( fd->files );
    g_free ( fd->up_text );
    fd->current_dir = NULL;
    fd->files = NULL;
    fd->up_text = NULL;
}

void load_files ( FileBrowserFileData *fd )
{
    free_files ( fd );

    /* Insert the parent dir. */
    fd->files = g_realloc ( fd->files, sizeof ( FBFile ) );
    fd->size_files = 1;
    fd->num_files = 1;

    FBFile *up = &fd->files[0];
    up->type = UP;
    up->name = fd->up_text;
    up->path = g_build_filename ( fd->current_dir, "..", NULL );
    up->depth = 0;
    up->icon = NULL;

    /* Load the files. */
    global_pd = fd;
    nftw ( fd->current_dir, add_file, 16, FTW_ACTIONRETVAL );

    /* Sort all but the parent dir. */
    if ( fd->sort_by_type ) {
        if ( fd->sort_by_depth ) {
            g_qsort_with_data ( &fd->files[1], fd->num_files - 1, sizeof ( FBFile ), compare_files_depth_type, NULL );
        } else {
            g_qsort_with_data ( &fd->files[1], fd->num_files - 1, sizeof ( FBFile ), compare_files_type, NULL );
        }
    } else {
        if ( fd->sort_by_depth ) {
            g_qsort_with_data ( &fd->files[1], fd->num_files - 1, sizeof ( FBFile ), compare_files_depth, NULL );
        } else {
            g_qsort_with_data ( &fd->files[1], fd->num_files - 1, sizeof ( FBFile ), compare_files, NULL );
        }
    }
}

void change_dir ( char *path, FileBrowserFileData *pd )
{
    char* new_dir = get_existing_abs_path ( path, pd->current_dir );
    g_free ( pd->current_dir );
    pd->current_dir = new_dir;
    load_files ( pd );
}


static int add_file ( const char *fpath, G_GNUC_UNUSED const struct stat *sb, int typeflag, struct FTW *ftwbuf )
{
    FileBrowserFileData *pd = global_pd;

    /* Skip hidden files. */
    if ( ! pd->show_hidden && fpath[ftwbuf->base] == '.' ) {
        return FTW_SKIP_SUBTREE;
    /* Skip the current dir itself. */
    } else if ( ftwbuf->level == 0 ) {
        return FTW_CONTINUE;
    }

    /* Determine the start position of the display name in the path. */
    int pos = strlen ( fpath ) - 1;
    int level = ftwbuf->level;
    while ( level > 0 ) {
        pos--;
        if ( fpath[pos] == G_DIR_SEPARATOR ) {
            level--;
        }
    }
    pos++;

    FBFile fbfile;
    fbfile.path = g_strdup ( fpath );
    fbfile.name = &fbfile.path[pos];
    fbfile.depth = ftwbuf->level;
    fbfile.icon = NULL;

    switch ( typeflag ) {
        case FTW_F:
            fbfile.type = RFILE;
            break;
        case FTW_D:
            fbfile.type = DIRECTORY;
            break;
        case FTW_DNR:
            fbfile.type = INACCESSIBLE;
            break;
        default:
            fbfile.type = INACCESSIBLE;
            break;
    }

    /* Increase the array size if needed. */
    if ( pd->size_files <= pd->num_files ) {
        pd->size_files *= 2;
        pd->files = g_realloc ( pd->files, ( pd->size_files ) * sizeof ( FBFile ) );
    }
    pd->files[pd->num_files] = fbfile;
    pd->num_files++;

    if ( ( ftwbuf->level < global_pd->depth ) || pd->depth == 0 ) {
        return FTW_CONTINUE;
    } else {
        return FTW_SKIP_SUBTREE;
    }
}

gint compare_files ( gconstpointer a, gconstpointer b, G_GNUC_UNUSED gpointer data )
{
    FBFile *fa = ( FBFile * ) a;
    FBFile *fb = ( FBFile * ) b;
    return g_strcmp0 ( fa->name, fb->name );
}

gint compare_files_type ( gconstpointer a, gconstpointer b, G_GNUC_UNUSED gpointer data )
{
    FBFile *fa = ( FBFile * ) a;
    FBFile *fb = ( FBFile * ) b;
    if ( fa->type != fb->type ){
        return fa->type - fb->type;
    } else {
        return g_strcmp0 ( fa->name, fb->name );
    }
}

gint compare_files_depth ( gconstpointer a, gconstpointer b, G_GNUC_UNUSED gpointer data )
{
    FBFile *fa = ( FBFile * ) a;
    FBFile *fb = ( FBFile * ) b;
    if ( fa->depth != fb->depth ) {
        return fa->depth - fb->depth;
    } else {
        return g_strcmp0 ( fa->name, fb->name );
    }
}

gint compare_files_depth_type ( gconstpointer a, gconstpointer b, G_GNUC_UNUSED gpointer data )
{
    FBFile *fa = ( FBFile * ) a;
    FBFile *fb = ( FBFile * ) b;
    if ( fa->depth != fb->depth ) {
        return fa->depth - fb->depth;
    } else if ( fa->type != fb->type ) {
        return fa->type - fb->type;
    } else {
        return g_strcmp0 ( fa->name, fb->name );
    }
}
