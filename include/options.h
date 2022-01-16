#ifndef FILE_BROWSER_OPTIONS_H
#define FILE_BROWSER_OPTIONS_H

#include <stdbool.h>

#include "types.h"

/**
 * Sets the command line options and options from the config file.
 * Command line options override config-file options / are prepended if the option can be used multiple times.
 * For missing options, defaults are set.
 * Returns false if some option could not be set and the initialization should be aborted.
 */
bool set_options ( FileBrowserModePrivateData *pd );

/**
 * Frees data used by the config-file options.
 */
void destroy_options ( FileBrowserModePrivateData *pd );

// TODO
bool write_resume_file ( FileBrowserModePrivateData *pd );

#endif
