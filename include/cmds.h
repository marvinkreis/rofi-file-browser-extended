#ifndef FILE_BROWSER_CMDS_H
#define FILE_BROWSER_CMDS_H

#include <gmodule.h>

#include "types.h"

/**
 * Sets the commands for open-bookmarks from the command line strings.
 */
void set_user_bookmarks(char **bookmark_strs, FileBrowserModePrivateData *pd);

/**
 * Sets the commands for open-custom from the command line strings.
 */
void set_user_cmds(char **cmd_strs, FileBrowserModePrivateData *pd);

/**
 * Search the PATH environment variable for executables and add them to custom commands.
 */
void search_path_for_cmds(FileBrowserModePrivateData *pd);

/**
 * Frees the commands for open-custom.
 */
void destroy_cmds(FileBrowserModePrivateData *pd);

/**
 * Frees the bookmarks for open-bookmarks.
 */
void destroy_bookmarks(FileBrowserModePrivateData *pd);

#endif
