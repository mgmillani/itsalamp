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

#define IS_DIGIT(c) ((c) >= '0' && (c) <= '9')
#define IS_HEXAH(c) ((c) >= 'A' && (c) <= 'F')
#define IS_HEXAL(c) ((c) >= 'a' && (c) <= 'f')

#define FROM_HEXA(c) ( (c) < 'A' ? (c) - '0' : (c) - 'A' + 10)

#define PARSE_ICON    (-1)
#define PARSE_ITEM    (-2)
#define PARSE_QUIT    (-3)
#define PARSE_EMPTY   (-4)

typedef enum {CMD_QUIT} e_cmd;

typedef struct s_colorInput
{
	unsigned char color[3];
	char *icon;
	char *message;
}t_colorInput;

typedef struct s_itemInput
{
	char *item;
	char *id;
	char add; // bool
}t_itemInput;

typedef struct s_commandInput
{
	e_cmd cmd;
}t_commandInput;

typedef union u_input
{
	t_colorInput colorInput;
	t_itemInput itemInput;
	t_commandInput commandInput;
}t_input;

/**
 * Parses an input line.
 * Returns one of PARSE_ICON, PARSE_COMMAND, PARSE_ITEM, PARSE_EMPTY on success, or the position of the first incorrect character if the parse fails.
 * Packs the input into the `parsed` union.
 * If PARSE_ICON    is returned, then parsed->iconInput    is valid
 * If PARSE_ITEM    is returned, then parsed->itemInput    is valid
 * If PARSE_EMPTY or PARSE_QUIT is returned, then parsed is unchanged
 */
int parseInput(char *input, t_input *parsed);

/**
 * Parses an input line which encodes some change in the icon.
 * Returns -1 in success, or the position of the first incorrect character if the parse fails.
 * colorInput->icon and colorInput->message will point to a substring of input
 * If no icon or message was informed, colorInput->message or colorInput->icon will be set to NULL, respectively.
 */
int parseIcon(char *input, t_colorInput *colorInput);

/**
 * Parses an input line which encodes a new item that has to be added or removed.
 * Returns -1 on success, or the position of the first incorrect character if the parse fails.
 * each of the char * of itemInput will point to a substring of input.
 * itemInput->add will be set to one if the item has to be added, and 0 if it has to be removed
 * In the latter case, itemInput->id will point to a substring of input, or will be set to NULL if not present
 */
int parseItem(char *input, t_itemInput *itemInput);

/**
 * Parses an input line which encodes a some command.
 * Returns -1 on success, or the position of the first incorrect character if the parse fails.
 * commandInput->cmd will point to a substring of input
 */
int parseCommand(char *input, t_commandInput *commandInput);

/**
 * Reads a word from text.
 * Words are separated by whitespaces. Single and double quotes can be used in order to encode a word with spaces.
 * Characters can be escaped with '\'
 * Returns -1 on success, or the position of the first incorrect character if the parse fails.
 * On return, *text is set to the next character after word, and *word will point to a substring of the original *text
 */
int readWord(char **text, char **word);

/**
 * Replace escape sequences on the given text. The available sequences are \n, \r and \t.
 * All other cases will be replaced by the character after the backslash (\)
 */
void escapeText(char *text);

#endif /* PARSE_H */
