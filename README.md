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

#### Showing / hiding hidden files

Hidden files can be shown or hidden by changing the mode with `MENU_PREVIOUS` and `MENU_NEXT`.

#### Opening files with custom programs / commands

Simply type `:cmd`. The chosen file will then be opened with `cmd`. `cmd` can either be a normal program name (with or without arguments) or a string containing `%s`. `%s`will then be replaced by the file name.

### Command line options / Configuration

Option       | Arguments          | Description
------------ | ------------------ | --------------------------------------------------------------------------
`-fb_cmd`    | command            | Sets the command used to open the file with
`-fb_dir`    | /path/to/directory | Sets the starting directory
`-fb_theme`  | theme name         | Sets the icon theme, use this option multiple times to set fallback themes

The source file contains more configuration via `#define`.

### Screenshot

![Screenshot](https://marvinkreis.github.io/rofi-plugins/rofi-file_browser-extended/example.png)

<br/>
<br/>
<br/>

# rofi-dict

Reads all json-files in a directory as dictionaries and lets the user query values by keys.

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

This plugin Supports three matching modes:

* `exact matching (0)`: only match if the key is matches the search string exactly
* `substring matching (1)`: match if the key containts the search string
* `Levenshtein distance matching (2)`: match if the [Levenshtein distance](https://en.wikipedia.org/wiki/Levenshtein_distance) is below a certain thresholf

Matching modes can be switched by switching modes with `MENU_PREVIOUS` and `MENU_NEXT`.

### Command line options / Configuration

Option       | Arguments          | Description
------------ | ------------------ | -----------------------------
`-dict_path` | /path/to/directory | Sets the dictionary directory
`-dict_mode` | 0 \| 1 \| 2        | Sets the matching mode

The source file contains more configuration via `#define`.

### Screenshot

![Screenshot](https://marvinkreis.github.io/rofi-plugins/rofi-dict/example.png)

<br/>
<br/>
<br/>

# rofi-prompt

Read entries from a json file and execute the entries with or without arguments.
The json file can specify a name, desription, command and icon for each entry.

### Purpose

This plugin can be used to quickly open entries from a, list of frequently used programs or scripts with
abbreviated names, and open them with arguments.

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

Option          | Arguments     | Description
--------------- | ------------- | --------------------------------------------------------------------------
`-prompt_file`  | /path/to/file | Sets the json file containing the entries.
`-prompt_theme` | theme name    | Sets the icon theme, use this option multiple times to set fallback themes

The source file contains more configuration via `#define`.

![Screenshot](https://marvinkreis.github.io/rofi-plugins/rofi-prompt/example.png)
