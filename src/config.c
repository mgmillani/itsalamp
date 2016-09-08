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

#include <string.h>
#include <stdlib.h>

#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>

#include "config.h"

#include "debug.h"

char *gConfigDir = NULL;
char *gIconDir = NULL;

void prepareConfigDirectory()
{
	char *configDir = getConfigDirectory();
	char *iconDir = getIconDirectory();
	mkdir(configDir, 0755);
	mkdir(iconDir, 0755);
	free(iconDir);
	free(configDir);
}

char *getConfigDirectory()
{
	int n;
	if(gConfigDir != NULL)
	{
		n = strlen(gConfigDir) + 1;
		char *configDir = malloc(n);
		memcpy(configDir, gConfigDir, sizeof(*configDir) * n);
		return configDir;
	}
	char *configDir = getenv ("XDG_CONFIG_HOME");

	if(configDir == NULL)
	{
		configDir = getenv ("HOME");
		if(configDir == NULL)
			return NULL;
		n = strlen(configDir) + 1;
		char subdir[] = "/.config/itsalamp";
		char *oldDir = configDir;
		configDir = malloc(n*sizeof(*configDir) + sizeof(subdir));
		memcpy(configDir, oldDir, n * sizeof(*configDir));
		strcat(configDir, subdir);
		n += sizeof(subdir);
	}
	else
	{
		n = strlen(configDir) + 1;
		char subdir[] = "/itsalamp";
		char *oldDir = configDir;
		configDir = malloc(n*sizeof(*configDir) + sizeof(subdir));
		memcpy(configDir, oldDir, n * sizeof(*configDir));
		strcat(configDir, subdir);
		n += sizeof(subdir);
	}

	gConfigDir = malloc(n);
	memcpy(gConfigDir, configDir, sizeof(*gConfigDir) * n);

	TRACE("conf dir: %s", gConfigDir);

	return configDir;
}

char *getIconDirectory()
{
	if(gIconDir != NULL)
	{
		int n = strlen(gIconDir);
		char *iconDir = malloc((n+1) * sizeof(*iconDir));
		memcpy(iconDir, gIconDir, sizeof(*iconDir) * n);
		return iconDir;
	}
	char *cDir = getConfigDirectory();
	if(cDir == NULL)
		return NULL;
	int n = strlen(cDir);
	char subdir[] = "/icons";
	cDir = realloc(cDir, n + sizeof(subdir));
	strcat(cDir, subdir);
	return cDir;
}
