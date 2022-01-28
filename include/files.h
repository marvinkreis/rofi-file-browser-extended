#ifndef FILE_BROWSER_FILES_H
#define FILE_BROWSER_FILES_H

#include "types.h"

/**
 * Frees the current file list and loads the file list for the current directory and options.
 */
void load_files ( FileBrowserFileData *fd );

/**
 * Loads the file list from stdin.
 * Paths must either be absolute or relative to the current directory.
 * Paths will be displayed as they are given from stdin, including the order.
 * It is not checked if the paths actually exist.
 * Paths must be separated by newlines.
 */
void load_files_from_stdin ( FileBrowserFileData *fd );

/**
 * Simplifies the given path (e.g. removes "..") and changes directory to it.
 */
void change_dir ( char *path, FileBrowserFileData *fd );

/**
 * Destroys the file data.
 */
void destroy_files ( FileBrowserFileData *fd );

#endif
