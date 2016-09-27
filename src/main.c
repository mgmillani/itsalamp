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

#include "image.h"
#include "parse.h"
#include "config.h"

#include <unistd.h>

#include "debug.h"

typedef struct s_iconData
{
	GdkPixbuf *origBuf;
	GdkPixbuf *currentBuf;
	GtkStatusIcon *icon;
	GtkMenu *menu;
	char *iconFile;
	int iconFileLen;
}t_iconData;

char *gIconDir;

void printMenu(GtkMenuItem *menuItem, gpointer user_data)
{
	char *id = user_data;
	if(id == NULL)
		puts(gtk_menu_item_get_label(menuItem));
	else
		puts(id);
}

/**
 * Searches for the file on multiple locations:
 *  1. working dir
 *  2. icon dir
 *  3. /usr/share/icons
 *  4. /usr/local/share/icons
 */
GdkPixbuf *findAndLoadPixbuf(const char *fname, GError **error)
{
	static const char localLamp[] = "/usr/local/share/icons/itsalamp/";
	static const char local[] = "/usr/local/share/icons/";
	static const char shareLamp[] = "/usr/share/icons/itsalamp/";
	static const char share[] = "/usr/share/icons/";

	const char *dirs[] = {gIconDir, localLamp, shareLamp, local, share};
	int len[] = {0, sizeof(localLamp), sizeof(shareLamp), sizeof(local), sizeof(share)};
	int m = strlen(gIconDir) + 1;
	len[0] = m;
	// working dir or absolute path
	GdkPixbuf *newIcon = gdk_pixbuf_new_from_file(fname, error);
	if(newIcon != NULL)
		return newIcon;

	// if path is absolute, there is no point seaching in other directories
	if(fname[0] == '/')
		return NULL;

	// default directories
	int n = strlen(fname) + 1;
	int k = MAX(sizeof(localLamp), (unsigned) m) + 1; // local is larger than share, +1 because of the /
	char *path = malloc( k + n*sizeof(*path));
	unsigned int i;
	for(i = 0 ; i < sizeof(dirs)/sizeof(*dirs) ; i++)
	{
		m = len[i]-1;
		memcpy(path, dirs[i], m);
		path[m-1] = '/';
		memcpy(path + m, fname, n);
		TRACE("searching for: %s", path);

		*error = NULL;
		newIcon = gdk_pixbuf_new_from_file(path, error);
		if(newIcon != NULL)
		{
			free(path);
			return newIcon;
		}
	}

	// file not found
	free(path);
	return NULL;
}

