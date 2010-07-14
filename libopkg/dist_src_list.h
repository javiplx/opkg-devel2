/* dist_src_list.h - the opkg package management system

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

#ifndef DIST_SRC_LIST_H
#define DIST_SRC_LIST_H

#include "nv_pair.h"
#include "void_list.h"


typedef struct 
{
  char *name;
  char *value;
  char **extra_data;
  int plain;
} dist_src_t;

typedef struct void_list_elt dist_src_list_elt_t;

typedef struct void_list dist_src_list_t;

void dist_src_list_init(dist_src_list_t *list);
void dist_src_list_deinit(dist_src_list_t *list);

dist_src_t *dist_src_list_append(dist_src_list_t *list, const char *name, const char *root_dir, const char *extra_data, int plain);

char *dist_src_release(dist_src_t *dist);
char *dist_src_package(dist_src_t *dist, char *compname);

char **parse_simple_list(const char *raw);
void simple_list_deinit(char **list);

#endif
