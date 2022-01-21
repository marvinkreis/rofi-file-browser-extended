#define _XOPEN_SOURCE 700
#define _GNU_SOURCE

#include <stdbool.h>
#include <stdio.h>
#include <gmodule.h>
#include <glib/gstdio.h>

#include "types.h"
#include "util.h"
#include "files.h"
#include "compat/extended_nftw.h"

/**
 * Save file browser data globally so nftw's callback can access it.
 */
static FileBrowserFileData* global_fd;

/**
 * Frees the current files and initializes the file list with size 1.
 */
static void free_files ( FileBrowserFileData *fd );

/**
 * Inserts a file into the file list of FileBrowserFileData, expanding the list if necessary.
 */
static void insert_file ( FBFile *fbfile, FileBrowserFileData *fd );

/**
 * Matches a base name to the specified exclude glob patterns.
 */
static bool match_glob_patterns(const char *basename, FileBrowserFileData *fd);

/**
 * Function used by nftw to add files to the list recursively.
 */
static inline int add_file ( const char *fpath, G_GNUC_UNUSED const struct stat *sb, int typeflag, struct FTW *ftwbuf );

/**
 * Compares files alphabetically.
 */
static gint compare_files ( gconstpointer a, gconstpointer b, gpointer data );

/**
 * Compares files to sort by type (directories first, inaccessible files last).
 * Then compares files alphabetically.
 */
static gint compare_files_type ( gconstpointer a, gconstpointer b, gpointer data );

/**
 * Compares files to sort by depth.
 * Then compares files alphabetically.
 */
static gint compare_files_depth ( gconstpointer a, gconstpointer b, gpointer data );

/**
 * Compares files to sort by depth.
 * Then compares files to sort by type (directories first, inaccessible files last).
 * Then compares files alphabetically.
 */
static gint compare_files_depth_type ( gconstpointer a, gconstpointer b, gpointer data );

/**
 * Directories appear before regular files, inaccessible directories and files appear last.
 * Files of the same type are sorted alphabetically.
 */

// ================================================================================================================= //

static void free_files ( FileBrowserFileData *fd )
{
    FBFile *files = fd->files;
    for ( unsigned int i = 0; i < fd->num_files; i++ ) {
        g_free ( files[i].path );
    }
    fd->num_files = 0;
    fd->files = g_realloc ( fd->files, sizeof ( FBFile ) );
    fd->size_files = 1;
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
    for ( int i = 0; i < fd->num_exclude_patterns; i++ ) {
        g_pattern_spec_free ( fd->exclude_patterns[i] );
    }
    g_free ( fd->exclude_patterns );
    fd->num_exclude_patterns = 0;
}

static void insert_file ( FBFile *fbfile, FileBrowserFileData *fd ) {
    /* Increase the array size if needed. */
    if ( fd->size_files <= fd->num_files ) {
        fd->size_files *= 2;
        fd->files = g_realloc ( fd->files, ( fd->size_files ) * sizeof ( FBFile ) );
    }
    fd->files[fd->num_files] = *fbfile;
    fd->num_files++;
}

void load_files ( FileBrowserFileData *fd )
{
    free_files ( fd );

    if ( ! fd->hide_parent ) {
        /* Insert the parent dir. */
        FBFile up;
        up.type = UP;
        up.name = fd->up_text;
        up.path = g_build_filename ( fd->current_dir, "..", NULL );
        up.depth = -1;
        up.icon_fetcher_requests = NULL;
        up.num_icon_fetcher_requests = 0;
        insert_file(&up, fd);
    }

    /* Load the files. */
    global_fd = fd;

    int nftw_flags = fd->follow_symlinks ? FTW_ACTIONRETVAL : ( FTW_ACTIONRETVAL | FTW_PHYS );
    /* Workaround to make nftw work if the current directory is a symlink. */
    char *path = g_build_filename ( fd->current_dir, ".", NULL );
    extended_nftw ( path , add_file, 16, nftw_flags );
    g_free ( path );

    /* Exclude the parent dir from sorting. */
    FBFile *sort_files = fd->files;
    int num_sort_files = fd->num_files;
    if ( ! fd->hide_parent ) {
        sort_files++;
        num_sort_files--;
    }

    /* Sort all but the parent dir. */
    if ( fd->sort_by_type ) {
        if ( fd->sort_by_depth ) {
            g_qsort_with_data ( sort_files, num_sort_files, sizeof ( FBFile ), compare_files_depth_type, NULL );
        } else {
            g_qsort_with_data ( sort_files, num_sort_files, sizeof ( FBFile ), compare_files_type, NULL );
        }
    } else {
        if ( fd->sort_by_depth ) {
            g_qsort_with_data ( sort_files, num_sort_files, sizeof ( FBFile ), compare_files_depth, NULL );
        } else {
            g_qsort_with_data ( sort_files, num_sort_files, sizeof ( FBFile ), compare_files, NULL );
        }
    }
}

void change_dir ( char *path, FileBrowserFileData *pd )
{
    char* new_dir = get_canonical_abs_path ( path, pd->current_dir );
    g_free ( pd->current_dir );
    pd->current_dir = new_dir;
    g_chdir ( new_dir );
}

