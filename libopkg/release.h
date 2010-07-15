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

#include "config.h"

#include "pkg_vec.h"
#include "release_cksum_list.h"

struct release
{
     char *name;
     char *datestring;
     char **architectures;
     char **components;
     release_cksum_list_t *md5sums;
#if defined HAVE_SHA256
     release_cksum_list_t *sha256sums;
#endif
};

release_t *release_new(void);
void release_deinit(release_t *release);
int release_init_from_file(release_t *release, const char *filename);

int release_has_architecture(const char *archname, release_t *release);
int release_has_component(const char *compname, release_t *release);
int release_get_packages(release_t *release, dist_src_t *dist, char *lists_dir, char *tmpdir);

int release_get_size(const char *filename, release_t *release, char *extension);
char *release_get_md5(const char *filename, release_t *release, char *extension);
char *release_get_sha256(const char *filename, release_t *release, char *extension);

void release_formatted_info(FILE *fp, release_t *release);
void release_formatted_field(FILE *fp, release_t *release, const char *field);

#endif
