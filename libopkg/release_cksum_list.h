/* release_cksum_list.h - the opkg package management system

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

#ifndef RELEASE_CKSUM_LIST_H
#define RELEASE_CKSUM_LIST_H

#include "nv_pair.h"
#include "void_list.h"

typedef struct 
{
  char *filename;
  int filesize;
  char *value;
} release_cksum_t;

typedef struct void_list_elt release_cksum_list_elt_t;

typedef struct void_list release_cksum_list_t;

void release_cksum_list_init(release_cksum_list_t *list);
void release_cksum_list_deinit(release_cksum_list_t *list);

//release_cksum_t *release_cksum_list_append(release_cksum_list_t *list, const char *name, int size, const char *value);
release_cksum_t *release_cksum_list_append(release_cksum_list_t *list, char **itemlist);

#endif
