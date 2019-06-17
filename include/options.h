#ifndef FILE_BROWSER_OPTIONS_H
#define FILE_BROWSER_OPTIONS_H

#include <stdbool.h>

#include "types.h"

/**
 * Sets the command line options and the defaults for missing command line options.
 * Returns false if some option could not be set and the initialization should be aborted.
 */
bool set_command_line_options ( FileBrowserModePrivateData *pd );

#endif
