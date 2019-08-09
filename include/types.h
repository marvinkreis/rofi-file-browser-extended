#ifndef FILE_BROWSER_TYPES_H
#define FILE_BROWSER_TYPES_H

#include <stdbool.h>
#include <cairo.h>
#include <nkutils-xdg-theme.h>

// ================================================================================================================= //

typedef enum FBFileType {
    UP,
    DIRECTORY,
    RFILE,
    INACCESSIBLE
} FBFileType;

typedef struct {
    /* Absolute path of the file. */
    char *path;
    /* Path of the file relative to the current dir.
     * This only points to path with an offset,
     * except for the parent dir's name, which points to up_text. */
    char *name;
    /* Type of the file. */
    enum FBFileType type;
    /* Depth of the file when listing recursively. */
    unsigned int depth;
    /* Cached icon. */
    cairo_surface_t *icon;
} FBFile;

typedef struct {
    /* Absolute path of the current directory. */
    char *current_dir;
    /* List of displayed files, not NULL-terminated. */
    FBFile *files;
    /* Number of displayed files. */
    unsigned int num_files;
    /* Glob patterns to exclude dirs / files, not NULL-terminated. */
    GPatternSpec **exclude_patterns;
    /* Number of exclude glob patters. */
    unsigned int num_exclude_patterns;
    /* Lower bound of the size of the files array. */
    unsigned int size_files;
    /* Show hidden files. */
    bool show_hidden;
    /* Only show dirs. */
    bool only_dirs;
    /* Only show files. */
    bool only_files;
    /* Scan files recursively up to a given depth. 0 means no limit. */
    int depth;
    /* Show directories first, inaccessible files last. */
    bool sort_by_type;
    /* Show files with lower depth first. */
    bool sort_by_depth;
    /* Text for the "go-up" entry. */
    char *up_text;
} FileBrowserFileData;

// ================================================================================================================= //

typedef struct {
    /* Show icons in the file browser. */
    bool show_icons;
    /* Loaded icons by their names. */
    GHashTable *icons;
    /* Icon theme context. */
    NkXdgThemeContext *xdg_context;
    /* Icon themes with fallbacks, NULL-terminated. */
    char **icon_themes;
    /* Icons names. */
    char *up_icon;
    char *inaccessible_icon;
    char *fallback_icon;
    char *error_icon;
} FileBrowserIconData;

// ================================================================================================================= //

/* Keys for custom key bindings. */
typedef enum FBKey {
    KB_CUSTOM_1,  KB_CUSTOM_2,  KB_CUSTOM_3,  KB_CUSTOM_4,  KB_CUSTOM_5,
    KB_CUSTOM_6,  KB_CUSTOM_7,  KB_CUSTOM_8,  KB_CUSTOM_9,  KB_CUSTOM_10,
    KB_CUSTOM_11, KB_CUSTOM_12, KB_CUSTOM_13, KB_CUSTOM_14, KB_CUSTOM_15,
    KB_CUSTOM_16, KB_CUSTOM_17, KB_CUSTOM_18, KB_CUSTOM_19, KB_CUSTOM_20,
    KB_ACCEPT_ALT,
    KEY_NONE,       // no key is assigned to an action
    KEY_UNSUPPORTED // key is not a supported FBKey
} FBKey;

typedef struct {
    /* Only KB_CUSTOM_* and KB_ACCEPT_ALT supported. */
    /* Key for custom program prompt. */
    FBKey open_custom_key;
    /* Key for opening file without closing. */
    FBKey open_multi_key;
    /* Key for toggling hidden files. */
    FBKey toggle_hidden_key;
    /* Use kb-mode-previous and kb-mode-next to toggle hidden files. */
    bool use_mode_keys;
} FileBrowserKeyData;

// ================================================================================================================= //

typedef struct {
    FileBrowserFileData file_data;
    FileBrowserIconData icon_data;
    FileBrowserKeyData key_data;

    /* Command to open files with. */
    char *cmd;
    /* Show the status bar. */
    bool show_status;
    /* Print the absolute file path of selected file instead of opening it. */
    bool dmenu;
    /* Open directories instead of descending into them. */
    bool no_descend;
    /* Status bar format. */
    char *show_hidden_symbol;
    char *hide_hidden_symbol;
    char *path_sep;

    /* ---- Custom command prompt ---- */
    /* User is currently opening a file with a custom program.
       This prompts the user for a program to open the file with. */
    bool open_custom;
    /* The selected file index to be opened. */
    int open_custom_index;
} FileBrowserModePrivateData;

#endif
