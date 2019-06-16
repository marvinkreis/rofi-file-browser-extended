#ifndef FILE_BROWSER_H
#define FILE_BROWSER_H

#include <ftw.h>
#include <gmodule.h>

#include "types.h"

/**
 * Frees the current file list.
 */
void free_files ( FileBrowserModePrivateData *pd );

/**
 * Function used by nftw to add files to the list recursively.
 */
int add_file ( const char *fpath, const struct stat *sb, int typeflag, struct FTW *ftwbuf );

/**
 * Frees the current file list and loads the file list for the current directory and options.
 */
void load_files ( FileBrowserModePrivateData *pd );

/**
 * Simplifies the given path (e.g. removes ".."), sets it, and loads the file list for the new path.
 */
void change_dir ( char *path, FileBrowserModePrivateData *pd );

/**
 * Compares files to sort by type.
 * Directories appear before regular files, inaccessible directories and files appear last.
 * Files of the same type are sorted alphabetically.
 */
gint compare_files_type ( gconstpointer a, gconstpointer b, gpointer data );

/**
 * Compares files to sort alphabetically.
 */
gint compare_files_no_type ( gconstpointer a, gconstpointer b, gpointer data );

/**
 * Gets the most specific icon for a file, and caches it in a hash map.
 * The cairo surface is destroyed when the plugin exits.
 */
cairo_surface_t *get_icon_surf ( FBFile fbfile, int icon_size, FileBrowserModePrivateData *pd );

/**
 * If the dmenu option is not given, opens the file at the given path.
 * If the dmenu option is given, prints the absolute path to stdout.
 */
void open_file ( char *path, FileBrowserModePrivateData *pd );

#endif
