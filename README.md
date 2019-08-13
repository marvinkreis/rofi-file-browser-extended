# rofi-file-browser-extended

Use rofi as a file browser.

![Screenshot](https://marvinkreis.github.io/rofi-file-browser-extended/example.png)

# Index

- [Features](#features)
    - [Open custom](#open-custom)
    - [Stdin mode](#stdin-mode)
- [Key bindings](#key-bindings)
- [Command line options](#command-line-options)
    - [Behaviour](#behaviour)
    - [Key bindings](#key-bindings-1)
    - [Appearance](#appearance)
- [Configuration](#configuration)
    - [Config file(s)](#config-files)
- [Troubleshooting](#troubleshooting)
- [Installation](#installation)
    - [Arch User Repository](#arch-user-repository)
    - [Dependencies](#dependencies)
    - [Compilation](#compilation)

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

## Open custom

Press the `open custom` key (see [Key bindings](#key-bindings)) to enter `open custom` mode on the selected file.
When in `open custom` mode, rofi will display a list of commands to open the selected file with.

![Screenshot](https://marvinkreis.github.io/rofi-file-browser-extended/open-custom.png)

- A list of all executables in `$PATH` can be added to the list with `-file-browser-oc-search-path`.
- User-defined commands can be added with `-file-browser-oc-cmd` (multiple by passing the option multiple times).
- If no commands are specified the file to be opened will be shown.

User-defined commands can optionally specify an icon and a display name (with pango markup).

Example:

```
-file-browser-oc-cmd "gimp"
-file-browser-oc-cmd "pcmanfm-qt;icon:system-file-manager;name:pcmanfm"
-file-browser-oc-cmd "deadbeef --queue;icon:deadbeef;name:deadbeef <span alpha='75%'>(queue)</span>"

Format:
<command>;icon:<icon-name>;name:<name-to-displayed>
```

`icon` and `name` are optional.
The order of `icon` and `name` does not matter as long as the command comes first.

## Stdin mode

`-file-browser-stdin` puts the plugin into `stdin mode`.
In `stdin mode` the displayed paths are read from stdin.
Paths must either be relative to the starting directory (`-file-browser-dir`) or absolute.
It is not checked if the files actually exist.
The paths are not sorted or matched to any exclude patters.

Example:

```
fd | rofi -show file-browser -file-browser-stdin
fd -a | rofi -show file-browser -file-browser-stdin
ls somedir | rofi -show file-browser -file-browser-stdin -file-browser-dir somedir
```

After loading the paths, the plugin behaves no different than usual.
You may want to use this option with `-file-browser-no-descend` and / or `-file-browser-stdout`.

# Key bindings

Key                                                                              | Action
-------------------------------------------------------------------------------- | ----------------------------------------------------------------------------------
`kb-accept-alt` <br/> *(default: `Shift+Return`)* <br/> **configurable**         | `open custom`: Open the selected file with a custom command.
`kb-custom-11` <br/> *(default: `Alt+Shift+1`)* <br/> **configurable**           | `open multi`: Open the selected file without closing rofi. <br/> Can be used in the prompt of `open custom`.
`kb-custom-12` <br/> *(default: `Alt+Shift+2`)* <br/> **configurable**           | Toggle hidden files.
`kb-mode-next` <br/> *(default: `Shift+Right`)* <br/> **disabled by default**    | Show hidden files. <br/> Switch to the next rofi-mode if hidden files are already shown. <br/> (Can be disabled with `-file-browser-disable-mode-keys`)
`kb-mode-previous` <br/> *(default: `Shift+Left`)* <br/> **disabled by default** | Hide hidden files. <br/> Switch to the previous rofi-mode if hidden files are a already hidden. <br/> (Can be disabled with `-file-browser-disable-mode-keys`)

Configurable key bindings can be changed via command line options.

# Configuration

Default options can be set in `include/defaults.h`.

## Config file(s)

The default config file location is `$XDG_USER_CONFIG_DIR/rofi/file-browser` (defaults to `$HOME/config/rofi/file-browser`).
It consists of newline-separated command line options without the "-file-browser-" prefix.

Example:

```
icon-theme "Numix-Circle"
cmd        "exo-open"
oc-cmd     "evince;icon:evince"
oc-cmd     "gimp;icon:gimp"

# use-mode-keys
open-parent-as-self
```

Options can be commented out with `#`.
Quotes inside string arguments must not be escaped.
Escape sequences are currently not supported.

Command line options will override the config file (or add to config-file options if the option can be specified multiple times).
Another config file can be specified with `-file-browser-config` (multiple by passing the option multiple times).
All command line options but `-file-browser-config` itself can be used in the config file.

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
> When symlinks are followed, no file is reported twice.

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
>
> Example:
> ```
> -file-browser-exclude workspace
> -file-browser-exclude '*.pdf'
> ```

#### -file-browser-stdin
> Read paths from stdin.
> *(default: disabled)*
>
> Paths must either be relative to the starting directory (`-file-browser-dir`) or absolute.
> It is not checked if the files actually exist.
> The paths are not sorted or matched to any exclude patters.
>
> Example:
>
> ```
> fd | rofi -show file-browser -file-browser-stdin
> fd -a | rofi -show file-browser -file-browser-stdin
> ls somedir | rofi -show file-browser -file-browser-stdin -file-browser-dir somedir
> ```
>
> After loading the paths, the plugin behaves no different than usual.
You may want to use this option with `-file-browser-no-descend` and / or `-file-browser-stdout`.

#### -file-browser-stdout
> Instead of opening files, print absolute paths of selected files to stdout.
> *(default: disabled)*

#### -file-browser-oc-search-path
> Search `$PATH` for executables and display them in `open custom` mode (after user-defined commands).
> *(default: disabled)*

#### -file-browser-oc-cmd
> Specify user-defined commands to be displayed in `open custom` mode.
> *(default: none)*
>
> Format: `<command>;icon:<icon-name>;name:<name-to-displayed>`
>
> `icon` and `name` are optional.
> The order of `icon` and `name` does not matter as long as the command comes first.
> `name` may use pango markup.
>
> Example:
>
> ```
> -file-browser-oc-cmd "gimp"
> -file-browser-oc-cmd "pcmanfm-qt;icon:system-file-manager;name:pcmanfm"
> -file-browser-oc-cmd "deadbeef --queue;icon:deadbeef;name:deadbeef <span alpha='75%'>(queue)</span>"
> ```

#### -file-browser-sort-by-type `<0/1>`
> Enable / disable sort-by-type (directories first, files second, inaccessible directories last).
> *(default: 1)*

#### -file-browser-sort-by-depth `<0/1>`
> Enable / disable sort-by-depth when listing files recursively.
> Sort-by-type is secondary to sort-by-depth if both are enabled.
> *(default: 0)*

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

Supported key bindings are `kb-accept-alt` and `kb-custom-*`.
You can change the actual key bindings that correspond to `kb-accept-alt` and `kb-custom-*` in rofi's options.
Run `rofi -show keys` to display rofi's key bindings and what they are bound to.
Run `rofi -dump-config` or `rofi -dump-xresources` to get a list of available options.

#### -file-browser-use-mode-keys
> Show / hide hidden files with `kb-mode-next` and `kb-mode-previous`.
> *(default: disabled)*

#### -file-browser-open-custom-key `<rofi-key>`
> Set the key binding for `open custom`.
> *(default: `kb-accept-alt`)*

#### -file-browser-open-multi-key `<rofi-key>`
> Set the key binding for `open multi`.
> *(default: `kb-custom-11`)*

#### -file-browser-open-toggle-hidden `<rofi-key>`
> Set the key binding for toggling hidden files.
> *(default: `kb-custom-12`)*

## Appearance

The plugin will load faster when the GTK icon theme is specified.
`gtk3-icon-browser` can be used to search for icon names.

#### -file-browser-icon-theme `<theme-name>`
> Set the GTK icon theme.
> *(default: system GTK icon theme)*

#### -file-browser-theme `<theme-name>`
> Same as `-file-browser-icon-theme`.

#### -file-browser-disable-icons
> Disable icons.
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

#### -file-browser-error-icon `<icon-name>`
> Set the icon used when an error occurs (which should never happen; don't ask me why this option exists).
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
| rofi       | 1.4+    |
| gtk3       | 3.0+    |

## Compilation

This plugin can be compiled with either **CMake** or the **Autotools** build system.

### CMake

Use the following steps to compile the plugin with **CMake**:

```bash
git submodule init
git submodule update

cmake .
make
make install # optional: install the plugin
```

### Autotools

Use the following steps to compile the plugin with the **Autotools** build system:

```bash
git submodule init
git submodule update

autoreconf -i
./configure
make
make install # optional: install the plugin
```
