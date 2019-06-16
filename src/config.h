#ifndef FILE_BROWSER_CONFIG_H
#define FILE_BROWSER_CONFIG_H

/* The starting directory. */
#define START_DIR g_get_current_dir()

/* The default command used to open files. */
#define CMD "xdg-open \"%s\""

/* The depth up to which files are recursively listed. */
#define DEPTH 1

/* Show hidden files by default. */
#define SHOW_HIDDEN false

/* Sort file by type: directories first, inaccessible files last */
#define SORT_BY_TYPE true

/* Print the file path instead of opening the file. */
#define DMENU false

/* Use mode keys (kb-mode-next, kb-mode-previous) to toggle hidden files. */
#define USE_MODE_KEYS true

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

#endif
