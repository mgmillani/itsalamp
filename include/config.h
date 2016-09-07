/*
 * Copyright 2016 Marcelo Garlet Millani

 This file is part of itsalamp.

 itsalamp is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.

 itsalamp is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with itsalamp.  If not, see <http://www.gnu.org/licenses/>.

 */

#ifndef CONFIG_H
#define CONFIG_H

/**
 * Creates necessary configuration directories.
 */
void prepareConfigDirectory();

/**
 * Returns the absolute path to the configuration directory.
 * If $XDG_CONFIG_HOME is defined, then the directory is $XDG_CONFIG_HOME/itsalamp.
 * Otherwise, it uses $HOME/.config/itsalamp.
 */
char *getConfigDirectory();
/**
 * Returns the absolute path to the directory which contains the icons.
 */
char *getIconDirectory();

#endif /* CONFIG_H */
