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

#include <ctype.h>
#include <stdlib.h>
#include <string.h>

#include "parse.h"

#include "debug.h"

int parseInput(char *input, t_input *parsed)
{
	int i = 0;
	int retval = PARSE_EMPTY;
	while(input[i] != '\0')
	{
		switch(input[i])
		{
			case '#':
				retval = parseIcon(input+i+1, &parsed->colorInput);
				if(retval == -1)
					retval = PARSE_ICON;
				break;
			case '+':
				parsed->itemInput.add = 1;
				retval = parseItem(input+i+1, &parsed->itemInput);
				if(retval == -1)
					retval = PARSE_ITEM;
				break;
			case '-':
				parsed->itemInput.add = 0;
				retval = parseItem(input+i+1, &parsed->itemInput);
				if(retval == -1)
					retval = PARSE_ITEM;
				break;
			case '!':
				retval = parseCommand(input+i+1, &parsed->commandInput);
				// if(retval == -1)
					// retval = PARSE_COMMAND;
				break;
			case ' ':
			case '\t':
			case '\n':
			case '\r':
				break;
			// unknown character
			default:
				return 0;
		}

		if(retval != PARSE_EMPTY)
			return retval;
		i++;
	}
	return PARSE_EMPTY;
}

int parseItem(char *input, t_itemInput *itemInput)
{
	// item
	int retval = readWord(&input, &itemInput->item);
	if(retval != -1)
		return retval;
	TRACE("item: %s", itemInput->item);
	TRACE("rest: %s", input);

	// reads the ID
	itemInput->id = NULL;
	int i;
	for(i=0 ; input[i] == '\n' || input[i] == ' ' || input[i] == '\t'; i++)
		;

	if(input[i] != '\0')
		itemInput->id = input+i;

	return -1;
}

int parseCommand(char *input, t_commandInput *commandInput)
{
	char *origInput = input;
	int diff;
	// exit / quit
	char *cmd;
	int retval = readWord(&input, &cmd);
	if(retval != -1)
		return retval;
	diff = input - origInput;
	TRACE("cmd: (%s)", cmd);

	int i;
	for(i=0 ; cmd[i] !='\0'; i++)
		cmd[i] = tolower(cmd[i]);

	if(strcmp(cmd, "exit") == 0 || strcmp(cmd,"quit") == 0)
	{
		commandInput->cmd = CMD_QUIT;
		return PARSE_QUIT;
	}

	return diff;
}

int readWord(char **text, char **word)
{
	typedef enum {START, NO_QUOTE, QUOTE, ESCAPE} e_state;
	e_state state = START;
	e_state next = START;

	char *input = *text;
	char quote = '\0';
	int i=0;
	int w=0;

	while(input[i] != '\0')
	{
		switch(state)
		{
			case START:
				switch(input[i])
				{
					// open quote
					case '\'':
					case '"': state = QUOTE; quote = input[i]; *word = input+i+1; break;
					// escape
					case '\\': next = state; state = ESCAPE; break;
					// whitespace
					case ' ':
					case '\n':
					case '\r':
					case '\t': break;
					// some other character
					default: state = NO_QUOTE; *word = input+i ; break;
				}
				break;
			case QUOTE:
				if(input[i] == quote)
				{
					*text = input+i+1;
					input[w] = '\0';
					return -1;
				}
				else if(input[i] == '\\')
				{
					next = state;
					state = ESCAPE;
					w--;
				}
				break;
			case NO_QUOTE:
				switch(input[i])
				{
					// words ends on whitespace
					case ' ':
					case '\n':
					case '\r':
					case '\t': *text = input+i+1; input[w] = '\0'; return -1;
					// escape
					case '\\': next = state; state = ESCAPE; w--; break;
					default: break;
				}
				break;
			case ESCAPE:
				state = next;
				w--;
				break;
		}
		input[w++] = input[i];
		i++;
	}

	*text = input + i;
	if(state == START || state == NO_QUOTE)
		return -1;
	else
		return i;

}

int parseIcon(char *input, t_colorInput *colorInput)
{
	typedef enum {START, COLOR_MOST, COLOR_LEAST, ICON, NO_QUOTE, DOUBLE_QUOTE, ESCAPE, SINGLE_QUOTE, MESSAGE} e_state;
	e_state state = START;
	e_state next = START;

	int i = 0;
	int w = 0;
	int cl = 0;

	char **message = &(colorInput->message);
	char **icon = &(colorInput->icon);
	unsigned char *color = colorInput->color;
	*message = NULL;
	*icon = NULL;

	while(input[i] != '\0')
	{
		char c = input[i];
		switch(state)
		{
			// skips whitespaces and #
			case START:
				if(c != ' ' && c != '\t' && c != '#')
				{
					// checks if it is an hexadecimal digit
					if( ! (IS_DIGIT(c) || IS_HEXAH(toupper(c))))
						return i;
					state = COLOR_LEAST;
					color[cl] = FROM_HEXA(c) << 4;
				}
				break;
			// reads the 4 least significant bits of the current channel
			case COLOR_LEAST:
				// checks if it is an hexadecimal digit
				if( ! (IS_DIGIT(c) || IS_HEXAH(toupper(c))) )
					return i;
				color[cl] |= FROM_HEXA(c); // color is definitely already initialized
				cl++;
				state = COLOR_MOST;
				break;
			// reads the 4 most significant bits of the current channel
			case COLOR_MOST:
				// icon
				if(c == ':')
				{
					if(cl == 3)
					{
						state = ICON;
						*icon = input + i + 1;
					}
					else
						return i;
					break;
				}
				// message
				else if(c == ' ' || c == '\t')
				{
					state = MESSAGE;
					break;
				}
				// checks if it is an hexadecimal digit
				if( ! (IS_DIGIT(c) || IS_HEXAH(toupper(c))) )
					return i;
				// read more channels than what we were supposed to
				if(cl > 2)
					return i;
				color[cl] = FROM_HEXA(c) << 4;
				state = COLOR_LEAST;
				break;
			case ICON:
				w = *icon - input + 1;
				if( c == '"')
				{
					(*icon)++;
					state = DOUBLE_QUOTE;
				}
				else if( c == '\'')
				{
					(*icon)++;
					state = SINGLE_QUOTE;
				}
				else
					state = NO_QUOTE;
				break;
			case DOUBLE_QUOTE:
				if( c == '"' )
				{
					state = MESSAGE;
					input[w++] = '\0';
				}
				else if ( c == '\\' )
				{
					next = state;
					state = ESCAPE;
				}
				else
				{
					input[w++] = c;
				}
				break;
			case SINGLE_QUOTE:
				if( c == '\'' )
				{
					state = MESSAGE;
					input[w++] = '\0';
				}
				else if ( c == '\\' )
				{
					next = state;
					state = ESCAPE;
				}
				else
					input[w++] = c;
				break;
			case ESCAPE:
				input[w++] = c;
				state = next;
				break;
			case NO_QUOTE:
				if( c == ' ' || c == '\t')
				{
					state = MESSAGE;
					input[w++] = '\0';
				}
				else if ( c == '\\' )
				{
					next = state;
					state = ESCAPE;
				}
				else
					input[w++] = c;
				break;
			case MESSAGE:
				if( c != ' ' && c != '\t')
				{
					*message = input + i;
					return -1;
				}
				break;
		}
		i++;
	}
	// read all color channels and not in the middle of something else
	if(cl == 0 && state == START)
		return -1;
	if(cl == 3)
	{
		if(state == COLOR_MOST || state == NO_QUOTE)
		{
			*message = NULL;
			return -1;
		}
	}
	// parse stopped in the middle
	return i;
}

