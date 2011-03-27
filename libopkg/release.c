/* release.c - the opkg package management system

   Javier Palacios

   Copyright (C) 2010 Javier Palacios

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public License as
   published by the Free Software Foundation; either version 2, or (at
   your option) any later version.

   This program is distributed in the hope that it will be useful, but
   WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   General Public License for more details.
*/

#include <unistd.h>
#include <ctype.h>

#include "release.h"
#include "opkg_utils.h"
#include "libbb/libbb.h"

#include "opkg_download.h"
#include "sprintf_alloc.h"


void simple_list_deinit(char **list) {
    char **l = list;
    if (list != NULL )
        while (*list != NULL )
            free(*list++);
    free(l);
}

char **parse_simple_list(const char *raw) {
    int n = 0;

    char **list = (char **) xcalloc(n+1, sizeof(char *));
    if (list == NULL) {
        return NULL;
    }
    *list = NULL;

    char *c0 = (char *) raw;
    while ( *raw != '\0' ) {
        if (isspace(*raw)) {
            *(list+n) = xstrndup(c0, raw-c0);
            list = (char **) xrealloc(list, (++n+1) * sizeof (char *));
            *(list+n) = NULL;
            while(isspace(*raw))
                raw++;
            c0 = (char *) raw;
            raw--;
        }
        raw++;
    }
    *(list+n) = xstrndup( c0 , raw-c0 );
    list = (char **) xrealloc (list , (++n+1) * sizeof (char *));
    *(list+n) = NULL;

    return list;
}

static int
is_release_field(const char *type, const char *line)
{
        if (!strncmp(line, type, strlen(type)))
                return 1;
        return 0;
}

static char *
release_parse_simple(const char *type, const char *line)
{
	return trim_xstrdup(line + strlen(type) + 2);
}

static char **
release_parse_list(const char *type, const char *line)
{
	return parse_simple_list(line + strlen(type) + 2);
}

static int
release_parse_line(release_t *release, const char *line)
{
	int ret = 0;

	switch (*line) {
	case 'A':
		if (is_release_field("Architectures", line)) {
			release->architectures = release_parse_list("Architectures", line);
		}
		break;

	case 'C':
		if (is_release_field("Codename", line)) {
			release->name = release_parse_simple("Codename", line);
	    	}
		else if (is_release_field("Components", line)) {
			release->components = release_parse_list("Components", line);
	    	}
		break;

	case 'D':
		if (is_release_field("Date", line)) {
			release->datestring = release_parse_simple("Date", line);
		}
		break;

	case ' ':
		break;

	default:
		ret = 1;
	}

	return ret;
}

int
release_parse_from_stream(release_t *release, FILE *fp)
{
	int ret = 0;
	char *buf = NULL;
	size_t buflen, nread;

	nread = getline(&buf, &buflen, fp);
	while ( nread != -1 ) {
		if (buf[nread-1] == '\n') buf[nread-1] = '\0';
		if (release_parse_line(release, buf))
                        opkg_msg(DEBUG, "Failed to parse release line for %s:\n\t%s\n",
					release->name, buf);
		nread = getline(&buf, &buflen, fp);
	}

	if (!feof(fp)) {
		opkg_perror(ERROR, "Problems reading Release file for %sd\n", release->name);
		ret = -1;
	}

	return ret;
}


static void
release_init(release_t *release)
{
     release->name = NULL;
     release->datestring = NULL;
     release->architectures = NULL;
     release->components = NULL;
}

release_t *
release_new(void)
{
     release_t *release;

     release = xcalloc(1, sizeof(release_t));
     release_init(release);

     return release;
}

void
release_deinit(release_t *release)
{
    free(release->name);
    free(release->datestring);
    simple_list_deinit(release->architectures);
    simple_list_deinit(release->components);
}

int
release_init_from_file(release_t *release, const char *filename)
{
	int err = 0;
	FILE *release_file;

	release_file = fopen(filename, "r");
	if (release_file == NULL) {
		opkg_perror(ERROR, "Failed to open %s", filename);
		return -1;
	}

	if((err=release_parse_from_stream(release, release_file)))
		unlink (filename);

	return err;
}

