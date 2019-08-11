#ifndef FILE_BROWSER_CMDS_H
#define FILE_BROWSER_CMDS_H

#include <gmodule.h>

#include "types.h"

/**
 * Sets the commands for open-custom from the command line strings.
 */
void set_open_custom_cmds ( char** cmd_strs, FileBrowserModePrivateData *pd );

/**
 * Search the PATH environment variable for executables and add them to custom commands.
 */
void find_custom_cmds ( FileBrowserModePrivateData *pd );


/**
 * Frees the commands for open-custom.
 */
void destroy_open_custom_cmds ( FileBrowserModePrivateData *pd );

#endif
