/* release.h - the opkg package management system

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

#ifndef RELEASE_H
#define RELEASE_H

#include <stdio.h>
#include "pkg.h"

struct release
{
     char *name;
     char *datestring;
     char **architectures;
     unsigned int architectures_count;
     char **components;
     unsigned int components_count;
     char **complist;
     unsigned int complist_count;
};

typedef struct release release_t;

release_t *release_new(void);
void release_deinit(release_t *release);
int release_init_from_file(release_t *release, const char *filename);

int release_arch_supported(release_t *release);
int release_comps_supported(release_t *release, const char *complist);
int release_download(release_t *release, pkg_src_t *dist, char *lists_dir, char *tmpdir);

#endif
