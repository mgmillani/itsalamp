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

#include "parse.h"

#include "debug.h"

int parseInput(char *input, guchar *color, char **message, char **icon)
{
	typedef enum {START, COLOR_MOST, COLOR_LEAST, ICON, NO_QUOTE, DOUBLE_QUOTE, ESCAPE, SINGLE_QUOTE, MESSAGE} e_state;
	e_state state = START;
	e_state next = START;

	int i = 0;
	int w = 0;
	int cl = 0;

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
					return 0;
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
			return 0;
		}
	}
	// parse stopped in the middle
	return i;
}
