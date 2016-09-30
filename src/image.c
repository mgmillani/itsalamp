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

#include "image.h"

void colorMultiply(GdkPixbuf *source, GdkPixbuf *target, guchar color[])
{
	int width, height, rowstride, channels;
	guchar *tPixels, *sPixels;

	channels = gdk_pixbuf_get_n_channels (target);

	g_assert (gdk_pixbuf_get_colorspace (target) == GDK_COLORSPACE_RGB);
	g_assert (gdk_pixbuf_get_bits_per_sample (target) == 8);
	g_assert (channels == 4 || channels == 3);

	width = gdk_pixbuf_get_width (target);
	height = gdk_pixbuf_get_height (target);

	rowstride = gdk_pixbuf_get_rowstride (target);
	tPixels = gdk_pixbuf_get_pixels (target);
	sPixels = gdk_pixbuf_get_pixels (source);

	int x,y;
	for(y=0 ; y < height * rowstride ; y+=rowstride)
	{
		for(x=0 ; x < width * channels ; x += channels)
		{
			int c;
			for(c = 0 ; c < 3 ; c++)
			{
				tPixels[c + x + y] = (sPixels[c + x + y] * color[c])/255;
			}
		}
	}
}


void colorSet(GdkPixbuf *target, guchar color[])
{
	int width, height, rowstride, channels;
	guchar *tPixels;

	channels = gdk_pixbuf_get_n_channels (target);

	g_assert (gdk_pixbuf_get_colorspace (target) == GDK_COLORSPACE_RGB);
	g_assert (gdk_pixbuf_get_bits_per_sample (target) == 8);
	g_assert (channels == 4 || channels == 3);

	width = gdk_pixbuf_get_width (target);
	height = gdk_pixbuf_get_height (target);

	rowstride = gdk_pixbuf_get_rowstride (target);
	tPixels = gdk_pixbuf_get_pixels (target);

	int x,y;
	for(y=0 ; y < height * rowstride ; y+=rowstride)
	{
		for(x=0 ; x < width * channels ; x += channels)
		{
			int c;
			for(c = 0 ; c < channels ; c++)
			{
				tPixels[c + x + y] = color[c];
			}
		}
	}
}
