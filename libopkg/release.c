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
     release->complist = NULL;
     release->complist_count = 0;
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
    free(release->architectures);

    for(i = 0; i < release->components_count; i++){
	free(release->components[i]);
    }
    free(release->components);

    for(i = 0; i < release->complist_count; i++){
	free(release->complist[i]);
    }
    free(release->complist);

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

const char *
item_in_list(const char *comp, char **complist, const unsigned int count)
{
     int i;

     if (!complist)
	  return comp;

     for(i = 0; i < count; i++){
	  if (strcmp(comp, complist[i]) == 0)
		    return complist[i];
     }

     return NULL;
}

int
release_arch_supported(release_t *release)
{
     nv_pair_list_elt_t *l;

     list_for_each_entry(l , &conf->arch_list.head, node) {
	  nv_pair_t *nv = (nv_pair_t *)l->data;
	  if (item_in_list(nv->name, release->architectures, release->architectures_count)) {
	       opkg_msg(DEBUG, "Arch %s (priority %s) supported for dist %s.\n",
			       nv->name, nv->value, release->name);
	       return 1;
	  }
     }

     return 0;
}

int
release_comps_supported(release_t *release, const char *complist)
{
     int ret = 1;
     int i;

     if (complist) {
	  release->complist = parse_list(complist, &release->complist_count, ' ', 1);
	  for(i = 0; i < release->complist_count; i++){
	       if (!item_in_list(release->complist[i], release->components, release->components_count)) {
		    opkg_msg(ERROR, "Component %s not supported for dist %s.\n",
				    release->complist[i], release->name);
		    ret = 0;
	       }
	  }
     }

     return ret;
}

int
release_download(release_t *release, pkg_src_t *dist, char *lists_dir, char *tmpdir)
{
     int ret = 0;
     unsigned int ncomp = release->complist_count;
     char **comps = release->complist;
     nv_pair_list_elt_t *l;
     int i;

     if (!comps) {
	  ncomp = release->components_count;
	  comps = release->components;
     }

     for(i = 0; i < ncomp; i++){
	  int err = 0;
	  char *prefix;

	  sprintf_alloc(&prefix, "%s/dists/%s/%s/binary", dist->value, dist->name,
			comps[i]);

	  list_for_each_entry(l , &conf->arch_list.head, node) {
	       char *url;
	       char *tmp_file_name, *list_file_name;

	       nv_pair_t *nv = (nv_pair_t *)l->data;

	       sprintf_alloc(&url, "%s-%s/%s", prefix, nv->name, dist->gzip ? "Packages.gz" : "Packages");

	       sprintf_alloc(&list_file_name, "%s/%s-%s-%s", lists_dir, dist->name, nv->name, comps[i]);

	       sprintf_alloc(&tmp_file_name, "%s/%s-%s-%s%s", tmpdir, dist->name, comps[i], nv->name, ".gz");

	       err = opkg_download(url, tmp_file_name, NULL, NULL, 1);
	       if (!err) {
		    FILE *in, *out;
		    opkg_msg(NOTICE, "Inflating %s.\n", url);
		    in = fopen (tmp_file_name, "r");
		    out = fopen (list_file_name, "w");
		    if (in && out) {
			 err = unzip (in, out);
			 if (err)
			      opkg_msg(INFO, "Corrumpt file at %s.\n", url);
		    } else
			 err = 1;
		    if (in)
			 fclose (in);
		    if (out)
			 fclose (out);
		    unlink (tmp_file_name);
	       }
	       free(url);

	       if (err) {
		    sprintf_alloc(&url, "%s-%s/Packages", prefix, nv->name);
		    err = opkg_download(url, list_file_name, NULL, NULL, 1);
		    free(url);
	       }

	       free(tmp_file_name);
	       free(list_file_name);
	  }

	  if(err)
	       ret = 1;

	  free(prefix);
     }

     return ret;
}
