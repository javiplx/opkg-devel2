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

#include "release_parse.h"

static void
release_init(release_t *release)
{
     release->name = NULL;
     release->datestring = NULL;
     release->architectures = NULL;
     release->architectures_count = 0;
     release->components = NULL;
     release->components_count = 0;
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
    int i;

    free(release->name);
    free(release->datestring);

    for(i = 0; i < release->architectures_count; i++){
	free(release->architectures[i]);
    }
    release->architectures_count = 0;
    free(release->architectures);

    for(i = 0; i < release->components_count; i++){
	free(release->components[i]);
    }
    release->components_count = 0;
    free(release->components);

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

