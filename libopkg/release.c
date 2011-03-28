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

#include <ctype.h>

#include "release.h"
#include "opkg_utils.h"
#include "libbb/libbb.h"

#include "opkg_download.h"
#include "sprintf_alloc.h"

#include "release_parse.h"

#include "parse_util.h"

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

	err=release_parse_from_stream(release, release_file);
	if (!err) {
		if (!release_arch_supported(release)) {
			opkg_msg(ERROR, "No valid architecture found on Release file.\n",
				HOST_CPU_STR);
			err = -1;
		}
	}

	return err;
}

int
release_arch_supported(release_t *release)
{
     nv_pair_list_elt_t *l;
     int i;

     list_for_each_entry(l , &conf->arch_list.head, node) {
	  nv_pair_t *nv = (nv_pair_t *)l->data;
	  for(i = 0; i < release->architectures_count; i++){
	       if (strcmp(nv->name, release->architectures[i]) == 0) {
		    opkg_msg(DEBUG, "Arch %s (priority %s) supported for dist %s.\n",
				    nv->name, nv->value, release->name);
		    return 1;
	       }
	  }
     }

     return 0;
}

int
release_comps_supported(release_t *release, const char *complist)
{
     int ret = 0;
     unsigned int ncomp;
     char **comps;
     int i, j;

     comps = parse_list(complist, &ncomp, ' ', 1);

     for(j = 0; j < ncomp; j++){
	  for(i = 0; i < release->components_count; i++){
	       if (strcmp(comps[j], release->components[i]) == 0) {
		    opkg_msg(DEBUG, "Component %s supported for dist %s.\n",
				    comps[j], release->name);
		    ret = 1;
		    break;
	       }
	  }
     }

     for(j = 0; j < ncomp; j++){
	  free(comps[j]);
     }
     free(comps);

     return ret;
}
