### List of plugins

* [rofi-file_browser-extended](#rofi-file_browser-extended)
* [rofi-dict](#rofi-dict)
* [rofi-prompt](#rofi-prompt)

### Compiling

Use the `PKGBUILD`s with `makepkg` to generate packages for the plugins.

#### Manual compilation / installation:

Go into the plugin's directory and execute the following commands:

1. `git submodule init`
2. `git submodule update`
3. `autoreconf -i`
4. `./configure`
5. `make`
6. to install: `sudo make install`

<br/>
<br/>
<br/>

# rofi-file_browser-extended

An extension of the example file browser plugin presented in [Qball's Webblog](https://blog.sarine.nl/2017/04/19/rofi-140-sneak-preview-plugins.html).

### Additional features:

* Icons depending on file types
* Showing / hiding hidden files
* Opening files with custom programs / commands
* A dmenu mode that outputs the absolute file-path to stdout instead of opening the file

### Key-Bindings

Key                                                  | Action
---------------------------------------------------- | ----------------------------------------------------------------------------------
`kb-mode-next` <br/> (default: `Shift+Right`)        | Show hidden files. <br/> Switch to the next rofi-mode if hidden files are already shown. <br/> (See the `-fb_disable_mode_keys` option)
`kb-mode-previous` <br/> (default: `Shift+Left`)     | Hide hidden files. <br/> Switch to the previous rofi-mode if hidden files are a already hidden. <br/> (See the `-fb_disable_mode_keys` option)
`kb-accept-custom` <br/> (default: `Control+Return`) | Toggle hidden files (only if the input is empty).
`kb-accpet-alt` <br/> (default: `Shift+Return`)      | Open the selected file with custom a command. <br/> You will be prompted to enter the command with which to open the file.

### Command line options / Configuration

Option                       | Description
---------------------------- | ----------------------------------------------------------------------------------
`-fb_cmd <arg>`              | Sets the command used to open the selected file with.
`-fb_dir <string>`           | Sets the starting directory.
`-fb_show_hidden`            | Shows hidden files.
`-fb_dmenu`                  | Print the absolute path of the selected file instead of opening it.
`-fb_disable_mode_keys`      | Disables toggling hidden files with kb-mode-next and kb-mode-previous.
`-fb_theme <arg>`            | Sets the icon theme, can be used multiple times to set fallback themes.
`-fb_disable_icons`          | Disables icons.
`-fb_disable_status`         | Disables the status bar that shows the current path and if hidden files are shown.
`-fb_hidden_symbol <arg>`    | Set the status bar symbol that indicates that hidden files are shown.
`-fb_no_hidden_symbol <arg>` | Set the status bar symbol that indicates that hidden files are not shown.
`-fb_path_sep <arg>`         | Set the path separator for the current path in the status bar.

The source file contains more configuration via `#define`.

### Screenshot

![Screenshot](https://marvinkreis.github.io/rofi-plugins/rofi-file_browser-extended/example.png)

<br/>
<br/>
<br/>

# rofi-dict

Reads all json-files in a directory as dictionaries and lets the user query translations.

### Dictionaries

Dictionaries must be saved as utf-8 and must have the following structure:

```json
{
    "key1": "value1",
    "key2": "value2",
    "key3": "value3"
}
```

The `convert_dict` python script can be used to convert [FreeDict dictionaries](https://github.com/freedict/fd-dictionaries) into the json format.

The plugin will only read json files with the .json file extension.

### Matching modes

This plugin supports three matching modes:

* `exact matching (0)`: only match if the key is matches the search string exactly
* `substring matching (1)`: match if the key containts the search string
* `Levenshtein distance matching (2)`: match if the [Levenshtein distance](https://en.wikipedia.org/wiki/Levenshtein_distance) is below a certain threshold

### Key-Bindings

Key                                                  | Action
---------------------------------------------------- | ----------------------------------------------------------------------------------
`kb-mode-next` <br/> (default: `Shift+Right`)        | Switch to the next matching mode. <br/> Switch to the next rofi-mode if the last matching mode is already selected.
`kb-mode-previous` <br/> (default: `Shift+Left`)     | Switch to the previous matching mode. <br/> Switch to the previous rofi-mode if the first matching mode is already selected.
`kb-accept-custom` <br/> (default: `Control+Return`) | Switch to the next matching mode (only if the input is empty).

### Command line options / Configuration

Option       | Arguments          | Description
------------ | ------------------ | -----------------------------
`-dict_path` | /path/to/directory | Sets the dictionary directory.
`-dict_mode` | 0 \| 1 \| 2        | Sets the matching mode.

The source file contains more configuration via `#define`.

### Screenshot

![Screenshot](https://marvinkreis.github.io/rofi-plugins/rofi-dict/example.png)

<br/>
<br/>
<br/>

# rofi-prompt

Read prompt entries from a json file and execute them with or without arguments.
The json file can specify a name, desription, command and icon for each entry.

### Purpose

This plugin can be used to quickly open prompt entries from a list of frequently used programs or scripts with
abbreviated names.

The plugin uses custom matching, which should make this easier:
* e.g. with default matching:
     - typing "int" might match "PrINTing" before "INTelliJ" if it comes first.
     - typing "thunar /tmp" won't match "Thunar" nor open it with "/tmp" as an argument

### Matching

* Typing part of an entry's name will match all entries' names that start with the input.
     - Selecting the entry will execute it's command.
* Typing an entry's name with arguments will match the entry.
     - Selecting the entry will execute it's command with the given arguments.
* Custom input will execute the input as a command.

### Configuration file

The entries are read from a json file (default: `XDG_CONFIG_HOME/menu_entries`) with the following format:
```json
{
    "o": {
        "description":  "file browser",
        "cmd":          "thunar",
        "terminal":     false,
        "icon":         "thunar"
    },
    "thunderbird": {
        "terminal":     false,
    },
}
```

All properties are optional with default values:

Property    | Default Value | Description
----------- | ------------- | ------------------------------------------------
description | none          | A description to display next to the name
cmd         | entry name    | The command to execute, when the entry is picked
terminal    | false         | Whether to open the command in a terminal or not
icon        | entry name    | The name of the icon to display with the entry

### Command line options / Configuration

Option                  | Arguments     | Description
----------------------- | ------------- | --------------------------------------------------------------------------
`-prompt_file`          | /path/to/file | Sets the json file containing the entries.
`-prompt_disable_icons` |               | Disables icons.
`-prompt_theme`         | theme name    | Sets the icon theme, use this option multiple times to set fallback themes.

The source file contains more configuration via `#define`.

### Screenshot

![Screenshot](https://marvinkreis.github.io/rofi-plugins/rofi-prompt/example.png)
