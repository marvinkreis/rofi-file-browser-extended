#ifndef FILE_BROWSER_TYPES_H
#define FILE_BROWSER_TYPES_H

#include <stdbool.h>
#include <gmodule.h>
#include <nkutils-xdg-theme.h>

typedef enum FBFileType {
    UP,
    DIRECTORY,
    RFILE,
    INACCESSIBLE
} FBFileType;

typedef struct {
    /* Path of the file relative to the current dir. */
    char *name;
    /* Absolute path of the file. */
    char *path;
    enum FBFileType type;
    /* Cached icon. */
    cairo_surface_t *icon;
} FBFile;

/* Keys for custom key bindings. */
typedef enum FBKey {
    KB_CUSTOM_1,  KB_CUSTOM_2,  KB_CUSTOM_3,  KB_CUSTOM_4,  KB_CUSTOM_5,
    KB_CUSTOM_6,  KB_CUSTOM_7,  KB_CUSTOM_8,  KB_CUSTOM_9,  KB_CUSTOM_10,
    KB_CUSTOM_11, KB_CUSTOM_12, KB_CUSTOM_13, KB_CUSTOM_14, KB_CUSTOM_15,
    KB_CUSTOM_16, KB_CUSTOM_17, KB_CUSTOM_18, KB_CUSTOM_19, KB_CUSTOM_20,
    KB_ACCEPT_ALT,
    KEY_NONE
} FBKey;

typedef struct {
    /* Absolute path of the current directory. */
    char *current_dir;

    /* ---- File list ---- */
    /* List of displayed files. */
    FBFile *files;
    /* Number of displayed files. */
    unsigned int num_files;
    /* Show hidden files. */
    bool show_hidden;
    /* Scan files recursively up to a given depth. 0 means no limit. */
    int depth;
    /* Show directories first, Inaccessible files last. */
    bool sort_by_type;

    /* ---- Icons ---- */
    /* Loaded icons by their names. */
    GHashTable *icons;
    /* Icon theme context. */
    NkXdgThemeContext *xdg_context;
    /* Icon themes with fallbacks. */
    char **icon_themes;

    /* ---- Custom command prompt ---- */
    /* User is currently opening a file with a custom program.
       This prompts the user for a program to open the file with. */
    bool open_custom;
    /* The selected file index to be opened. */
    int open_custom_index;

    /* ---- Key bindings ---- */
    /* Only KB_CUSTOM_* and KB_ACCEPT_ALT supported. */
    /* Key for custom program prompt. */
    FBKey open_custom_key;
    /* Key for opening file without closing. */
    FBKey open_multi_key;
    /* Key for toggling hidden files. */
    FBKey toggle_hidden_key;
    /* Use kb-mode-previous and kb-mode-next to toggle hidden files. */
    bool use_mode_keys;

    /* ---- Other command line options ---- */
    /* Command to open files with. */
    char *cmd;
    /* Show icons in the file browser. */
    bool show_icons;
    /* Show the status bar. */
    bool show_status;
    /* Print the absolute file path of selected file instead of opening it. */
    bool dmenu;
    /* Text for the "go-up" entry. */
    char *up_text;
    /* Status bar format. */
    char *show_hidden_symbol;
    char *hide_hidden_symbol;
    char *path_sep;
    /* Icons names. */
    char *up_icon;
    char *inaccessible_icon;
    char *fallback_icon;
    char *error_icon;
} FileBrowserModePrivateData;

#endif