static bool match_glob_patterns ( const char *basename, FileBrowserFileData *fd )
{
    int len = strlen ( basename );
    for ( int i = 0; i < fd->num_exclude_patterns; i++ ) {
        if ( g_pattern_match ( fd->exclude_patterns[i], len, basename, NULL ) ) {
            return false;
        }
    }
    return true;
}

static int add_file ( const char *fpath, G_GNUC_UNUSED const struct stat *sb, int typeflag, struct FTW *ftwbuf )
{
    FileBrowserFileData *fd = global_fd;

    const char *basename = &fpath[ftwbuf->base];

    /* Skip the current dir itself. */
    if ( ftwbuf->level == 0 ) {
        return FTW_CONTINUE;
    /* Skip hidden files. */
    } else if ( ! fd->show_hidden && basename[0] == '.' ) {
        return FTW_SKIP_SUBTREE;
    /* Skip excluded patterns. */
    } else if ( ! match_glob_patterns ( basename, fd ) ) {
        return FTW_SKIP_SUBTREE;
    }

    FBFile fbfile;

    switch ( typeflag ) {

        /* Regular file. */
        case FTW_F:
        file:
            if ( fd->only_dirs ) {
                goto skip_file;
            } else {
                fbfile.type = RFILE;
            }
            break;

        /* Regular directory. */
        case FTW_D:
        directory:
            if ( fd->only_files ) {
                goto skip_file;
            } else {
                fbfile.type = DIRECTORY;
            }
            break;

        /* Inaccessible directory. */
        case FTW_DNR:
            fbfile.type = INACCESSIBLE;
            break;

        /* Symbolic link. */
        case FTW_SL:
            if ( g_file_test ( fpath, G_FILE_TEST_IS_DIR ) ) {
                goto directory;
            } else {
                goto file;
            }
            break;

        /* Symbolic link pointing to nonexistent file. */
        case FTW_SLN:
            fbfile.type = INACCESSIBLE;
            break;

        default:
            fbfile.type = UNKNOWN;
            break;
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

    fbfile.path = g_strdup ( fpath );
    fbfile.name = &fbfile.path[pos];
    fbfile.depth = ftwbuf->level;
    fbfile.icon_fetcher_requests = NULL;
    fbfile.num_icon_fetcher_requests = 0;

    insert_file ( &fbfile, fd );

skip_file:

    if ( ftwbuf->level >= global_fd->depth && fd->depth != 0 ) {
        return FTW_SKIP_SUBTREE;
    } else {
        return FTW_CONTINUE;
    }
}

void load_files_from_stdin ( FileBrowserFileData *fd ) {
    free_files ( fd );
    size_t current_dir_len = strlen ( fd->current_dir );

    char *buffer = NULL;
    size_t len = 0;
    ssize_t read;

    while ( ( read = getline ( &buffer, &len, stdin ) ) != -1 ) {
        /* Strip the newline. */
        buffer[read - 1] = '\0';

        FBFile fbfile;
        fbfile.type = UNKNOWN;
        fbfile.depth = 1;
        fbfile.icon_fetcher_requests = NULL;
        fbfile.num_icon_fetcher_requests = 0;

        /* If path is absolute. */
        if ( g_path_is_absolute ( buffer ) ) {
            fbfile.path = g_strdup ( buffer );
            fbfile.name = fbfile.path;
        } else {
            fbfile.path = g_strconcat ( fd->current_dir, "/", buffer, NULL );
            fbfile.name = &fbfile.path[current_dir_len + 1];
        }

        insert_file ( &fbfile, fd );
    }

    g_free ( buffer );
}

static gint compare_files ( gconstpointer a, gconstpointer b, G_GNUC_UNUSED gpointer data )
{
    const FBFile *fa = a;
    const FBFile *fb = b;
    return g_strcmp0 ( fa->name, fb->name );
}

static gint compare_files_type ( gconstpointer a, gconstpointer b, G_GNUC_UNUSED gpointer data )
{
    const FBFile *fa = a;
    const FBFile *fb = b;
    if ( fa->type != fb->type ) {
        return fa->type - fb->type;
    } else {
        return g_strcmp0 ( fa->name, fb->name );
    }
}

static gint compare_files_depth ( gconstpointer a, gconstpointer b, G_GNUC_UNUSED gpointer data )
{
    const FBFile *fa = a;
    const FBFile *fb = b;
    if ( fa->depth != fb->depth ) {
        return fa->depth - fb->depth;
    } else {
        return g_strcmp0 ( fa->name, fb->name );
    }
}

static gint compare_files_depth_type ( gconstpointer a, gconstpointer b, G_GNUC_UNUSED gpointer data )
{
    const FBFile *fa = a;
    const FBFile *fb = b;
    if ( fa->depth != fb->depth ) {
        return fa->depth - fb->depth;
    } else if ( fa->type != fb->type ) {
        return fa->type - fb->type;
    } else {
        return g_strcmp0 ( fa->name, fb->name );
    }
}
