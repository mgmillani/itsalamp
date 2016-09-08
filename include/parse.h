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

#include <gtk/gtk.h>

#ifndef PARSE_H
#define PARSE_H

/**
 * Parses an input line.
 * Returns 0 in success, or the position of the first incorrect character if the parse fails.
 * If the input is empty or contain only whitespaces, returns -1.
 * icon and message will point to a substring of input
 * If no icon or message was informed, *message or *icon will be set to NULL, respectively.
 */
int parseInput(char *input, guchar *color, char **message, char **icon);

#endif /* PARSE_H */