void updateIcon(t_iconData *iconData, t_colorInput *colorInput)
{
	char *iconF = colorInput->icon;
	GtkStatusIcon *icon = iconData->icon;
	if(iconF != NULL)
	{
		// if the icon is different from the current one
		if(strcmp(iconF, iconData->iconFile) != 0)
		{
			TRACE("New icon");
			GError *error = NULL;
			GdkPixbuf *newIcon = findAndLoadPixbuf(iconF, &error);
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
	TRACE("updating icon...");
	gdk_threads_enter();
	colorMultiply(iconData->origBuf, iconData->currentBuf, colorInput->color);
	gtk_status_icon_set_from_pixbuf (icon, iconData->currentBuf);
	gtk_status_icon_set_tooltip_text(icon, colorInput->message);
	gdk_threads_leave();

}

void updateItem(t_iconData *iconData, t_itemInput *item)
{
	TRACE("item: %s", item->item);
	GtkWidget *mitem = gtk_menu_item_new_with_label(item->item);
	g_signal_connect(mitem, "activate", G_CALLBACK(printMenu), item->id);
	gtk_menu_shell_prepend(GTK_MENU_SHELL(iconData->menu), mitem);

}

/**
 * Updates the icon based on input from stdin
 * each line should be in one of the following formats
 * #COLOR[:ICON] MESSAGE
 * +OPTION [ID]
 * -OPTION
 * ![quit | exit]
 * where
 *   COLOR   - is an RGB color in hexa (e.g. Ff00Cc).
 *   ICON    - the name of the icon to be used. Its either an absolute path or relative to itsalamp's icon directory.
 *   MESSAGE - any text
 *   OPTION  - name of the option to be modified
 *   ID      - an identifier for the option. This is optional and does not need to be unique. If informed, it will be printed when the respective option is selected by the user. Otherwise the options itself is printed.
 * If any word contain spaces, use either single or double quotes. Escaping is also possible by preceding a character with \
 */
gpointer updateAll(gpointer user_data)
{

	t_iconData iconData = *((t_iconData*) user_data);

	while(!feof(stdin))
	{
		int textLen = 1024;
		char text[textLen]; // TODO: make length dynamic

		if(fgets(text, textLen, stdin) == NULL)
			exit(0);

		t_input input;
		int type = parseInput(text, &input);

		if(type >= 0)
			ERR("Parse error on character %d\n", type);
		else
		{
			switch(type)
			{
				case PARSE_ICON:
					updateIcon(&iconData, &input.colorInput);
					break;
				case PARSE_ITEM:
					updateItem(&iconData, &input.itemInput);
					break;
				case PARSE_EMPTY:
					break;
				case PARSE_QUIT:
					exit(0);
			}
		}
	}

	// TODO: we should probably let the main thread exit
	exit(0);
	return NULL;
}

void quitMenu(GtkMenuItem *menuItem, gpointer user_data)
{
	// just to suppress a warning
	menuItem = menuItem;
	user_data = user_data;
	//
	exit(0);
}

void popupMenu (GtkStatusIcon *status_icon, guint button, guint activate_time, gpointer user_data)
{
	GtkMenu *menu = user_data;
	gtk_widget_show_all(GTK_WIDGET(menu));
	gtk_menu_popup (menu, NULL, NULL, gtk_status_icon_position_menu, status_icon, button, activate_time);
}

void setupMenu(t_iconData *iconData)
{
	GtkWidget *menu = gtk_menu_new();
	iconData->menu = GTK_MENU(menu);

	GtkWidget *quit = gtk_menu_item_new_with_label ("Quit");
	g_signal_connect(quit, "activate", G_CALLBACK(quitMenu), NULL);
	gtk_menu_shell_prepend(GTK_MENU_SHELL(menu), quit);

	g_signal_connect(iconData->icon, "popup-menu", G_CALLBACK(popupMenu), menu);
}

int main (int argc, char **argv)
{
	/* Initialize i18n support */
	gtk_set_locale ();
	/* Initialize the widget set */
	gtk_init (&argc, &argv);

	// prepareConfigDirectory(); // removed so that we do not need to depend on system-specific libraries
	gIconDir = getIconDirectory();

	// loads a default icon
	t_iconData iconData;
	char iconF[] = "default.svg";
	iconData.iconFileLen = sizeof(iconF)*2;
	iconData.iconFile = malloc(iconData.iconFileLen);
	strcpy(iconData.iconFile, iconF);
	GError *error = NULL;
	GdkPixbuf *iconBuf = findAndLoadPixbuf(iconData.iconFile, &error);
	if(iconBuf == NULL)
		printf("Error: %s\n", error->message);

	// starts the status icon
	GtkStatusIcon *icon = gtk_status_icon_new_from_pixbuf(iconBuf);
	gtk_status_icon_set_tooltip_text(icon, "");

	iconData.origBuf = gdk_pixbuf_copy (iconBuf);
	iconData.currentBuf = iconBuf;
	iconData.icon = icon;

	setupMenu(&iconData);

	/*GThread *updateAllThread =*/ g_thread_new ("update icon", updateAll, &iconData);

	gdk_threads_enter();
	TRACE("thread created");
	gtk_main ();
	gdk_threads_leave();

	free(gIconDir);

	return 0;
}
