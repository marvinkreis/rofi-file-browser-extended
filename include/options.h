#ifndef FILE_BROWSER_OPTIONS_H
#define FILE_BROWSER_OPTIONS_H

#include <stdbool.h>

#include "types.h"

/**
 * Sets the command line options and option from the config file.
 * For missing options, defaults are set.
 * Returns false if some option could not be set and the initialization should be aborted.
 */
bool set_options ( FileBrowserModePrivateData *pd );

/**
 * Frees data used by the options.
 */
void destroy_options ( FileBrowserModePrivateData *pd );

#endif
