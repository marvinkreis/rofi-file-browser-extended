The rofi-plugins repository was split, you can find the plugins here:

| Plugin                     | URL                                                       |
| -------------------------- | --------------------------------------------------------- |
| rofi-file-browser-extended | https://github.com/marvinkreis/rofi-file-browser-extended |
| rofi-json-menu             | https://github.com/marvinkreis/rofi-json-menu             |
| rofi-json-dict             | https://github.com/marvinkreis/rofi-json-dict             |

## Description

Use rofi to open files with a simple file browser. This is an extension Qballs's file browser plugin ([Blog Post](https://blog.sarine.nl/2017/04/19/rofi-140-sneak-preview-plugins.html), [Git](https://gitcrate.org/qtools/rofi-file_browser.)).

![Screenshot](https://marvinkreis.github.io/rofi-plugins/rofi-file_browser-extended/example.png)

### Features

* Icons depending on file types
* Showing / hiding hidden files
* Opening files with custom programs / commands
* A dmenu mode that outputs the absolute file path to stdout instead of opening the file

### Key-Bindings

Key                                                  | Action
---------------------------------------------------- | ----------------------------------------------------------------------------------
`kb-mode-next` <br/> (default: `Shift+Right`)        | Show hidden files. <br/> Switch to the next rofi-mode if hidden files are already shown. <br/> (Can be disabled with `-file-browser-disable-mode-keys`)
`kb-mode-previous` <br/> (default: `Shift+Left`)     | Hide hidden files. <br/> Switch to the previous rofi-mode if hidden files are a already hidden. <br/> (Can be disabled with `-file-browser-disable-mode-keys`)
`kb-accept-custom` <br/> (default: `Control+Return`) | Toggle hidden files (only if the input is empty).
`kb-accpet-alt` <br/> (default: `Shift+Return`)      | Open the selected file with custom a command. <br/> You will be prompted to enter the command with which to open the file.

### Command line options / Configuration

Option                                    | Description
----------------------------------------- | -----------
`-file-browser-cmd <cmd>`                 | Set the command to open selected files with (default: `xdg-open`).
`-file-browser-dir <path>`                | Set the starting directory (default: current working directory).
`-file-browser-show-hidden`               | Show hidden files (default: disabled).
`-file-browser-dmenu`                     | Print the absolute path of the selected file to stdout instead of opening it (default: disabled).
`-file-browser-disable-mode-keys`         | Disable toggling hidden files with `kb-mode-next` and `kb-mode-previous` (default: enabled).
`-file-browser-theme <theme-name>`        | Set the icon theme (default: `Adwaita`).
`-file-browser-disable-icons`             | Disable icons (default: enabled).
`-file-browser-disable-status`            | Disable the status bar (default: enabled).
`-file-browser-no-hidden-symbol <string>` | Set the status bar symbol that indicates that hidden files are not shown.
`-file-browser-hidden-symbol <string>`    | Set the status bar symbol that indicates that hidden files are shown.
`-file-browser-disable-hidden-symbol`     | Disable the status bar symbol that indicates if hidden files are shown.
`-file-browser-path-sep <string>`         | Set the path separator for the current path in the status bar.

Theme options can be used multiple times to set fallback themes.
The source file contains more configuration via `#define`.

## Compilation

### Dependencies

| Dependency | Version |
| ---------- | ------- |
| rofi       | 1.4     |

### Installation

Use the following steps to compile the plugin with the **autotools** build system:

```bash
git submodule init
git submodule update

autoreconf -i
./configure
make
make install
```
