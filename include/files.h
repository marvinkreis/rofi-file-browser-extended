#ifndef FILE_BROWSER_FILES_H
#define FILE_BROWSER_FILES_H

#define _XOPEN_SOURCE 700
#define _GNU_SOURCE

#include <ftw.h>

#include "types.h"

/**
 * Frees the current file list and loads the file list for the current directory and options.
 */
void load_files ( FileBrowserFileData *fd );

/**
 * Simplifies the given path (e.g. removes ".."), sets it, and loads the file list for the new path and options.
 */
void change_dir ( char *path, FileBrowserFileData *fd );

/**
 * Destroys the file data.
 */
void destroy_files ( FileBrowserFileData *fd );

#endif
