# rofi-file-browser-extended

Use rofi to quickly open files.

![Screenshot](https://marvinkreis.github.io/rofi-file-browser-extended/example.png)

See also: `man rofi-file-browser-extended`

# Index

- [Synopsis](#synopsis)
- [Description](#description)
- [Features](#features)
- [Usage](#usage)
    - [Listing files recursively](#listing-files-recursively)
    - [Opening files with custom commands](#opening-files-with-custom-commands)
    - [Reading paths from stdin](#reading-paths-from-stdin)
- [Configuration](#configuration)
- [Key bindings](#key-bindings)
- [Command line options](#command-line-options)
    - [Behaviour](#behaviour)
    - [Key bindings](#key-bindings-1)
    - [Appearance](#appearance)
- [Troubleshooting](#troubleshooting)
- [Installation](#installation)
    - [Arch User Repository](#arch-user-repository)
    - [Dependencies](#dependencies)
    - [Compilation](#compilation)

# Synopsis

`rofi -show file-browser-extended [ -file-browser-dir <dir> ] [ -file-browser-cmd <cmd> ]` <br/>
`rofi -show file-browser-extended [ -file-browser-depth <depth> ] [ -file-browser-follow-symlinks ]` <br/>
`rofi -show file-browser-extended [ -file-browser-oc-cmd <cmd> ] [ -file-browser-oc-search-path ]` <br/>
`fd | rofi -show file-browser-extended -file-browser-stdin`

# Description

**rofi-file-browser-extended** is a configurable file browser plugin for rofi.
Its main use case is to quickly open files without having to open a window to
navigate to the file.

# Features

* Icons depending on file type
* Enter any absolute path to directly switch to it
* Open files with custom commands (`open custom`)
* Open multiple files without closing rofi (`open multi`)
* Show / hide hidden files
* List files recursively (up to a configurable depth)
* Exclude files through glob patterns
* Read options from (a) config file(s)
* Output the absolute file path to stdout instead of opening a file (`stdout mode`)
* Read and show (absolute or relative) file paths from stdin (`stdin mode`)

# Usage

## Listing files recursively

`-file-browser-depth` can be used to list files recursively up to a certain depth.
A depth of 0 means files are listed without a depth limit.

Symlinks are not followed by default.
`-file-browser-follow-symlinks` can be used to follow symlinks.
When symlinks are followed, every file is still only reported once.

## Opening files with custom commands

Press the `open custom` key (see [Key bindings](#key-bindings)) to enter `open custom` mode on the selected file.
The plugin will then display a list of commands to open the selected file with.

![Screenshot](https://marvinkreis.github.io/rofi-file-browser-extended/open-custom.png)

- All executables in `$PATH` can be added to this list with `-file-browser-oc-search-path`.
- User-defined commands can be added with `-file-browser-oc-cmd` (multiple by passing the option multiple times).
- If no commands are specified, the file to be opened will be shown instead of a list of commands.

User-defined commands can optionally specify an icon and a display name (with pango markup).

### Format:

```
<command>;icon:<icon-name>;name:<name-to-displayed>
```

`icon` and `name` are optional.
The order of `icon` and `name` does not matter as long as the command comes first.

### Example:

```
-file-browser-oc-cmd "gimp"
-file-browser-oc-cmd "pcmanfm-qt;name:pcmanfm;icon:system-file-manager"
-file-browser-oc-cmd "deadbeef --queue;icon:deadbeef;name:deadbeef <i>(queue)</i>"
```

## Reading paths from stdin

`-file-browser-stdin` can be used to read displayed paths from stdin.
Paths must either be relative to the starting directory (`-file-browser-dir`) or absolute.
It is not checked if the paths actually exist.
The paths are not sorted or matched to any exclude patters.

After reading the paths, the plugin behaves no different than usual.
You may want to use this option with `-file-browser-no-descend` and / or `-file-browser-stdout`
to make it more dmenu-like.

### Example:

```
fd | rofi -show file-browser-extended -file-browser-stdin
fd -a | rofi -show file-browser-extended -file-browser-stdin
ls somedir | rofi -show file-browser-extended -file-browser-stdin -file-browser-dir somedir
```

# Configuration

The default config file location is `$XDG_USER_CONFIG_DIR/rofi/file-browser` (defaults to `$HOME/config/rofi/file-browser`).
The config file consists of newline-separated command line options **without** the **"-file-browser-"** prefix.

Example:

```
icon-theme "Numix-Circle"
cmd        "exo-open"
oc-cmd     "evince;icon:evince"
oc-cmd     "gimp;icon:gimp"
depth      2

open-parent-as-self
```

Comments start with `#`.
Quotes inside string arguments must not be escaped.
Escape sequences are currently not supported.

Command line options will override the config file (or add to the config file arguments if the option can be specified multiple times).
A different config file can be specified with `-file-browser-config` (multiple by passing the option multiple times).
All command line options but `-file-browser-config` itself can be used in the config file.

# Key bindings

Key                                                              | Action
---------------------------------------------------------------- | ----------------------------------------------------------------------------------
`kb-accept-alt` <br/> *(default: `Shift+Return`)* <br/>          | `open custom`: Open the selected file with a custom command.
`kb-custom-1` <br/> *(default: `Alt+1`)* <br/>                   | `open multi`: Open the selected file without closing rofi. <br/> Can be used in `open custom`.
`kb-custom-2` <br/> *(default: `Alt+2`)* <br/>                   | Toggle hidden files.

Key bindings can be changed via command line options (see [Command line options/Key bindings](#key-bindings-1)).

# Command line options

## Behaviour

#### -file-browser-cmd `<cmd>`
> Set the command to open selected files with.
> *(default: `xdg-open`)*

#### -file-browser-dir `<path>`
> Set the starting directory.
> *(default: current working directory)*

#### -file-browser-depth `<depth>`
> List files recursively until a depth is reached.
> A value of 0 means no depth limit.
> *(default: 1)*

#### -file-browser-follow-symlinks
> Follow symlinks when listing files recursively.
> *(default: don't follow symlinks)*
>
> When symlinks are followed, every file is still only reported once.

#### -file-browser-show-hidden
> Show hidden files.
> *(default: hidden)*

#### -file-browser-only-dirs
> Only show directories.
> *(default: disabled)*

#### -file-browser-only-files
> Only show files.
> *(default: disabled)*

#### -file-browser-no-descend
> Open directories instead of descending into them.
> *(default: disabled)*

#### -file-browser-open-parent-as-self
> Treat the parent directory (`..`) as the current directory when opened.
> *(default: disabled)*

#### -file-browser-exclude
> Exclude paths by matching the basename to glob patterns.
> *(default: none)*
>
> Supports `*` and `?`.

#### -file-browser-stdin
> Read paths from stdin.
> *(default: disabled)*
>
> Paths must either be relative to the starting directory (`-file-browser-dir`) or absolute.
> It is not checked if the files actually exist.
> The paths are not sorted or matched to any exclude patters.

#### -file-browser-stdout
> Instead of opening files, print absolute paths of selected files to stdout.
> *(default: disabled)*

#### -file-browser-oc-search-path
> Search `$PATH` for executables and display them in `open custom` mode (after user-defined commands).
> *(default: disabled)*

#### -file-browser-oc-cmd `<cmd>`
> Specify user-defined commands to be displayed in `open custom` mode.
> *(default: none)*

#### -file-browser-sort-by-type, -file-browser-no-sort-by-type
> Enable / disable sort-by-type (directories first, files second, inaccessible directories last).
> *(default: enabled)*

#### -file-browser-sort-by-depth, -file-browser-no-sort-by-depth
> Enable / disable sort-by-depth when listing files recursively.
> Sort-by-type is secondary to sort-by-depth if both are enabled.
> *(default: disabled)*

#### -file-browser-hide-parent
> Hide the parent directory (`..`).
> *(default: shown)*

#### -file-browser-config `<path>`
> Load options from the specified config file.
> *(default: `$XDG_USER_CONFIG_DIR/rofi/file-browser`)*
>
> Can be used multiple times to load options from multiple config files.
> When this option is specified, the default config file will not be loaded.

## Key bindings

Supported key bindings are `kb-accept-alt`, `kb-custom-[0-19]` and `none` (disables the key binding).
You can change the actual key bindings that correspond to `kb-accept-alt` and `kb-custom-[0-19]` in rofi's options.
Run `rofi -show keys` to display rofi's key bindings and what they are bound to.
Run `rofi -dump-config` or `rofi -dump-xresources` to get a list of available options.

#### -file-browser-open-custom-key `<rofi-key>`
> Set the key binding for `open custom`.
> *(default: `kb-accept-alt`)*

#### -file-browser-open-multi-key `<rofi-key>`
> Set the key binding for `open multi`.
> *(default: `kb-custom-1`)*

#### -file-browser-open-toggle-hidden `<rofi-key>`
> Set the key binding for toggling hidden files.
> *(default: `kb-custom-2`)*

## Appearance

#### -file-browser-disable-icons
> Disable icons.
> *(default: enabled)*

#### -file-browser-disable-thumbnails
> Disable thumbnails for image files.
> *(default: enabled)*

#### -file-browser-disable-status
> Disable the status line that shows the current path.
> *(default: enabled)*

#### -file-browser-path-sep `<string>`
> Set the path separator for the status line.
> *(default: `" / "`)*

#### -file-browser-hide-hidden-symbol `<string>`
> Set the indicator that hidden files are hidden.
> *(default: `"[-]"`)*

#### -file-browser-show-hidden-symbol `<string>`
> Set the indicator that hidden files are shown.
> *(default: `"[+]"`)*

#### -file-browser-up-text `<string>`
> Set the text for the parent directory.
> *(default: `".."`)*.

#### -file-browser-up-icon `<icon-name>`
> Set the icon for the parent directory.
> *(default: `"go-up"`)*

#### -file-browser-fallback-icon `<icon-name>`
> Set the fallback icon used for files without icons (e.g. block devices).
> *(default: `"text-x-generic"`)*

#### -file-browser-inaccessible-icon `<icon-name>`
> Set the icon for inaccessible directories.
> *(default: `"error"`)*

## Example

```bash
rofi -modi file-browser -show file-browser        \
    -file-browser-cmd "exo-open"                  \
    -file-browser-dir "/"                         \
    -file-browser-depth 2                         \
    -file-browser-open-multi-key "kb-accept-alt"  \
    -file-browser-open-custom-key "kb-custom-11"  \
    -file-browser-icon-theme "Adwaita"            \
    -file-browser-hide-hidden-symbol ""           \
    -file-browser-path-sep "/"                    \
    -file-browser-up-text "up"                    \
    -file-browser-up-icon "go-previous"           \
    -file-browser-oc-search-path                  \
    -file-browser-oc-cmd "gimp;icon:gimp"         \
    -file-browser-exclude workspace               \
    -file-browser-exclude '*.pdf'
```

# Troubleshooting

If you encounter a problem, try running rofi from the command line.
The plugin prints error messages if things go wrong.
If that doesn't help, feel free to [create a new issue](#https://github.com/marvinkreis/rofi-file-browser-extended/issues/new).

# Installation

## Arch User Repository

This plugin can be found in the AUR under [rofi-file-browser-extended-git](https://aur.archlinux.org/packages/rofi-file-browser-extended-git/).

## Dependencies

| Dependency | Version |
| ---------- | ------- |
| rofi       | 1.6+    |

## Compilation

Use the following steps to compile the plugin with **CMake**:

```bash
cmake .
make
make install # optional: install the plugin
```
