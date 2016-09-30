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
#include <gdk-pixbuf/gdk-pixbuf.h>
#include <glib.h>

#ifndef IMAGE_H
#define IMAGE_H

/**
 * Point-wise multiplication of each pixel with 'color'
 * Input image has to use GDK_COLORSPACE_RGB and 8 bits per sample
 * Alpha is ignored if present
 */
void colorMultiply(GdkPixbuf *source, GdkPixbuf *target, guchar color[]);

/**
 * Sets the entire buf to the specific color
 */
void colorSet(GdkPixbuf *source, guchar color[]);

#endif // IMAGE_H
