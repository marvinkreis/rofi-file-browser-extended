# rofi-file-browser-extended

Use rofi to open files with a simple file browser.

![Screenshot](https://marvinkreis.github.io/rofi-file-browser-extended/example.png)

## Features

* Icons depending on file type
* Open files with custom commands (`open custom`)
* Open multiple files without closing rofi (`open multi`)
* Output the absolute file path to stdout instead of opening a file (`dmenu mode`)
* Show hidden files on demand  (`toggle hidden`)
* List files recursively (up to a configurable depth)

## Key-Bindings

Key                                                                      | Action
------------------------------------------------------------------------ | ----------------------------------------------------------------------------------
`kb-accept-alt` <br/> *(default: `Shift+Return`)* <br/> **configurable** | `open custom`: Open the selected file with custom a command. <br/> You will be prompted to enter the command with which to open the file.
`kb-custom-11` <br/> *(default: `Alt+Shift+1`)* <br/> **configurable**   | `open multi`: Open the selected file without closing rofi. <br/> Can be used in the prompt of `open custom`.
`kb-custom-12` <br/> *(default: `Alt+Shift+2`)* <br/> **configurable**   | `toggle hidden`: Toggle if hidden files are shown.
`kb-mode-next` <br/> *(default: `Shift+Right`)*                          | Show hidden files. <br/> Switch to the next rofi-mode if hidden files are already shown. <br/> (Can be disabled with `-file-browser-disable-mode-keys`)
`kb-mode-previous` <br/> *(default: `Shift+Left`)*                       | Hide hidden files. <br/> Switch to the previous rofi-mode if hidden files are a already hidden. <br/> (Can be disabled with `-file-browser-disable-mode-keys`)

Configurable key bindings can be changed via command line options.

## Configuration

Default options can be set in `include/config.h`.

# Command line options

## Behaviour

Option                               | Description
------------------------------------ | -----------
`-file-browser-cmd <cmd>`            | Set the command to open selected files with. *(default: `xdg-open`)*
`-file-browser-dir <path>`           | Set the starting directory. *(default: current working directory)*
`-file-browser-depth <depth>`        | Set the depth up to which files are recursively listed, set to 0 to set no limit. *(default: 1)*
`-file-browser-show-hidden`          | Show hidden files. *(default: disabled)*
`-file-browser-dmenu`                | `dmenu mode`: Print the absolute path of the selected file to stdout instead of opening it. *(default: disabled)*
`-file-browser-disable-sort-by-type` | Don't show directories first and inaccessible directories last. *(default: enabled)*

## Key bindings

Option                                      | Description
------------------------------------------- | -----------
`-file-browser-disable-mode-keys`           | Disable showing / hiding hidden files with `kb-mode-next` and `kb-mode-previous`. *(default: enabled)*
`-file-browser-open-custom-key <key>`       | Set the key binding for `open custom`. *(default: `kb-accept-alt`)*
`-file-browser-open-multi-key <key>`        | Set the key binding for `open multi`. *(default: `kb-custom-11`)*
`-file-browser-toggle-hidden-key <key>`     | Set the key binding for `toggle hidden`. *(default: `kb-custom-12`)*

Supported key bindings are `kb-accept-alt` and `kb-custom-*`.
You can change the actual key bindings that correspond to `kb-accept-alt` and `kb-custom-*` via [X resources](https://wiki.archlinux.org/index.php/X_resources).
Run `rofi -dump-xresources` to get a list of available options.

## Appearance

Option                                        | Description
--------------------------------------------- | -----------
`-file-browser-theme <theme-name>`            | Set the GTK icon theme. *(default: system GTK icon theme)*
`-file-browser-disable-icons`                 | Disable icons. *(default: enabled)*
`-file-browser-disable-status`                | Disable the status bar. *(default: enabled)*
`-file-browser-hide-hidden-symbol <string>`   | Set the indicator that hidden files are hidden. *(default: `"[-]"`)*
`-file-browser-show-hidden-symbol <string>`   | Set the indicator that hidden files are shown. *(default: `"[+]"`)*
`-file-browser-path-sep <string>`             | Set the path separator for the status bar. *(default: `" / "`)*
`-file-browser-up-text <string>`              | Set the text for the parent directory. *(default: `".."`)*.
`-file-browser-up-icon <icon-name>`           | Set the icon for the parent directory. *(default: `"go-up"`)*
`-file-browser-fallback-icon <icon-name>`     | Set the fallback icon used for files without icons. *(default: `"text-x-generic"`)*
`-file-browser-inaccessible-icon <icon-name>` | Set the icon for inaccessible directories. *(default: `"error"`)*
`-file-browser-error-icon <icon-name>`        | Set the icon used when an error occurs. *(default: `"error"`)*

`gtk3-icon-browser` can be used to search for icon names.
The theme option can be specified multiple times to set fallback themes.

## Example

```bash
rofi -modi file-browser -show file-browser        \
    -file-browser-cmd "exo-open"                  \
    -file-browser-dir "/"                         \
    -file-browser-depth 2                         \
    -file-browser-open-multi-key "kb-accept-alt"  \
    -file-browser-open-custom-key "kb-custom-11"  \
    -file-browser-theme "Adwaita"                 \
    -file-browser-hide-hidden-symbol ""           \
    -file-browser-path-sep "/"                    \
    -file-browser-up-text "up"                    \
    -file-browser-up-icon "go-previous"

```

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
