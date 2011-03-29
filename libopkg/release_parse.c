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

#include "config.h"

#include <stdio.h>

#include "release.h"
#include "release_parse.h"
#include "libbb/libbb.h"
#include "parse_util.h"

static int
release_parse_line(release_t *release, const char *line)
{
	int ret = 0;

	switch (*line) {
	case 'A':
		if (is_field("Architectures", line)) {
			release->architectures = parse_list(line, &release->architectures_count, ' ', 0);
		}
		break;

	case 'C':
		if (is_field("Codename", line)) {
			release->name = parse_simple("Codename", line);
	    	}
		else if (is_field("Components", line)) {
			release->components = parse_list(line, &release->components_count, ' ', 0);
	    	}
		break;

	case 'D':
		if (is_field("Date", line)) {
			release->datestring = parse_simple("Date", line);
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

	free(buf);
	return ret;
}

