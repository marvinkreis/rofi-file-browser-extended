#ifndef FILE_BROWSER_DEFAULTS_H
#define FILE_BROWSER_DEFAULTS_H

/* The starting directory. */
#define START_DIR g_get_current_dir ()

/* The configuration file. */
#define CONFIG_FILE g_build_filename ( g_get_user_config_dir(), "rofi", "file-browser", NULL )

/* The default command used to open files. */
#define CMD "xdg-open \"%s\""

/* The depth up to which files are recursively listed. */
#define DEPTH 1

/* Only show directories. */
#define ONLY_DIRS false

/* Only show files. */
#define ONLY_FILES false

/* Follow symlinks. */
#define FOLLOW_SYMLINKS false

/* Show hidden files by default. */
#define SHOW_HIDDEN false

/* Treat the parent directory (..) as the current directory when opening it. */
#define OPEN_PARENT_AS_SELF false

/* Sort file by type: directories first, inaccessible files last. */
#define SORT_BY_TYPE true

/* Sort file by depth: files with lower depth first. */
#define SORT_BY_DEPTH false

/* Print the file path instead of opening the file. */
#define STDOUT_MODE false

/* Open directories instead of descending into them. */
#define NO_DESCEND false

/* Hide the parent directory (..). */
#define HIDE_PARENT false

/* Read paths to display from stdin. */
#define STDIN_MODE false

/* Add executables from $PATH to the cmds. */
#define SEARCH_PATH_FOR_CMDS false

/* Use mode keys (kb-mode-next, kb-mode-previous) to toggle hidden files. */
#define USE_MODE_KEYS false

/* Show icons. */
#define SHOW_ICONS true

/* The fallback icon themes. */
#define FALLBACK_ICON_THEMES "Adwaita", "gnome"

/* Show a status with the current path and mode. */
#define SHOW_STATUS true

/* The status format. */
#define HIDE_HIDDEN_SYMBOL "[-]"
#define SHOW_HIDDEN_SYMBOL "[+]"
#define PATH_SEP " / "

/* The name to display for the parent directory. */
#define UP_TEXT ".."

/* Special / Fallback icons. */
#define UP_ICON "go-up"
#define INACCESSIBLE_ICON "error"
#define FALLBACK_ICON "text-x-generic"
#define ERROR_ICON "error"

/* The message to display when prompting the user to enter the program to open a file with.
   If the message contains %s, it will be replaced with the file name. */
#define OPEN_CUSTOM_MESSAGE_FORMAT "Enter command to open '%s' with, or cancel to go back."

/* Keys for custom bindings. Only KB_CUSTOM_* and KB_ACCEPT_ALT supported. See types.h. */
/* Key for opening file with custom command. */
#define OPEN_CUSTOM_KEY KB_ACCEPT_ALT
/* Key for opening file without closing. */
#define OPEN_MULTI_KEY KB_CUSTOM_11
/* Key for toggling hidden files. */
#define TOGGLE_HIDDEN_KEY KB_CUSTOM_12

/* Separators for open-custom commands. */
#define OPEN_CUSTOM_CMD_NAME_SEP ";name:"
#define OPEN_CUSTOM_CMD_ICON_SEP ";icon:"

#endif
