## Compiling

You will need `gtk+2`. Then run

    mkdir release
    cd release
    cmake -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=/usr/local ..
    sudo make install

This will install the programs `itsalamp`, `itsacounter` and `itsalarm` to `/usr/local/bin` and the icons to `/usr/local/share/icons/itsalamp`.

## Usage

### itsalamp

It is what the name says: a lamp. The idea is to be able to control the tray icon through stdin, so that a program can display permanent notifications that do not popup.
`itsalamp` receives commands from the standard input. The input is read line by line and each line is expected to be of one of the following forms:

- #_COLOR_\[:_ICON_\] _MESSAGE_
  - Change the icon color to _COLOR_ and usage _MESSAGE_ as tooltip. If _ICON_ is informed, a new icon is loaded.
- +_OPTION_ \[_VALUE_\]
  - Adds a new option to the menu. If the user selects _OPTION_, `itsalamp` will print _VALUE_ to the standard output, or _OPTION_ if _VALUE_ was not provided.
- -_OPTION_
  - Removes the given option from the menu.
- !quit
  - Quits the program (!exit works as well).

_COLOR_ is an RGB color in hexadecimal format (e.g., BB99CC). Single or double quotes can be used when a term has spaces on it. A `\\` can also be used to escape any character. Icons are searched in the following directories:

    $XDG_CONFIG_HOME/itsalamp/icons/
    /usr/local/share/icons/itsalamp/
    /usr/share/icons/itsalamp/
    /usr/local/share/icons/
    /usr/share/icons/

If `XDG_CONFIG_HOME` is not defined, `$HOME/.config/` is used instead.

### itsacounter

This program is a countdown which outputs commands for `itsalamp`. It should be called with:

    itsacounter <[HOURSh][MINUTESm][SECONDSs]> <INITIAL COLOR> <FINAL COLOR> <ICONS...>

As time passes, the icon color will go from _INITIAL COLOR_ to _FINAL COLOR_ using a brightness preserving interpolation. The icon is changed in a way that each icon appears for the same amount of time. `itsacounter` also receives commands from the standard input:

- reset
  - Resets the timer.
- [_HOURS_h]\[_MINUTES_m\]\[_SECONDS_s\]
  - Sets the timer to the specified time.

### itsalarm

`itsalarm` connects `itsacounter` with `itsalamp`. It is used with:

    itsalarm <[HOURSh][MINUTESm][SECONDSs]> [ICONS...]

If no icons are provided, a set of default icons will be chosen.
Note that `itsalarm` is a very small shell script. If you want to change the behaviour of the GUI, the easiest way is to copy this file to `~/.bin` and edit it.
