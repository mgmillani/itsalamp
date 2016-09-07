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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <gtk/gtk.h>
#include <glib.h>

#include <unistd.h>

#include "image.h"
#include "parse.h"
#include "config.h"

#include "debug.h"

typedef struct s_iconData
{
  GdkPixbuf *origBuf;
  GdkPixbuf *currentBuf;
  GtkStatusIcon *icon;
  char *iconFile;
  int iconFileLen;
}t_iconData;

/**
 * Updates the icon based on input from stdin
 * each line should be in the following format
 * COLOR[:ICON] MESSAGE
 * where
 *   COLOR - is an RGB color in hexa (e.g. Ff00Cc). It can be preceded by an #.
 *   ICON - the name of the icon to be used. Its either an absolute path or relative to itsalamp's icon directory. If the path contain spaces, use either single or double quotes. Escaping is also possible by preceding a character with \
 *   MESSAGE - any text
 */
gpointer updateIcon(gpointer user_data)
{
  t_iconData *iconData = user_data;
  GtkStatusIcon *icon = iconData->icon;
  int textLen = 1024;
  gchar text[textLen];
  guchar color[3] = {0,0,0};
  while(!feof(stdin))
  {
    fgets(text, textLen, stdin);
    int j;
    for(j=0 ; text[j] != '\0' && text[j] != '\n' ; j++)
      ;
    text[j] = '\0';

    char *message;
    char *iconF = NULL;
    int p = parseInput(text, color, &message, &iconF);

    if(p > 0)
      ERR("Parse error on chacacter %d\n", p);

    // printf("color = %d,%d,%d\n", color[0], color[1], color[2]);
    if(iconF != NULL)
    {
      TRACE("New icon: %s\n", iconF);
      // if the icon is different from the current one
      if(strcmp(iconF, iconData->iconFile) != 0)
      {
        GError *error = NULL;
        GdkPixbuf *newIcon = gdk_pixbuf_new_from_file(iconF, &error);
        if(newIcon == NULL)
          ERR("Error: %s\n", error->message);
        else
        {
          // increases string size if necessary
          int len = strlen(iconF);
          if(len > iconData->iconFileLen)
          {
            iconData->iconFileLen = len;
            iconData->iconFile = realloc(iconData->iconFile, iconData->iconFileLen);
          }
          strcpy(iconData->iconFile, iconF);
          g_object_unref(iconData->origBuf);
          g_object_unref(iconData->currentBuf);
          iconData->currentBuf = gdk_pixbuf_copy(newIcon);
          iconData->origBuf = newIcon;
        }
      }
    }

    // updates the icon and the tooltip text
    gdk_threads_enter();
    colorMultiply(iconData->origBuf, iconData->currentBuf, color);
    gtk_status_icon_set_from_pixbuf (icon, iconData->currentBuf);
    gtk_status_icon_set_tooltip_text(icon, message);
    gdk_threads_leave();
  }

  return NULL;
}

int main (int argc, char **argv)
{
  /* Initialize i18n support */
  gtk_set_locale ();
  /* Initialize the widget set */
  gtk_init (&argc, &argv);

  // go to the icon directory
  prepareConfigDirectory();
  char *iconDir = getIconDirectory();
  chdir(iconDir);
  free(iconDir);

  // loads a default icon
  t_iconData iconData;
  char iconF[] = "default.svg";
  iconData.iconFileLen = sizeof(iconF)*2;
  iconData.iconFile = malloc(iconData.iconFileLen);
  strcpy(iconData.iconFile, iconF);
  GError *error = NULL;
  GdkPixbuf *iconBuf = gdk_pixbuf_new_from_file(iconData.iconFile, &error);
  if(iconBuf == NULL)
    printf("Error: %s\n", error->message);

  // starts the status icon
	GtkStatusIcon *icon = gtk_status_icon_new_from_pixbuf(iconBuf);
  gtk_status_icon_set_tooltip_text(icon, "");

  iconData.origBuf = gdk_pixbuf_copy (iconBuf);
  iconData.currentBuf = iconBuf;
  iconData.icon = icon;

  GThread *updateIconThread = g_thread_new ("update icon", updateIcon, &iconData);

  gdk_threads_enter();
  gtk_main ();
  gdk_threads_leave();

  return 0;
}
