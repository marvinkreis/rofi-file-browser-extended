#ifndef FILE_BROWSER_TYPES_H
#define FILE_BROWSER_TYPES_H

#include <stdbool.h>
#include <gmodule.h>
#include <stdint.h>

// ================================================================================================================= //

typedef enum FBFileType {
    UP,
    DIRECTORY,
    RFILE,
    INACCESSIBLE,
    UNKNOWN
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

    /* Rofi icon fetcher request IDs for possible icons. */
    uint32_t *icon_fetcher_requests;
    unsigned int num_icon_fetcher_requests;
} FBFile;

typedef struct {
    /* Absolute path of the current directory. */
    char *current_dir;
    /* List of displayed files, not NULL-terminated. */
    FBFile *files;
    /* Number of displayed files. */
    unsigned int num_files;
    /* Size of the files array. */
    unsigned int size_files;
    /* Glob patterns to exclude dirs / files, not NULL-terminated. */
    GPatternSpec **exclude_patterns;
    /* Number of exclude glob patters. */
    unsigned int num_exclude_patterns;
    /* Follow symlinks. */
    bool follow_symlinks;
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
    /* Hide the parent directory (..). */
    bool hide_parent;
    /* Text for the parent directory (..). */
    char *up_text;
} FileBrowserFileData;

// ================================================================================================================= //

typedef struct {
    /* Show icons in the file browser. */
    bool show_icons;
    /* Show thumbnails for image files where possible. */
    bool show_thumbnails;
    /* Icons names. */
    char *up_icon;
    char *inaccessible_icon;
    char *fallback_icon;
} FileBrowserIconData;

// ================================================================================================================= //

/* Keys for custom key bindings. */
typedef enum FBKey {
    KB_CUSTOM_1,  KB_CUSTOM_2,  KB_CUSTOM_3,  KB_CUSTOM_4,  KB_CUSTOM_5,
    KB_CUSTOM_6,  KB_CUSTOM_7,  KB_CUSTOM_8,  KB_CUSTOM_9,  KB_CUSTOM_10,
    KB_CUSTOM_11, KB_CUSTOM_12, KB_CUSTOM_13, KB_CUSTOM_14, KB_CUSTOM_15,
    KB_CUSTOM_16, KB_CUSTOM_17, KB_CUSTOM_18, KB_CUSTOM_19,
    KB_ACCEPT_ALT,
    KEY_NONE,       // no key is assigned to an action
    KEY_UNSUPPORTED // key is not a supported FBKey
} FBKey;

typedef struct {
    /* Only KB_ACCEPT, KB_ACCEPT_ALT and KB_CUSTOM_* are supported. */
    /* Key for bookmarks prompt. */
    FBKey open_bookmarks_key;
    /* Key for custom program prompt. */
    FBKey open_custom_key;
    /* Key for opening file without closing. */
    FBKey open_multi_key;
    /* Key for toggling hidden files. */
    FBKey toggle_hidden_key;
} FileBrowserKeyData;

// ================================================================================================================= //

typedef struct {
    /* The path. */
    char *path;
    /* A name to display instead of the path, or a copy of path. */
    char *name;
    /* Name of the icon, or NULL for no icon. */
    char *icon_name;

    /* Rofi icon fetcher request ID. */
    uint32_t icon_fetcher_request;
} FBBookmark;

typedef struct {
    /* The command. */
    char *cmd;
    /* A name to display instead of the command, or a copy of cmd. */
    char *name;
    /* Name of the icon, or NULL for no icon. */
    char *icon_name;

    /* Rofi icon fetcher request ID. */
    uint32_t icon_fetcher_request;
} FBCmd;

typedef struct {
    FileBrowserFileData file_data;
    FileBrowserIconData icon_data;
    FileBrowserKeyData key_data;

    /* Command to open files with. */
    char *cmd;
    /* Show the status bar. */
    bool show_status;
    /* Print the absolute file path of selected file instead of opening it. */
    bool stdout_mode;
    /* Open directories instead of descending into them. */
    bool no_descend;
    /* Treat the parent directory (..) as the current directory when opening it. */
    bool open_parent_as_self;
    /* Read paths to display from stdin, implies no_descend. */
    bool stdin_mode;
    /* Status bar format. */
    char *show_hidden_symbol;
    char *hide_hidden_symbol;
    char *path_sep;
    /* Absolute path of a file containing a path to resume from. */
    char *resume_file;
    /* Whether to resume from the path set in resume_file or not. */
    bool resume;

    /* Table used to save options from the config file. */
    GHashTable *config_table;

    /* ---- Custom command prompt ---- */
    /* User is currently opening a file with a custom program.
       This prompts the user for a program to open the file with. */
    bool open_custom;
    /* The selected file index to be opened. */
    int open_custom_index;
    /* Commands to show in open-custom. */
    FBCmd *cmds;
    /* Number of commands. */
    int num_cmds;
    /* Show the commands, equal to (num_cmds > 0). */
    bool show_cmds;
    /* Add executables from $PATH to the cmds the next time they are shown. */
    bool search_path_for_cmds;
    /* This prompts the user for a bookmark to change current working directory. */
    bool open_bookmarks;
    /* Bookmarks to show in open-bookmarks. */
    FBBookmark *bookmarks;
    /* Number of bookmarks. */
    int num_bookmarks;
    /* Show the bookmarks, equal to (num_bookmarks > 0). */
    bool show_bookmarks;
} FileBrowserModePrivateData;

#endif
