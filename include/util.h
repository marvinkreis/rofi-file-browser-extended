#ifndef FILE_BROWSER_UTIL_H
#define FILE_BROWSER_UTIL_H

/**
 * If the given path is not absolute, constructs an absolute file path with the current dir.
 * If a file exists for the path, returns the absolute path, otherwise returns NULL.
 */
char *get_existing_abs_path ( char *path, char *current_dir );

/**
 * Prints an error message to stderr.
 */
void print_err ( const char *format, ... );

#endif
