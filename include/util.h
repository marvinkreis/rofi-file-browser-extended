#ifndef FILE_BROWSER_UTIL_H
#define FILE_BROWSER_UTIL_H

/**
 * Returns the canonical version of the given path.
 */
char *canonicalize_path ( char* path );

/**
 * If the given path is not absolute, constructs an absolute file path with the current dir.
 * If a file exists for the path, returns the canonical absolute path, otherwise returns NULL.
 */
char *get_existing_abs_path ( char *path, char *current_dir );

/**
 * Prints an error message to stderr.
 */
void print_err ( const char *format, ... );

/**
 * Counts the elements in a NULL-terminated string array.
 * If the array is NULL, 0 is returned.
 */
unsigned int count_strv ( const char **array );

#endif
