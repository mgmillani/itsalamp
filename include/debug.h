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

#ifndef DEBUG_HPP
#define DEBUG_HPP

#include <stdio.h>

#ifndef ERR_OFF
#define ERR(...) fprintf(stderr, ##__VA_ARGS__)
#else
#define ERR(...) ;
#endif

#ifndef TRACE_OFF
#define TRACE(...) {ERR("(%s: %s, Line %d)",__FILE__,__FUNCTION__,__LINE__); fprintf(stderr, ##__VA_ARGS__) ; ERR("\n"); }
#else
#define TRACE(...) ;
#endif
#endif // DEBUG_HPP

