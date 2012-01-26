# EditorConfig Code::Blocks Plugin

This is an [EditorConfig][] plugin for [Code::Blocks][].

## Installation

### Install from Source

Before installation, you must have [Code::Blocks][] installed.

1.  Download the [EditorConfig core][] and follow the instructions in the README
    and INSTALL files to compile and install it.

2.  Download the [EditorConfig plugin for Code::Blocks][CBEditorConfig] and
    extract it.

3.  Use Code::Blocks to open the Code::Blocks project file:
    `editorconfig-codeblocks_unix.cbp`. To do this, launch Code::Blocks, click
    menu `File->Open...`, and select `editorconfig-codeblocks_unix.cbp` in the
    popped up file dialog.

4.  Click menu `Build->Build` to compile the plugin.

5.  Click menu `Plugins->Manage Plugins...`, then click on the `Install new`
    button. In the popped up file dialog, select the built
    `CBEditorConfig.cbplugin` file.

## Supported properties

The EditorConfig Code::Blocks plugin supports the following EditorConfig
[properties][]:

* indent_style
* indent_size
* tab_width
* end_of_line
* root (only used by EditorConfig core)


[CBEditorConfig]: https://github.com/editorconfig/editorconfig-codeblocks
[Code::Blocks]: http://www.codeblocks.org
[EditorConfig]: http://editorconfig.org
[EditorConfig core]: https://github.com/editorconfig/editorconfig
[properties]: http://editorconfig.org/#supported-properties
